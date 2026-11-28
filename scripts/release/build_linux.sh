#!/usr/bin/env bash
# Build hai gói .deb Linux amd64 trên máy Linux từ xa qua SSH rồi chép kết quả về bằng scp.
# Máy từ xa chỉ cần git, python3, bwrap (bubblewrap) và user namespace không đặc quyền.
# Usage: build_linux.sh <version> <commit> <dist-dir>
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

HOST="${CROSSDESK_RELEASE_LINUX_SSH:-}"
[[ -n "$HOST" ]] || die "Set CROSSDESK_RELEASE_LINUX_SSH (for example user@linux-host)"
PORT="${CROSSDESK_RELEASE_LINUX_SSH_PORT:-22}"
# Đường dẫn tương đối được tính từ thư mục home của user SSH.
REMOTE_ROOT="${CROSSDESK_RELEASE_LINUX_DIR:-build/crossdesk}"
REMOTE_ROOT="${REMOTE_ROOT#\~/}"
[[ "$REMOTE_ROOT" =~ ^[A-Za-z0-9._/-]+$ ]] || die "Unsupported CROSSDESK_RELEASE_LINUX_DIR: $REMOTE_ROOT"
SKIP_APT_SIMULATE=0
[[ "${CROSSDESK_RELEASE_SKIP_APT_SIMULATE:-0}" == 1 ]] && SKIP_APT_SIMULATE=1

SSH_OPTS=(-o BatchMode=yes -o ServerAliveInterval=30 -o ServerAliveCountMax=20)
require_cmd ssh scp

log "Uploading Linux build scripts to $HOST:$REMOTE_ROOT/tools"
ssh "${SSH_OPTS[@]}" -p "$PORT" "$HOST" "mkdir -p '$REMOTE_ROOT/tools/linux'"
scp -q "${SSH_OPTS[@]}" -P "$PORT" "$SCRIPT_DIR/lib.sh" "$HOST:$REMOTE_ROOT/tools/lib.sh"
scp -q "${SSH_OPTS[@]}" -P "$PORT" \
    "$SCRIPT_DIR/linux/remote_build.sh" \
    "$SCRIPT_DIR/linux/build_in_rootfs.sh" \
    "$SCRIPT_DIR/linux/fetch_image_rootfs.py" \
    "$HOST:$REMOTE_ROOT/tools/linux/"
# Mục Changes nhúng vào hộp thoại Release Notes; remote_build.sh đưa nó vào rootfs build rồi xoá khi
# kết thúc. Tên riêng cho mỗi lần chạy để lần chạy khác trên cùng thư mục build (sẽ bị khoá từ chối)
# không ghi đè notes mà bản build đang chạy chưa đọc tới.
REMOTE_NOTES="release-notes-$(date +%Y%m%d%H%M%S)-$$.md"
scp -q "${SSH_OPTS[@]}" -P "$PORT" "$RELEASE_NOTES" "$HOST:$REMOTE_ROOT/tools/linux/$REMOTE_NOTES"

log "Running Linux build on $HOST"
ssh "${SSH_OPTS[@]}" -p "$PORT" "$HOST" \
    "CROSSDESK_RELEASE_SKIP_APT_SIMULATE=$SKIP_APT_SIMULATE bash '$REMOTE_ROOT/tools/linux/remote_build.sh' '$REMOTE_ROOT' '$VERSION' '$COMMIT' '$REMOTE_NOTES'" \
    </dev/null

STAGING="$(mktemp -d "${TMPDIR:-/tmp}/crossdesk-release-linux.XXXXXX")"
trap 'rm -rf "$STAGING"' EXIT
log "Copying Linux artifacts back"
scp -q "${SSH_OPTS[@]}" -P "$PORT" -r "$HOST:$REMOTE_ROOT/out/$VERSION" "$STAGING/"

mkdir -p "$DIST_DIR/gate/linux"
rm -f "$DIST_DIR/gate/linux/"*
cp "$STAGING/$VERSION/"apt-simulate-*.txt "$DIST_DIR/gate/linux/" 2>/dev/null || true
while IFS= read -r asset; do
    [[ -s "$STAGING/$VERSION/$asset" ]] || die "Linux build did not produce $asset"
    cp "$STAGING/$VERSION/$asset" "$DIST_DIR/"
    log "Linux package ready: $DIST_DIR/$asset ($(file_size "$DIST_DIR/$asset") bytes)"
done < <(platform_assets linux "$VERSION")
