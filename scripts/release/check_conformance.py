#!/usr/bin/env python3
"""Fork-conformance gate cho bản build CrossDesk Postcard.

Kiểm tra file thực thi trong từng gói đã build có trỏ đúng kênh của fork
(feed cập nhật và đường dẫn release của thuongtin/crossdesk, signaling
crossdesk.ecom.io.vn, khoá công khai Ed25519 của fork) và không còn địa chỉ cập nhật hay signaling của upstream.
Mặc định chỉ cảnh báo; với --strict thì thoát mã khác 0 khi có lỗi.
"""

import argparse
import io
import lzma
import gzip
import shutil
import subprocess
import sys
import tarfile
import tempfile
from pathlib import Path

FEED_PATH = "/thuongtin/crossdesk/desktop-updates/crossdesk-desktop-update.json"
# Trình cập nhật của fork chỉ chấp nhận URL asset https://github.com/thuongtin/crossdesk/releases/
# download/v<version>/<file>, đúng dạng URL mà make_release_feed.py ký.
RELEASE_PATH = "/thuongtin/crossdesk/releases"
FORK_SIGNALING_HOST = "crossdesk.ecom.io.vn"
UPSTREAM_SIGNALING_HOST = "api.crossdesk.cn"
UPSTREAM_UPDATE_ORIGINS = ("https://version.crossdesk.cn", "https://downloads.crossdesk.cn")
LINUX_PACKAGES = ("crossdesk", "crossdesk-virtual-desktop-xfce")


def encodings(text):
    """Chuỗi có thể nằm trong binary dạng ASCII hoặc UTF-16LE (wide string trên Windows)."""
    return (text.encode("ascii"), text.encode("utf-16-le"))


def contains(data, text):
    return any(encoded in data for encoded in encodings(text))


def check_binary(label, data, public_key, is_main):
    """Trả về danh sách lỗi của một file thực thi."""
    problems = []
    if not contains(data, public_key):
        problems.append(f"{label}: fork update public key is missing")
    if not contains(data, RELEASE_PATH):
        problems.append(f"{label}: fork release asset path {RELEASE_PATH} is missing")
    for origin in UPSTREAM_UPDATE_ORIGINS:
        if contains(data, origin):
            problems.append(f"{label}: upstream update origin {origin} is present")
    if is_main:
        if not contains(data, FEED_PATH):
            problems.append(f"{label}: fork update feed path is missing")
        if not contains(data, FORK_SIGNALING_HOST):
            problems.append(f"{label}: default signaling host {FORK_SIGNALING_HOST} is missing")
        if contains(data, UPSTREAM_SIGNALING_HOST):
            problems.append(f"{label}: upstream signaling host {UPSTREAM_SIGNALING_HOST} is present")
    return problems


def macos_binaries(package):
    with tempfile.TemporaryDirectory(prefix="crossdesk-gate-") as directory:
        expanded = Path(directory) / "expanded"
        subprocess.run(["/usr/sbin/pkgutil", "--expand-full", str(package), str(expanded)],
                       check=True, stdout=subprocess.DEVNULL)
        apps = list(expanded.glob("*/Payload/CrossDesk.app"))
        if len(apps) != 1:
            raise ValueError(f"{package.name}: expected a single CrossDesk.app payload")
        macos_dir = apps[0] / "Contents" / "MacOS"
        return [
            (f"{package.name}:CrossDesk", (macos_dir / "CrossDesk").read_bytes(), True),
            (f"{package.name}:crossdesk_updater", (macos_dir / "crossdesk_updater").read_bytes(), False),
        ]


def windows_binaries(gate_dir):
    main = gate_dir / "crossdesk.exe"
    updater = gate_dir / "crossdesk_updater.exe"
    for path in (main, updater):
        if not path.is_file():
            raise ValueError(f"missing Windows build output {path}")
    return [
        ("windows:crossdesk.exe", main.read_bytes(), True),
        ("windows:crossdesk_updater.exe", updater.read_bytes(), False),
    ]


