#!/usr/bin/env bash
# Build bộ cài Windows x64 trên máy Windows từ xa qua SSH rồi chép kết quả về bằng scp.
# Usage: build_windows.sh <version> <commit> <dist-dir>
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/release/lib.sh
source "$SCRIPT_DIR/lib.sh"

[[ $# -eq 3 ]] || die "Usage: $0 <version> <commit> <dist-dir>"
VERSION="$1"
COMMIT="$2"
DIST_DIR="$3"
RELEASE_NOTES="$DIST_DIR/INSTALLED_RELEASE_NOTES.md"
[[ -s "$RELEASE_NOTES" ]] || die "Missing $RELEASE_NOTES; run the build through release.sh"

HOST="${CROSSDESK_RELEASE_WINDOWS_SSH:-}"
[[ -n "$HOST" ]] || die "Set CROSSDESK_RELEASE_WINDOWS_SSH (for example user@windows-host)"
PORT="${CROSSDESK_RELEASE_WINDOWS_SSH_PORT:-22}"
# Thư mục gốc trên Windows, dùng dấu gạch chéo ngược cho PowerShell và gạch chéo xuôi cho scp.
REMOTE_ROOT="${CROSSDESK_RELEASE_WINDOWS_DIR:-C:\\build\\crossdesk}"
REMOTE_ROOT_SCP="${REMOTE_ROOT//\\//}"

SSH_OPTS=(-o BatchMode=yes -o ServerAliveInterval=30 -o ServerAliveCountMax=20)
require_cmd ssh scp
# Tên notes riêng cho mỗi lần chạy để lần chạy khác trên cùng thư mục build không ghi đè notes mà bản
# build đang chạy chưa đọc tới; xoá trên máy Windows khi script kết thúc.
REMOTE_NOTES="release-notes-$(date +%Y%m%d%H%M%S)-$$.md"
STAGING=""
cleanup() {
    ssh "${SSH_OPTS[@]}" -p "$PORT" "$HOST" \
        "powershell -NoProfile -Command \"Remove-Item -Force -ErrorAction SilentlyContinue '$REMOTE_ROOT\\tools\\$REMOTE_NOTES'\"" \
        </dev/null || true
    [[ -z "$STAGING" ]] || rm -rf "$STAGING"
}
trap cleanup EXIT

log "Uploading Windows build script to $HOST:$REMOTE_ROOT"
ssh "${SSH_OPTS[@]}" -p "$PORT" "$HOST" \
    "powershell -NoProfile -Command \"New-Item -ItemType Directory -Force -Path '$REMOTE_ROOT\\tools' | Out-Null\""
scp "${SSH_OPTS[@]}" -P "$PORT" "$SCRIPT_DIR/windows/build.ps1" "$HOST:$REMOTE_ROOT_SCP/tools/build.ps1"
# Mục Changes nhúng vào hộp thoại Release Notes của bản build này.
scp "${SSH_OPTS[@]}" -P "$PORT" "$RELEASE_NOTES" "$HOST:$REMOTE_ROOT_SCP/tools/$REMOTE_NOTES"

log "Running Windows build on $HOST"
ssh "${SSH_OPTS[@]}" -p "$PORT" "$HOST" \
    "powershell -NoProfile -ExecutionPolicy Bypass -File \"$REMOTE_ROOT\\tools\\build.ps1\" -Root \"$REMOTE_ROOT\" -RepoUrl \"$FORK_URL\" -Commit $COMMIT -Version $VERSION -PublicKey $FORK_UPDATE_PUBLIC_KEY -ReleaseNotes \"$REMOTE_ROOT\\tools\\$REMOTE_NOTES\""

STAGING="$(mktemp -d "${TMPDIR:-/tmp}/crossdesk-release-windows.XXXXXX")"
log "Copying Windows artifacts back"
scp "${SSH_OPTS[@]}" -P "$PORT" -r "$HOST:$REMOTE_ROOT_SCP/out/$VERSION" "$STAGING/"

INSTALLER="$(asset_name windows "$VERSION")"
[[ -s "$STAGING/$VERSION/$INSTALLER" ]] || die "Windows build did not produce $INSTALLER"
mkdir -p "$DIST_DIR/gate/windows"
rm -f "$DIST_DIR/gate/windows/"*
cp "$STAGING/$VERSION/gate/"* "$DIST_DIR/gate/windows/"
cp "$STAGING/$VERSION/windows-signing.txt" "$DIST_DIR/gate/windows-signing.txt"
cp "$STAGING/$VERSION/$INSTALLER" "$DIST_DIR/"
log "Windows installer ready: $DIST_DIR/$INSTALLER ($(file_size "$DIST_DIR/$INSTALLER") bytes)"
