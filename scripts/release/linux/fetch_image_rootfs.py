#!/usr/bin/env python3
"""Tải image build Linux từ Docker Hub theo digest và giải nén thành rootfs.

Dùng khi máy build không có quyền chạy Docker: rootfs này được chạy bằng
bubblewrap (bwrap) trong user namespace, không cần root hay daemon Docker.

Mọi blob đều được kiểm tra sha256 so với manifest đã ghim theo digest, nên nội
dung rootfs trùng khớp từng byte với image upstream đã chọn.
"""

from __future__ import annotations

import argparse
import gzip
import hashlib
import json
import os
import shutil
import stat
import subprocess
import sys
import tarfile
import urllib.parse
import urllib.request
from pathlib import Path

REGISTRY = "https://registry-1.docker.io"
AUTH_URL = "https://auth.docker.io/token"
MANIFEST_TYPES = (
    "application/vnd.oci.image.manifest.v1+json",
    "application/vnd.docker.distribution.manifest.v2+json",
)
LAYER_GZIP_TYPES = {
    "application/vnd.oci.image.layer.v1.tar+gzip",
    "application/vnd.docker.image.rootfs.diff.tar.gzip",
}
LAYER_ZSTD_TYPES = {"application/vnd.oci.image.layer.v1.tar+zstd"}
LAYER_TAR_TYPES = {"application/vnd.oci.image.layer.v1.tar"}
WHITEOUT_PREFIX = ".wh."
OPAQUE_MARKER = ".wh..wh..opq"
MAX_SYMLINK_DEPTH = 40
CHUNK_SIZE = 1 << 20


def log(message: str) -> None:
    print(f"[fetch-image] {message}", file=sys.stderr, flush=True)


def fail(message: str) -> None:
    raise SystemExit(f"[fetch-image] ERROR: {message}")


def parse_digest(digest: str) -> str:
    algorithm, _, value = digest.partition(":")
    if algorithm != "sha256" or len(value) != 64 or any(c not in "0123456789abcdef" for c in value):
        fail(f"unsupported digest: {digest}")
    return value


class Registry:
    def __init__(self, repository: str) -> None:
        self.repository = repository
        self.token = self._fetch_token()

    def _fetch_token(self) -> str:
        query = urllib.parse.urlencode(
            {"service": "registry.docker.io", "scope": f"repository:{self.repository}:pull"}
        )
        with urllib.request.urlopen(f"{AUTH_URL}?{query}", timeout=60) as response:
            payload = json.load(response)
        token = payload.get("token") or payload.get("access_token")
        if not token:
            fail("Docker Hub did not return an anonymous pull token")
        return token

    def _request(self, path: str, accept: tuple[str, ...] = ()) -> urllib.request.Request:
        request = urllib.request.Request(f"{REGISTRY}/v2/{self.repository}/{path}")
        # Token chỉ gửi tới registry, không đi theo redirect sang CDN lưu blob.
        request.add_unredirected_header("Authorization", f"Bearer {self.token}")
        if accept:
            request.add_header("Accept", ", ".join(accept))
        return request

    def manifest(self, digest: str) -> bytes:
        with urllib.request.urlopen(self._request(f"manifests/{digest}", MANIFEST_TYPES), timeout=120) as response:
            body = response.read()
        verify_bytes(body, digest, "manifest")
        return body

    def blob_bytes(self, digest: str) -> bytes:
        with urllib.request.urlopen(self._request(f"blobs/{digest}"), timeout=120) as response:
            body = response.read()
        verify_bytes(body, digest, "config")
        return body

    def blob_to_file(self, digest: str, destination: Path) -> None:
        expected = parse_digest(digest)
        if destination.exists() and sha256_file(destination) == expected:
            return
        partial = destination.with_suffix(".partial")
        hasher = hashlib.sha256()
        with urllib.request.urlopen(self._request(f"blobs/{digest}"), timeout=300) as response, open(
            partial, "wb"
        ) as output:
            while True:
                chunk = response.read(CHUNK_SIZE)
                if not chunk:
                    break
                hasher.update(chunk)
                output.write(chunk)
        if hasher.hexdigest() != expected:
            partial.unlink()
            fail(f"blob {digest} failed sha256 verification")
        partial.replace(destination)


def verify_bytes(body: bytes, digest: str, label: str) -> None:
    if hashlib.sha256(body).hexdigest() != parse_digest(digest):
        fail(f"{label} {digest} failed sha256 verification")


