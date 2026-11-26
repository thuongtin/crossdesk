#!/usr/bin/env bash
# Build, ký và đóng gói bản macOS arm64 ngay trên máy Mac đang chạy release.
# Usage: build_macos.sh <version> <commit> <dist-dir>
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/release/lib.sh
source "$SCRIPT_DIR/lib.sh"

[[ $# -eq 3 ]] || die "Usage: $0 <version> <commit> <dist-dir>"
VERSION="$1"
COMMIT="$2"
DIST_DIR="$3"
MODE="${CROSSDESK_RELEASE_MODE:-dry-run}"
BUILD_ROOT="${CROSSDESK_RELEASE_MACOS_DIR:-$HOME/build/crossdesk}"
SRC_DIR="$BUILD_ROOT/src"

[[ "$(uname -s)" == "Darwin" && "$(uname -m)" == "arm64" ]] ||
    die "The macOS arm64 build must run on an Apple Silicon Mac"
# pkg-config phải có sẵn như trên runner GitHub: meson của gói libsoup dùng nó để tìm glib.
require_cmd git xmake xcrun pkgutil productbuild codesign security pkg-config

SIGNED=0
NOTARIZED=0
if [[ -n "${CROSSDESK_MACOS_SIGN_IDENTITY:-}" || -n "${CROSSDESK_MACOS_INSTALLER_IDENTITY:-}" ]]; then
    SIGNED=1
fi
if [[ -n "${CROSSDESK_MACOS_NOTARY_PROFILE:-}" ]]; then
    NOTARIZED=1
fi

if [[ "$MODE" == "publish" ]]; then
    for variable in CROSSDESK_MACOS_SIGN_IDENTITY CROSSDESK_MACOS_INSTALLER_IDENTITY \
        CROSSDESK_MACOS_NOTARY_PROFILE; do
        [[ -n "${!variable:-}" ]] || die "$variable must be set for --publish"
    done
fi

# Chỉ kiểm tra tên identity có trong keychain, không đọc hay xuất khoá.
if [[ "$SIGNED" == 1 ]]; then
    keychain_args=()
    if [[ -n "${CROSSDESK_MACOS_KEYCHAIN:-}" ]]; then
        keychain_args+=("$CROSSDESK_MACOS_KEYCHAIN")
    fi
    identities="$(security find-identity -v ${keychain_args[@]+"${keychain_args[@]}"})"
    for identity in "${CROSSDESK_MACOS_SIGN_IDENTITY:-}" "${CROSSDESK_MACOS_INSTALLER_IDENTITY:-}"; do
        [[ -n "$identity" ]] || die "Set both CROSSDESK_MACOS_SIGN_IDENTITY and CROSSDESK_MACOS_INSTALLER_IDENTITY"
        grep -Fq "\"$identity\"" <<<"$identities" ||
            die "Signing identity not found in keychain: $identity"
    done
else
    log "No Developer ID identities configured: building an ad-hoc signed app in an unsigned pkg"
fi

log "Preparing $SRC_DIR at $COMMIT"
prepare_clone "$SRC_DIR" "$COMMIT"
cd "$SRC_DIR"

export MACOSX_DEPLOYMENT_TARGET="$MACOS_DEPLOYMENT_TARGET"
export CROSSDESK_UPDATE_PUBLIC_KEY="$FORK_UPDATE_PUBLIC_KEY"

# Phiên bản nhúng trong app không có tiền tố v, giống bản fork đã phát hành;
# tên gói vẫn giữ tiền tố v theo tag.
APP_VERSION="${VERSION#v}"
log "Building CrossDesk $APP_VERSION for macOS arm64 (Xcode SDK $(xcrun --sdk macosx --show-sdk-version))"
xmake repo -u
xmake f -y -p macosx -a arm64 -m release \
    --target_minver="$MACOS_DEPLOYMENT_TARGET" \
    --CROSSDESK_VERSION="$APP_VERSION" \
    --CROSSDESK_UPDATE_PUBLIC_KEY="$FORK_UPDATE_PUBLIC_KEY" \
    --USE_CUDA=false
xmake b -vy crossdesk

log "Packaging (signed=$SIGNED, notarized=$NOTARIZED)"
bash ./apps/desktop/scripts/macos/pkg_arm64.sh "$VERSION"

PACKAGE="$SRC_DIR/$(asset_name macos "$VERSION")"
[[ -s "$PACKAGE" ]] || die "Missing package: $PACKAGE"

VERIFY_DIR="$(mktemp -d "${TMPDIR:-/tmp}/crossdesk-release-pkg.XXXXXX")"
trap 'rm -rf "$VERIFY_DIR"' EXIT
pkgutil --expand-full "$PACKAGE" "$VERIFY_DIR/expanded"
APP_PATH="$(find "$VERIFY_DIR/expanded" -type d -name 'CrossDesk.app' -print -quit)"
[[ -n "$APP_PATH" ]] || die "CrossDesk.app is missing from $PACKAGE"
bash ./apps/desktop/scripts/macos/verify_app.sh "$APP_PATH" arm64

if [[ "$SIGNED" == 1 ]]; then
    pkgutil --check-signature "$PACKAGE"
fi
if [[ "$NOTARIZED" == 1 ]]; then
    xcrun stapler validate "$PACKAGE"
    spctl --assess --type install "$PACKAGE"
fi

cp "$PACKAGE" "$DIST_DIR/"
printf 'signed=%s\nnotarized=%s\nsdk=%s\n' "$SIGNED" "$NOTARIZED" \
    "$(xcrun --sdk macosx --show-sdk-version)" > "$DIST_DIR/gate/macos-signing.txt"
log "macOS package ready: $DIST_DIR/$(basename "$PACKAGE") ($(file_size "$PACKAGE") bytes)"
