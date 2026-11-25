#!/usr/bin/env python3
"""Generate, but do not publish, a signed macOS ARM64 update manifest."""

import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import plistlib
import re
import stat
import subprocess
import tempfile


REPOSITORY = "https://github.com/thuongtin/crossdesk"
PLATFORM = "macos-arm64"
ROOT = Path(__file__).resolve().parents[4]
SPEC = importlib.util.spec_from_file_location(
    "update_version_manifest", ROOT / ".github/scripts/update_version_manifest.py")
SIGNER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(SIGNER)


def check_package(package, version, public_key):
    expected = f"crossdesk-macos-arm64-v{version}.pkg"
    if package.name != expected:
        raise ValueError(f"Package must be named {expected}")
    spec = SIGNER.version_fields(version)
    if not spec["patch"] or not spec["date"]:
        raise ValueError("Expected a version of the form MAJOR.MINOR.PATCH-BUILD-YYYYMMDD")
    wanted_bundle_version = f'{spec["base"]}.{spec["date"]}.{spec["patch"]}'
    subprocess.run(["/usr/sbin/spctl", "--assess", "--type", "install", str(package)], check=True)
    subprocess.run(["xcrun", "stapler", "validate", str(package)], check=True, stdout=subprocess.DEVNULL)
    with tempfile.TemporaryDirectory(prefix="crossdesk-fork-update-") as directory:
        expanded = Path(directory) / "expanded"
        subprocess.run(["/usr/sbin/pkgutil", "--expand-full", str(package), str(expanded)], check=True)
        apps = list(expanded.glob("*/Payload/CrossDesk.app"))
        if len(apps) != 1:
            raise ValueError("Expected a single CrossDesk.app payload")
        app = apps[0]
        info = plistlib.loads((app / "Contents/Info.plist").read_bytes())
        if info["CFBundleVersion"] != wanted_bundle_version:
            raise ValueError(f"Package CFBundleVersion must be {wanted_bundle_version}")
        subprocess.run(["/usr/bin/codesign", "--verify", "--deep", "--strict", str(app)], check=True)
        for name in ("CrossDesk", "crossdesk_updater"):
            binary = str(app / "Contents/MacOS" / name)
            arch = subprocess.check_output(["/usr/bin/lipo", "-archs", binary], text=True)
            if arch.strip() != "arm64":
                raise ValueError(f"Unexpected architecture in {name}")
            strings = subprocess.check_output(["/usr/bin/strings", "-a", binary]).splitlines()
            if public_key.encode() not in strings:
                raise ValueError(f"Package update key is missing or mismatched in {name}")
            if b"https://version.crossdesk.cn" in strings or b"https://downloads.crossdesk.cn" in strings:
                raise ValueError(f"Upstream update origin found in {name}")
            if name == "CrossDesk" and (
                b"/thuongtin/crossdesk/desktop-updates/crossdesk-desktop-update.json" not in strings
                or b"crossdesk.ecom.io.vn" not in strings
                or b"api.crossdesk.cn" in strings
            ):
                raise ValueError("Package fork channel or default signaling hostname is wrong")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--package", required=True, type=Path)
    parser.add_argument("--tag", required=True)
    parser.add_argument("--notes", required=True, type=Path)
    parser.add_argument("--public-key", required=True, type=Path)
    parser.add_argument("--private-key", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    public_key = args.public_key.read_text().strip()
    if not re.fullmatch("[0-9a-f]{64}", public_key):
        parser.error("Expected a 32-byte lowercase hex public key")
    st = args.private_key.lstat()
    if not stat.S_ISREG(st.st_mode) or st.st_mode & 0o077:
        parser.error("Private key must be a regular file accessible only by its owner")
    if not args.tag.startswith("v") or args.tag.startswith("vv"):
        parser.error("Expected a tag in the form v<version>")
    version = args.tag[1:]
    package = args.package.resolve()
    check_package(package, version, public_key)
    with package.open("rb") as source:
        digest = hashlib.file_digest(source, "sha256").hexdigest()
    payload = {
        "schema": 1, "platform": PLATFORM, "version": version, "filename": package.name,
        "url": f"{REPOSITORY}/releases/download/{args.tag}/{package.name}",
        "size": package.stat().st_size, "sha256": digest,
    }
    signed = SIGNER.sign_update(
        json.dumps(payload, sort_keys=True, separators=(",", ":")),
        args.private_key.read_text(), public_key)
    info = SIGNER.version_info(version)
    manifest = dict(info, releaseName=f"CrossDesk {args.tag}",
                    releaseNotes=args.notes.read_text(),
                    tagName=args.tag,
                    downloads={PLATFORM: {
                        "version": version, "releaseDate": info["releaseDate"],
                        "filename": payload["filename"], "url": payload["url"],
                        "update": signed,
                    }})
    with args.output.open("x", encoding="utf-8") as destination:
        json.dump(manifest, destination, ensure_ascii=False, indent=2)
        destination.write("\n")
    print(f"Prepared {args.output.name}: {args.tag} {PLATFORM} SHA256={digest}")


if __name__ == "__main__":
    main()
