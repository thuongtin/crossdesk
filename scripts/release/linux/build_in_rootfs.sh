#!/usr/bin/env bash
# Chạy bên trong rootfs của image build Linux (bwrap, uid 0 giả lập trong user
# namespace). Các bước bám sát job build-linux (amd64) trong .github/workflows/build.yml.
# Usage: build_in_rootfs.sh <version> <update-public-key> <release-notes.md>
set -euo pipefail

[[ $# -eq 3 ]] || { echo "Usage: $0 <version> <update-public-key> <release-notes.md>" >&2; exit 1; }
VERSION="$1"
PUBLIC_KEY="$2"
# Mục Changes của release notes, nhúng vào hộp thoại Release Notes của app.
RELEASE_NOTES="$3"
[[ -s "$RELEASE_NOTES" ]] || { echo "Missing release notes file: $RELEASE_NOTES" >&2; exit 1; }
OUT_DIR=/out
PACKAGES=(crossdesk crossdesk-virtual-desktop-xfce)

step() {
    printf '[linux %s] %s\n' "$(date +%H:%M:%S)" "$*" >&2
}

cd /workspace

step "Verifying build image"
EXPECTED_ARCH=amd64 bash docker/linux-build/verify-build-image.sh

# Giống bước "Ensure Linux desktop build dependencies" của CI: image ghim
# digest chưa có các gói dev này nên cài bổ sung một lần vào rootfs. Trong user
# namespace không chuyển sang user _apt được nên đặt APT::Sandbox::User=root.
PIPEWIRE_HEADERS=(
    /opt/crossdesk-pipewire-sdk/include/pipewire-0.3/pipewire/pipewire.h
    /opt/crossdesk-pipewire-sdk/include/spa-0.2/spa/param/video/format-utils.h
)
has_pipewire_headers() {
    local header
    for header in "${PIPEWIRE_HEADERS[@]}"; do
        [[ -f "$header" ]] || return 1
    done
}
if ! pkg-config --exists dbus-1 fribidi libdrm fuse fuse3; then
    step "Installing dbus, fribidi, libdrm and fuse development packages"
    export DEBIAN_FRONTEND=noninteractive
    apt-get -o APT::Sandbox::User=root -qq update
    apt-get -o APT::Sandbox::User=root install -y --no-install-recommends \
        libdbus-1-dev libfribidi-dev libdrm-dev libfuse-dev libfuse3-dev
    rm -rf /var/lib/apt/lists/*
fi
if ! has_pipewire_headers; then
    step "Installing the PipeWire SDK"
    install-crossdesk-pipewire-sdk
fi
pkg-config --exists dbus-1 fribidi libdrm fuse fuse3 ||
    { echo "Build image is missing dbus-1, fribidi, libdrm, fuse or fuse3 development files" >&2; exit 1; }
has_pipewire_headers || { echo "Build image is missing the PipeWire SDK headers" >&2; exit 1; }

APP_VERSION="${VERSION#v}"
step "Building CrossDesk $APP_VERSION for Linux amd64"
export CROSSDESK_UPDATE_PUBLIC_KEY="$PUBLIC_KEY"
# Chế độ verbose giúp Xmake 3.1.1 cài phụ thuộc tuần tự, giống CI upstream.
xmake f -v --CROSSDESK_VERSION="$APP_VERSION" --CROSSDESK_UPDATE_PUBLIC_KEY="$PUBLIC_KEY" \
    --CROSSDESK_RELEASE_NOTES="$RELEASE_NOTES" \
    --USE_CUDA=true --USE_WAYLAND=true --cc=gcc-10 --cxx=g++-10 --root -y
xmake b -vy --root crossdesk

step "Packaging .deb files"
bash ./apps/desktop/scripts/linux/pkg_amd64.sh "$VERSION"

for package in "${PACKAGES[@]}"; do
    file="/workspace/${package}-linux-amd64-${VERSION}.deb"
    [[ -s "$file" ]] || { echo "Missing or empty Linux package: $file" >&2; exit 1; }
    verify_dir="$(mktemp -d "/tmp/${package}-glibc.XXXXXX")"
    dpkg-deb --extract "$file" "$verify_dir"
    step "Checking glibc 2.31 baseline for $package"
    bash docker/linux-build/verify-glibc-baseline.sh "$verify_dir" 2.31
    rm -rf "$verify_dir"
done

# Kiểm tra apt giải được phụ thuộc trên Ubuntu 20.04 như CI. APT::Sandbox::User=root
# vì trong user namespace không chuyển sang user _apt được.
if [[ "${CROSSDESK_RELEASE_SKIP_APT_SIMULATE:-0}" != 1 ]]; then
    step "Simulating apt installation on Ubuntu 20.04"
    apt-get -o APT::Sandbox::User=root -qq update
    for package in "${PACKAGES[@]}"; do
        apt-get -o APT::Sandbox::User=root install --simulate \
            "/workspace/${package}-linux-amd64-${VERSION}.deb" > "$OUT_DIR/apt-simulate-${package}.txt"
    done
    rm -rf /var/lib/apt/lists/*
fi

for package in "${PACKAGES[@]}"; do
    mv "/workspace/${package}-linux-amd64-${VERSION}.deb" "$OUT_DIR/"
done
step "Linux packages ready in $OUT_DIR"
