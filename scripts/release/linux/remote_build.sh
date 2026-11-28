#!/usr/bin/env bash
# Chạy trên máy Linux x86_64 (được build_linux.sh gọi qua SSH). Chuẩn bị bản clone
# riêng của fork, dựng rootfs từ image build upstream đã ghim digest rồi build gói
# .deb bên trong bubblewrap, không cần Docker hay sudo.
# Usage: remote_build.sh <root> <version> <commit> <release-notes-name>
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/release/lib.sh
source "$SCRIPT_DIR/../lib.sh"

[[ $# -eq 4 ]] || die "Usage: $0 <root> <version> <commit> <release-notes-name>"
VERSION="$2"
COMMIT="$3"
# build_linux.sh chép mục Changes của release notes cạnh script này dưới tên riêng của lần chạy; thư
# mục script được gắn chỉ đọc vào /tmp/crossdesk-release trong rootfs. Tệp chỉ dùng cho lần chạy này.
NOTES_NAME="$4"
[[ "$NOTES_NAME" =~ ^release-notes-[A-Za-z0-9._-]+\.md$ ]] || die "Unsupported release notes name: $NOTES_NAME"
NOTES_FILE="$SCRIPT_DIR/$NOTES_NAME"
trap 'rm -f "$NOTES_FILE"' EXIT
validate_version "$VERSION" dry-run
[[ "$COMMIT" =~ ^[0-9a-f]{40}$ ]] || die "Commit must be a full 40-character SHA, got: $COMMIT"
[[ "$(uname -s)" == "Linux" && "$(uname -m)" == "x86_64" ]] || die "The Linux amd64 build must run on x86_64 Linux"
require_cmd git bwrap python3 flock sha256sum
[[ -s "$NOTES_FILE" ]] || die "Missing $NOTES_FILE"

mkdir -p "$1"
ROOT="$(cd "$1" && pwd)"
SRC_DIR="$ROOT/src"
OUT_DIR="$ROOT/out/$VERSION"
ROOTFS="$ROOT/rootfs/${LINUX_BUILD_IMAGE_DIGEST#sha256:}"

# Mỗi thư mục build chỉ cho một lần chạy tại một thời điểm.
exec 9>"$ROOT/.build.lock"
flock -n 9 || die "Another release build is already using $ROOT"

log "Preparing $SRC_DIR at $COMMIT"
prepare_clone "$SRC_DIR" "$COMMIT"

if [[ ! -d "$ROOTFS" ]]; then
    log "Fetching build image $LINUX_BUILD_IMAGE_REPO@$LINUX_BUILD_IMAGE_DIGEST"
    python3 -I "$SCRIPT_DIR/fetch_image_rootfs.py" \
        --repository "$LINUX_BUILD_IMAGE_REPO" \
        --digest "$LINUX_BUILD_IMAGE_DIGEST" \
        --dest "$ROOTFS" \
        --cache "$ROOT/image-cache" </dev/null
fi
[[ "$(cat "$ROOTFS/.crossdesk-image-digest" 2>/dev/null)" == "$LINUX_BUILD_IMAGE_DIGEST" ]] ||
    die "Rootfs $ROOTFS does not match $LINUX_BUILD_IMAGE_DIGEST; delete it and rerun"

rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR"

# Biến môi trường lấy đúng từ cấu hình image (PATH, XMAKE_GLOBALDIR, CUDA_PATH, CC, CXX...).
env_args=()
while IFS= read -r line; do
    [[ -n "$line" ]] || continue
    env_args+=(--setenv "${line%%=*}" "${line#*=}")
done <"$ROOTFS/.crossdesk-image-env"

resolv_conf="$(readlink -f /etc/resolv.conf)"
log "Building inside bubblewrap (rootfs ${LINUX_BUILD_IMAGE_DIGEST:0:19}...)"
bwrap \
    --bind "$ROOTFS" / \
    --dev /dev \
    --tmpfs /dev/shm \
    --proc /proc \
    --tmpfs /tmp \
    --ro-bind "$resolv_conf" /etc/resolv.conf \
    --ro-bind /etc/hosts /etc/hosts \
    --bind "$SRC_DIR" /workspace \
    --bind "$OUT_DIR" /out \
    --ro-bind "$SCRIPT_DIR" /tmp/crossdesk-release \
    --unshare-user --uid 0 --gid 0 \
    --unshare-pid --unshare-ipc --unshare-uts --hostname crossdesk-build \
    --die-with-parent --new-session \
    --clearenv \
    --setenv HOME /root \
    --setenv USER root \
    --setenv TERM dumb \
    "${env_args[@]}" \
    --setenv CROSSDESK_RELEASE_SKIP_APT_SIMULATE "${CROSSDESK_RELEASE_SKIP_APT_SIMULATE:-0}" \
    --chdir /workspace \
    /bin/bash /tmp/crossdesk-release/build_in_rootfs.sh "$VERSION" "$FORK_UPDATE_PUBLIC_KEY" \
    "/tmp/crossdesk-release/$NOTES_NAME" </dev/null

while IFS= read -r asset; do
    [[ -s "$OUT_DIR/$asset" ]] || die "Linux build did not produce $asset"
    log "$asset $(file_size "$OUT_DIR/$asset") bytes sha256=$(sha256_of "$OUT_DIR/$asset")"
done < <(platform_assets linux "$VERSION")