def sha256_file(path: Path) -> str:
    hasher = hashlib.sha256()
    with open(path, "rb") as handle:
        for chunk in iter(lambda: handle.read(CHUNK_SIZE), b""):
            hasher.update(chunk)
    return hasher.hexdigest()


def decompress_layer(blob: Path, media_type: str, destination: Path) -> None:
    """Giải nén layer ra file tar thường để đọc được hai lượt."""
    if media_type in LAYER_TAR_TYPES:
        shutil.copyfile(blob, destination)
        return
    if media_type in LAYER_GZIP_TYPES:
        with gzip.open(blob, "rb") as source, open(destination, "wb") as output:
            shutil.copyfileobj(source, output, CHUNK_SIZE)
        return
    if media_type in LAYER_ZSTD_TYPES:
        try:
            from compression import zstd  # Python 3.14+

            with zstd.open(blob, "rb") as source, open(destination, "wb") as output:
                shutil.copyfileobj(source, output, CHUNK_SIZE)
        except ImportError:
            with open(destination, "wb") as output:
                subprocess.run(["zstd", "-dc", str(blob)], stdout=output, check=True)
        return
    fail(f"unsupported layer media type: {media_type}")


class RootfsWriter:
    """Ghi entry của layer vào rootfs như chroot: symlink trong image luôn được
    phân giải bên trong rootfs, không bao giờ trỏ ra hệ thống file của máy chủ."""

    def __init__(self, root: Path) -> None:
        self.root = root

    def _split(self, name: str) -> list[str]:
        parts = []
        for part in name.split("/"):
            if part in ("", "."):
                continue
            if part == "..":
                if parts:
                    parts.pop()
                continue
            parts.append(part)
        return parts

    def resolve_parent(self, name: str) -> tuple[Path, str] | None:
        """Trả về (thư mục cha thật trên đĩa, tên entry) với mọi symlink ở các
        thành phần cha được phân giải trong rootfs. None khi đường dẫn rỗng."""
        parts = self._split(name)
        if not parts:
            return None
        parent = self._resolve_dir(parts[:-1], create=True)
        return parent, parts[-1]

    def _resolve_dir(self, parts: list[str], create: bool, depth: int = 0) -> Path:
        if depth > MAX_SYMLINK_DEPTH:
            fail("too many levels of symbolic links in image layer")
        resolved: list[str] = []
        pending = list(parts)
        while pending:
            part = pending.pop(0)
            candidate = self.root.joinpath(*resolved, part)
            if candidate.is_symlink():
                target = os.readlink(candidate)
                base = [] if target.startswith("/") else list(resolved)
                combined = self._split("/".join(base + [target]))
                resolved = []
                pending = combined + pending
                depth += 1
                if depth > MAX_SYMLINK_DEPTH:
                    fail("too many levels of symbolic links in image layer")
                continue
            if not candidate.exists():
                if not create:
                    fail(f"missing directory in rootfs: {candidate}")
                candidate.mkdir(mode=0o755)
            elif not candidate.is_dir():
                if not create:
                    fail(f"not a directory in rootfs: {candidate}")
                candidate.unlink()
                candidate.mkdir(mode=0o755)
            resolved.append(part)
        return self.root.joinpath(*resolved)

    def resolve_existing(self, name: str) -> Path:
        """Đường dẫn thật của một file đã có trong rootfs (dùng cho hard link)."""
        located = self.resolve_parent(name)
        if located is None:
            fail(f"invalid hard link target: {name}")
        parent, leaf = located
        return parent / leaf


def remove_path(path: Path) -> None:
    if path.is_symlink() or path.is_file():
        path.unlink()
    elif path.is_dir():
        make_tree_writable(path)
        shutil.rmtree(path)
    elif os.path.lexists(path):
        path.unlink()


def make_tree_writable(path: Path) -> None:
    for directory, _, _ in os.walk(path):
        os.chmod(directory, os.stat(directory).st_mode | stat.S_IRWXU)


def apply_whiteouts(writer: RootfsWriter, layer_tar: Path) -> None:
    with tarfile.open(layer_tar, "r:") as archive:
        for member in archive:
            leaf = member.name.rstrip("/").rsplit("/", 1)[-1]
            if not leaf.startswith(WHITEOUT_PREFIX):
                continue
            located = writer.resolve_parent(member.name)
            if located is None:
                continue
            parent, leaf = located
            if leaf == OPAQUE_MARKER:
                # Thư mục opaque: xoá toàn bộ nội dung từ các layer phía dưới.
                for child in list(parent.iterdir()):
                    remove_path(child)
            else:
                target = parent / leaf[len(WHITEOUT_PREFIX):]
                if os.path.lexists(target):
                    remove_path(target)