def read_ar_members(data):
    """Đọc các member của file .deb (định dạng ar) mà không cần công cụ ngoài."""
    if not data.startswith(b"!<arch>\n"):
        raise ValueError("not an ar archive")
    offset = 8
    members = {}
    while offset + 60 <= len(data):
        header = data[offset:offset + 60]
        name = header[:16].decode("ascii").strip().rstrip("/")
        size = int(header[48:58].decode("ascii").strip())
        offset += 60
        members[name] = data[offset:offset + size]
        offset += size + (size % 2)
    return members


def decompress(name, payload):
    if name.endswith(".xz"):
        return lzma.decompress(payload)
    if name.endswith(".gz"):
        return gzip.decompress(payload)
    if name.endswith(".zst"):
        if shutil.which("zstd") is None:
            raise ValueError("zstd is required to read zstd-compressed .deb data")
        return subprocess.run(["zstd", "-dc"], input=payload, check=True, capture_output=True).stdout
    if name.endswith(".tar"):
        return payload
    raise ValueError(f"unsupported .deb data member {name}")


def linux_binaries(package):
    members = read_ar_members(package.read_bytes())
    data_name = next((name for name in members if name.startswith("data.tar")), None)
    if data_name is None:
        raise ValueError(f"{package.name}: data.tar member is missing")
    wanted = {
        "usr/lib/crossdesk/crossdesk-bin": True,
        "usr/lib/crossdesk/crossdesk_updater": False,
    }
    found = []
    with tarfile.open(fileobj=io.BytesIO(decompress(data_name, members[data_name])), mode="r:") as archive:
        for member in archive:
            name = member.name.lstrip("./") if member.name.startswith("./") else member.name
            if name in wanted and member.isreg():
                extracted = archive.extractfile(member)
                found.append((f"{package.name}:{name.rsplit('/', 1)[-1]}", extracted.read(), wanted[name]))
    if len(found) != len(wanted):
        raise ValueError(f"{package.name}: crossdesk-bin or crossdesk_updater is missing")
    return found


def collect(platform, dist, version):
    if platform == "macos":
        return macos_binaries(dist / f"crossdesk-macos-arm64-{version}.pkg")
    if platform == "windows":
        return windows_binaries(dist / "gate" / "windows")
    if platform == "linux":
        binaries = []
        for package in LINUX_PACKAGES:
            binaries.extend(linux_binaries(dist / f"{package}-linux-amd64-{version}.deb"))
        return binaries
    raise ValueError(f"unknown platform {platform}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dist", required=True, type=Path)
    parser.add_argument("--version", required=True)
    parser.add_argument("--platforms", default="macos,windows,linux")
    parser.add_argument("--public-key", required=True)
    parser.add_argument("--strict", action="store_true")
    args = parser.parse_args()

    report_lines = []
    failed = []
    for platform in [item for item in args.platforms.split(",") if item]:
        try:
            problems = []
            binaries = collect(platform, args.dist, args.version)
            for label, data, is_main in binaries:
                problems.extend(check_binary(label, data, args.public_key, is_main))
        except (OSError, ValueError, subprocess.CalledProcessError, tarfile.TarError) as error:
            problems = [f"{platform}: cannot inspect build output: {error}"]
        status = "PASS" if not problems else "FAIL"
        if problems:
            failed.append(platform)
        report_lines.append(f"{platform}: {status}")
        report_lines.extend(f"  - {problem}" for problem in problems)

    report = "\n".join(report_lines) + "\n"
    gate_dir = args.dist / "gate"
    gate_dir.mkdir(parents=True, exist_ok=True)
    (gate_dir / "CONFORMANCE.txt").write_text(report, encoding="utf-8")
    sys.stdout.write(report)
    if failed:
        level = "ERROR" if args.strict else "WARNING"
        print(f"{level}: fork-conformance gate failed for {', '.join(failed)}", file=sys.stderr)
        return 1 if args.strict else 0
    print("Fork-conformance gate passed", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
