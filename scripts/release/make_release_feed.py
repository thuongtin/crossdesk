#!/usr/bin/env python3
"""Sinh feed cập nhật đã ký cho một release CrossDesk Postcard.

Đọc feed hiện tại của nhánh desktop-updates, ký payload Ed25519 cho từng asset
desktop trong dist/<version>/ và ghi feed mới ra file chưa tồn tại. Script không
push, không gọi GitHub và không bao giờ in nội dung khoá bí mật.
"""

import argparse
import hashlib
import importlib.util
import json
import plistlib
import re
import stat
import subprocess
import sys
import tempfile
from pathlib import Path

REPOSITORY = "https://github.com/thuongtin/crossdesk"
ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location(
    "update_version_manifest", ROOT / ".github/scripts/update_version_manifest.py")
SIGNER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(SIGNER)

# Khoá nền tảng trong feed và tên asset tương ứng; phải khớp PlatformKey() và
# ExpectedFilename() trong apps/desktop/src/updater/update_core.cpp của fork.
PLATFORM_ASSETS = {
    "macos": [("macos-arm64", "crossdesk-macos-arm64-{tag}.pkg")],
    "windows": [("windows-x64", "crossdesk-win-x64-{tag}.exe")],
    "linux": [
        ("linux-amd64", "crossdesk-linux-amd64-{tag}.deb"),
        ("linux-amd64-virtual-desktop-xfce", "crossdesk-virtual-desktop-xfce-linux-amd64-{tag}.deb"),
    ],
}
TAG_PATTERN = re.compile(r"v[0-9]+\.[0-9]+\.[0-9]+-[0-9]+-[0-9]{8}")


def sha256_file(path):
    hasher = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1 << 20), b""):
            hasher.update(chunk)
    return hasher.hexdigest()


def require_openssl3():
    """sign_update dùng `openssl pkeyutl -rawin`, chỉ có từ OpenSSL 3; LibreSSL của macOS không có."""
    output = subprocess.run(["openssl", "version"], check=True, capture_output=True, text=True).stdout
    match = re.match(r"OpenSSL (\d+)\.", output)
    if not match or int(match.group(1)) < 3:
        raise ValueError(f"OpenSSL 3 or newer is required on PATH, found: {output.strip()}")


def check_private_key(path):
    info = path.lstat()
    if not stat.S_ISREG(info.st_mode) or info.st_mode & 0o077:
        raise ValueError("Private key must be a regular file accessible only by its owner (chmod 600)")


def check_macos_package(package, version):
    """Cùng các kiểm tra của make_fork_update_manifest.py: notarize, staple, chữ ký và CFBundleVersion."""
    fields = SIGNER.version_fields(version)
    wanted_bundle_version = f'{fields["base"]}.{fields["date"]}.{fields["patch"]}'
    subprocess.run(["/usr/sbin/spctl", "--assess", "--type", "install", str(package)], check=True)
    subprocess.run(["xcrun", "stapler", "validate", str(package)], check=True, stdout=subprocess.DEVNULL)
    with tempfile.TemporaryDirectory(prefix="crossdesk-feed-") as directory:
        expanded = Path(directory) / "expanded"
        subprocess.run(["/usr/sbin/pkgutil", "--expand-full", str(package), str(expanded)],
                       check=True, stdout=subprocess.DEVNULL)
        apps = list(expanded.glob("*/Payload/CrossDesk.app"))
        if len(apps) != 1:
            raise ValueError("Expected a single CrossDesk.app payload")
        app = apps[0]
        info = plistlib.loads((app / "Contents/Info.plist").read_bytes())
        if info.get("CFBundleVersion") != wanted_bundle_version:
            raise ValueError(f"CFBundleVersion is {info.get('CFBundleVersion')!r}, "
                             f"expected {wanted_bundle_version!r}")
        subprocess.run(["/usr/bin/codesign", "--verify", "--deep", "--strict", str(app)], check=True)
        for name in ("CrossDesk", "crossdesk_updater"):
            archs = subprocess.check_output(
                ["/usr/bin/lipo", "-archs", str(app / "Contents/MacOS" / name)], text=True).strip()
            if archs != "arm64":
                raise ValueError(f"Unexpected architecture {archs!r} in {name}")