def extract_entries(writer: RootfsWriter, layer_tar: Path) -> None:
    deferred_dirs: list[tuple[Path, int]] = []
    with tarfile.open(layer_tar, "r:") as archive:
        for member in archive:
            leaf_name = member.name.rstrip("/").rsplit("/", 1)[-1]
            if leaf_name.startswith(WHITEOUT_PREFIX):
                continue
            located = writer.resolve_parent(member.name)
            if located is None:
                continue
            parent, leaf = located
            path = parent / leaf
            mode = member.mode & 0o777

            if member.isdir():
                if os.path.lexists(path) and not (path.is_dir() and not path.is_symlink()):
                    remove_path(path)
                if not path.exists():
                    path.mkdir(mode=0o755)
                # Luôn giữ quyền rwx cho chủ sở hữu để layer sau còn ghi được.
                deferred_dirs.append((path, mode | stat.S_IRWXU))
                os.chmod(path, mode | stat.S_IRWXU)
                continue

            if os.path.lexists(path):
                remove_path(path)

            if member.issym():
                os.symlink(member.linkname, path)
            elif member.islnk():
                source = writer.resolve_existing(member.linkname)
                if not os.path.lexists(source) or (source.is_dir() and not source.is_symlink()):
                    fail(f"hard link target missing: {member.linkname}")
                # Không đi theo symlink: link tới chính entry trong rootfs.
                os.link(source, path, follow_symlinks=False)
            elif member.isreg():
                extracted = archive.extractfile(member)
                if extracted is None:
                    fail(f"cannot read {member.name}")
                with extracted, open(path, "wb") as output:
                    shutil.copyfileobj(extracted, output, CHUNK_SIZE)
                os.chmod(path, mode | stat.S_IRUSR | stat.S_IWUSR)
                os.utime(path, (member.mtime, member.mtime))
            else:
                # Device node và FIFO không tạo được khi không có root; bwrap
                # tự cung cấp /dev nên bỏ qua an toàn.
                continue
    for path, mode in deferred_dirs:
        if path.is_dir() and not path.is_symlink():
            os.chmod(path, mode)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repository", required=True)
    parser.add_argument("--digest", required=True, help="sha256 digest of the linux/amd64 manifest")
    parser.add_argument("--dest", required=True, type=Path, help="rootfs directory to create")
    parser.add_argument("--cache", required=True, type=Path, help="directory for downloaded blobs")
    args = parser.parse_args()

    parse_digest(args.digest)
    dest: Path = args.dest.resolve()
    if dest.exists():
        log(f"rootfs already present: {dest}")
        return 0

    args.cache.mkdir(parents=True, exist_ok=True)
    registry = Registry(args.repository)
    manifest = json.loads(registry.manifest(args.digest))
    if "layers" not in manifest or "config" not in manifest:
        fail("the pinned digest is not a single-platform image manifest")
    config = json.loads(registry.blob_bytes(manifest["config"]["digest"]))
    if config.get("os") != "linux" or config.get("architecture") != "amd64":
        fail(f"image platform is {config.get('os')}/{config.get('architecture')}, expected linux/amd64")

    staging = dest.with_name(dest.name + ".partial")
    if staging.exists():
        remove_path(staging)
    staging.mkdir(parents=True)
    writer = RootfsWriter(staging)

    layers = manifest["layers"]
    for index, layer in enumerate(layers, start=1):
        digest = layer["digest"]
        blob = args.cache / parse_digest(digest)
        log(f"layer {index}/{len(layers)} {digest} ({layer.get('size', 0)} bytes)")
        registry.blob_to_file(digest, blob)
        layer_tar = args.cache / f"{parse_digest(digest)}.tar"
        decompress_layer(blob, layer["mediaType"], layer_tar)
        try:
            apply_whiteouts(writer, layer_tar)
            extract_entries(writer, layer_tar)
        finally:
            layer_tar.unlink()

    image_config = config.get("config") or {}
    env_lines = [entry for entry in image_config.get("Env") or [] if "=" in entry and "\n" not in entry]
    (staging / ".crossdesk-image-env").write_text("\n".join(env_lines) + "\n", encoding="utf-8")
    (staging / ".crossdesk-image-digest").write_text(args.digest + "\n", encoding="utf-8")
    staging.replace(dest)

    # Rootfs đã giải nén xong thì không cần giữ blob layer nữa.
    for layer in layers:
        blob = args.cache / parse_digest(layer["digest"])
        if blob.exists():
            blob.unlink()
    log(f"rootfs ready: {dest}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
