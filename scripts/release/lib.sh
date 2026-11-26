# shellcheck shell=bash disable=SC2034
# Hàm và hằng số dùng chung cho quy trình release CrossDesk Postcard.
# File này được source bởi release.sh, các script build từng nền tảng và cả
# script chạy trên máy Linux từ xa, nên chỉ được dùng bash và coreutils.

# Fork là nơi duy nhất quy trình này đọc mã nguồn và ghi release.
FORK_REPO="thuongtin/crossdesk"
FORK_URL="https://github.com/thuongtin/crossdesk"
FEED_BRANCH="desktop-updates"
FEED_FILE="crossdesk-desktop-update.json"

# Khoá công khai Ed25519 của kênh cập nhật fork, được nhúng vào mọi bản build.
# Khoá bí mật tương ứng không bao giờ nằm trong repo.
FORK_UPDATE_PUBLIC_KEY="48f5a72e6310045ced34bbf325ebfd282d515067bcd7c58e5ab8c139a185012d"

# Image build Linux của upstream (Ubuntu 20.04, glibc 2.31, CUDA 12.6.3, xmake 3.1.1).
# Ghim theo digest của manifest linux/amd64 để bản build lặp lại được.
LINUX_BUILD_IMAGE_REPO="crossdesk/ubuntu20.04"
LINUX_BUILD_IMAGE_TAG="buildenv-cuda12.6.3-gcc10-cmake3.31.6-pipewire0.3.48-xmake3.1.1-rust1.92.0"
LINUX_BUILD_IMAGE_DIGEST="sha256:ff9ca3bed9d8b1747c28452f447252c5ae76a1d101f37f976bbc51c776657d46"

MACOS_DEPLOYMENT_TARGET="14.0"
ALL_PLATFORMS="macos windows linux"

log() {
    printf '[release %s] %s\n' "$(date +%H:%M:%S)" "$*" >&2
}

die() {
    log "ERROR: $*"
    exit 1
}

require_cmd() {
    local name
    for name in "$@"; do
        command -v "$name" >/dev/null 2>&1 || die "Required command is missing: $name"
    done
}

# Kiểm tra chuỗi phiên bản. Bản publish phải đúng dạng v<base>-<build>-<YYYYMMDD>;
# bản dry-run cho phép hậu tố chữ (ví dụ v1.7.7-3-dryrun) để không bị nhầm với bản thật.
validate_version() {
    local version="$1" mode="$2"
    if [[ "$mode" == "publish" ]]; then
        [[ "$version" =~ ^v[0-9]+\.[0-9]+\.[0-9]+-[0-9]+-[0-9]{8}$ ]] ||
            die "Publish version must look like v1.7.7-3-20261126, got: $version"
    else
        [[ "$version" =~ ^v[0-9]+\.[0-9]+\.[0-9]+-[0-9]+-[0-9A-Za-z]+$ ]] ||
            die "Version must look like v1.7.7-3-20261126 or v1.7.7-3-dryrun, got: $version"
    fi
}

# Tên asset trên GitHub release. Các tên này khớp với ExpectedFilename() trong
# trình cập nhật của ứng dụng, nên không được đổi tuỳ ý.
asset_name() {
    local platform="$1" version="$2"
    case "$platform" in
        macos) echo "crossdesk-macos-arm64-${version}.pkg" ;;
        windows) echo "crossdesk-win-x64-${version}.exe" ;;
        linux) echo "crossdesk-linux-amd64-${version}.deb" ;;
        linux-virtual-desktop) echo "crossdesk-virtual-desktop-xfce-linux-amd64-${version}.deb" ;;
        *) die "Unknown asset platform: $platform" ;;
    esac
}

# Danh sách asset mà một nền tảng build ra.
platform_assets() {
    local platform="$1" version="$2"
    case "$platform" in
        macos) asset_name macos "$version" ;;
        windows) asset_name windows "$version" ;;
        linux)
            asset_name linux "$version"
            asset_name linux-virtual-desktop "$version"
            ;;
        *) die "Unknown platform: $platform" ;;
    esac
}

# Nạp cấu hình máy build (host SSH, thư mục, tên identity ký). File cấu hình chỉ
# chứa tên và đường dẫn, không chứa mật khẩu hay khoá.
load_release_config() {
    local config="${CROSSDESK_RELEASE_CONFIG:-$HOME/.config/crossdesk/release.env}"
    if [[ -f "$config" ]]; then
        # set -a để các script build con (macOS, Windows, Linux) nhận được biến.
        set -a
        # shellcheck disable=SC1090
        source "$config"
        set +a
        log "Loaded release config: $config"
    fi
}

# Chuẩn bị bản clone riêng cho việc build: clone từ fork nếu chưa có, kiểm tra
# remote đúng là fork, checkout đúng commit rồi xoá sạch mọi file không được theo dõi.
prepare_clone() {
    local dir="$1" commit="$2"
    if [[ ! -d "$dir/.git" ]]; then
        mkdir -p "$(dirname "$dir")"
        git clone --no-checkout "$FORK_URL" "$dir"
    fi
    local url
    url="$(git -C "$dir" remote get-url origin)"
    if [[ "$url" != "$FORK_URL" && "$url" != "$FORK_URL.git" ]]; then
        die "Build clone $dir points at $url, expected $FORK_URL"
    fi
    git -C "$dir" fetch --force --prune origin '+refs/heads/*:refs/remotes/origin/*'
    if ! git -C "$dir" cat-file -e "${commit}^{commit}" 2>/dev/null; then
        git -C "$dir" fetch origin "$commit"
    fi
    git -C "$dir" checkout --force --detach "$commit"
    git -C "$dir" submodule sync --recursive
    git -C "$dir" submodule update --init --recursive --force
    git -C "$dir" clean -ffdxq
    git -C "$dir" submodule foreach --quiet --recursive git clean -ffdxq
    [[ "$(git -C "$dir" rev-parse HEAD)" == "$commit" ]] || die "Checkout of $commit failed in $dir"
    [[ -z "$(git -C "$dir" status --porcelain --ignore-submodules=none)" ]] ||
        die "Build clone $dir is not clean after checkout"
}

sha256_of() {
    if command -v sha256sum >/dev/null 2>&1; then
        sha256sum "$1" | awk '{print $1}'
    else
        shasum -a 256 "$1" | awk '{print $1}'
    fi
}

file_size() {
    wc -c < "$1" | tr -d '[:space:]'
}