def verify_signature(signed, public_key):
    """Tự kiểm chữ ký vừa tạo bằng khoá công khai giống cách client kiểm."""
    with tempfile.TemporaryDirectory(prefix="crossdesk-verify-") as directory:
        root = Path(directory)
        der = bytes.fromhex("302a300506032b6570032100" + public_key)
        (root / "public.der").write_bytes(der)
        (root / "payload.json").write_bytes(signed["payload"].encode("utf-8"))
        (root / "signature.bin").write_bytes(bytes.fromhex(signed["signature"]))
        subprocess.run(["openssl", "pkeyutl", "-verify", "-pubin", "-keyform", "DER",
                        "-inkey", str(root / "public.der"), "-rawin",
                        "-in", str(root / "payload.json"), "-sigfile", str(root / "signature.bin")],
                       check=True, stdout=subprocess.DEVNULL)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dist", required=True, type=Path)
    parser.add_argument("--tag", required=True)
    parser.add_argument("--platforms", default="macos,windows,linux")
    parser.add_argument("--notes", required=True, type=Path)
    parser.add_argument("--current", required=True, type=Path)
    parser.add_argument("--public-key", required=True)
    parser.add_argument("--private-key", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    if not TAG_PATTERN.fullmatch(args.tag):
        parser.error("Expected a tag like v1.7.7-3-20261126")
    if not re.fullmatch(r"[0-9a-f]{64}", args.public_key):
        parser.error("Expected a 32-byte lowercase hex public key")
    version = args.tag[1:]
    platforms = [item for item in args.platforms.split(",") if item]
    unknown = set(platforms) - set(PLATFORM_ASSETS)
    if unknown:
        parser.error(f"Unknown platforms: {', '.join(sorted(unknown))}")

    if args.output.exists():
        parser.error(f"Output {args.output} already exists")
    require_openssl3()
    check_private_key(args.private_key)
    current = json.loads(args.current.read_text(encoding="utf-8"))
    if not isinstance(current, dict) or not isinstance(current.get("downloads"), dict):
        raise ValueError("Current feed must contain a downloads object")
    info = SIGNER.version_info(version)
    current_order = SIGNER.release_order(current)
    if current_order is not None and SIGNER.release_order(info) <= current_order:
        raise ValueError(f"{args.tag} is not newer than the published feed version "
                         f"{current.get('latest_version') or current.get('version')}")

    private_key = args.private_key.read_text(encoding="utf-8")
    downloads = {}
    for platform in platforms:
        for key, template in PLATFORM_ASSETS[platform]:
            filename = template.format(tag=args.tag)
            path = args.dist / filename
            if not path.is_file() or path.stat().st_size == 0:
                raise ValueError(f"Missing release asset {path}")
            if key == "macos-arm64":
                check_macos_package(path, version)
            url = f"{REPOSITORY}/releases/download/{args.tag}/{filename}"
            payload = {
                "schema": 1, "platform": key, "version": version, "filename": filename,
                "url": url, "size": path.stat().st_size, "sha256": sha256_file(path),
            }
            signed = SIGNER.sign_update(json.dumps(payload, sort_keys=True, separators=(",", ":")),
                                        private_key, args.public_key)
            verify_signature(signed, args.public_key)
            downloads[key] = {"version": version, "releaseDate": info["releaseDate"],
                              "filename": filename, "url": url, "update": signed}
            print(f"Signed {key}: {filename} size={payload['size']} sha256={payload['sha256']}")
    del private_key

    # Giữ mục của các nền tảng không build trong lần này, ghi đè các nền tảng vừa ký.
    feed = dict(info, releaseName=f"CrossDesk {args.tag}",
                releaseNotes=args.notes.read_text(encoding="utf-8"), tagName=args.tag,
                downloads={**current["downloads"], **downloads})
    with args.output.open("x", encoding="utf-8") as destination:
        json.dump(feed, destination, ensure_ascii=False, indent=2)
        destination.write("\n")
    kept = sorted(set(current["downloads"]) - set(downloads))
    print(f"Wrote {args.output} for {args.tag}; kept previous entries: {', '.join(kept) or 'none'}")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        sys.exit(1)
