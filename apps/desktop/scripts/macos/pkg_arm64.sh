#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../../.." && pwd)"
APP_NAME="crossdesk"
APP_NAME_UPPER="CrossDesk"
EXECUTABLE_PATH="${PROJECT_ROOT}/build/macosx/arm64/release/crossdesk"
PLATFORM="macos"
ARCH="arm64"
BINARY_ARCH="arm64"
IDENTIFIER="cn.crossdesk.app"
ICON_PATH="${PROJECT_ROOT}/apps/desktop/resources/macos/crossdesk.icns"
MACOS_MIN_VERSION="14.0"

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 <version>" >&2
    exit 2
fi

# For distribution, configure both full Developer ID certificate names.
# Setting CROSSDESK_MACOS_NOTARY_PROFILE also notarizes and staples the pkg.
PRODUCT_SIGN_ARGS=()
if [[ -n "${CROSSDESK_MACOS_INSTALLER_IDENTITY:-}" || -n "${CROSSDESK_MACOS_NOTARY_PROFILE:-}" ||
      "${CROSSDESK_MACOS_SIGN_IDENTITY:--}" != "-" ]]; then
    if [[ "${CROSSDESK_MACOS_SIGN_IDENTITY:-}" != "Developer ID Application: "* ||
          "${CROSSDESK_MACOS_INSTALLER_IDENTITY:-}" != "Developer ID Installer: "* ]]; then
        echo "Signed packaging requires CROSSDESK_MACOS_SIGN_IDENTITY and CROSSDESK_MACOS_INSTALLER_IDENTITY." >&2
        exit 1
    fi
    if [[ "${CROSSDESK_MACOS_SIGN_IDENTITY##* (}" != "${CROSSDESK_MACOS_INSTALLER_IDENTITY##* (}" ]]; then
        echo "Application and installer certificates must belong to the same team." >&2
        exit 1
    fi
    PRODUCT_SIGN_ARGS+=(--sign "$CROSSDESK_MACOS_INSTALLER_IDENTITY" --timestamp)
    if [[ -n "${CROSSDESK_MACOS_KEYCHAIN:-}" ]]; then
        PRODUCT_SIGN_ARGS+=(--keychain "$CROSSDESK_MACOS_KEYCHAIN")
    fi
fi

cd "$PROJECT_ROOT"

normalize_app_version() {
    local input="$1"
    local prefix=""
    local body="$input"

    if [[ "$body" == v* ]]; then
        prefix="v"
        body="${body#v}"
    fi

    if [[ "$body" =~ ^([0-9]+(\.[0-9]+){1,3})-([0-9]{8})-([0-9]+)$ ]]; then
        echo "${prefix}${BASH_REMATCH[1]}-${BASH_REMATCH[4]}-${BASH_REMATCH[3]}"
    else
        echo "$input"
    fi
}

APP_VERSION="$(normalize_app_version "$1")"

# macOS Installer và trình cập nhật so phiên bản theo CFBundleVersion và pkg-ref, nên tag
# v<base>-<build>-<YYYYMMDD> phải thành <base>.<YYYYMMDD>.<build> (ví dụ 1.7.7.20261125.2),
# còn chuỗi hiển thị CFBundleShortVersionString chỉ là <base>. Tag không có ngày (bản thử)
# giữ nguyên chuỗi nhưng bỏ tiền tố "v".
APP_VERSION_BODY="${APP_VERSION#v}"
if [[ "$APP_VERSION_BODY" =~ ^([0-9]+(\.[0-9]+){1,3})-([0-9]+)-([0-9]{8})$ ]]; then
    BUNDLE_VERSION="${BASH_REMATCH[1]}.${BASH_REMATCH[4]}.${BASH_REMATCH[3]}"
    BUNDLE_SHORT_VERSION="${BASH_REMATCH[1]}"
else
    BUNDLE_VERSION="$APP_VERSION_BODY"
    BUNDLE_SHORT_VERSION="$APP_VERSION_BODY"
fi

APP_BUNDLE="${APP_NAME_UPPER}.app"
CONTENTS_DIR="${APP_BUNDLE}/Contents"
MACOS_DIR="${CONTENTS_DIR}/MacOS"
RESOURCES_DIR="${CONTENTS_DIR}/Resources"
INSTALLER_TITLE="${APP_NAME_UPPER} ${APP_VERSION}"

PKG_NAME="${APP_NAME}-${PLATFORM}-${ARCH}-${APP_VERSION}.pkg"
DMG_NAME="${APP_NAME}-${PLATFORM}-${ARCH}-${APP_VERSION}.dmg"
VOL_NAME="Install ${APP_NAME_UPPER}"

echo "delete old files"
rm -rf "${APP_BUNDLE}" "${PKG_NAME}" "${DMG_NAME}" build_pkg_temp build_pkg_scripts CrossDesk_dmg_temp

mkdir -p build_pkg_temp
mkdir -p "${MACOS_DIR}" "${RESOURCES_DIR}"

cp "${EXECUTABLE_PATH}" "${MACOS_DIR}/${APP_NAME_UPPER}"
cp "$(dirname "${EXECUTABLE_PATH}")/crossdesk_updater" "${MACOS_DIR}/crossdesk_updater"
chmod +x "${MACOS_DIR}/crossdesk_updater"
chmod +x "${MACOS_DIR}/${APP_NAME_UPPER}"

if [ -f "${ICON_PATH}" ]; then
    cp "${ICON_PATH}" "${RESOURCES_DIR}/crossedesk.icns"
    ICON_KEY="<key>CFBundleIconFile</key><string>crossedesk.icns</string>"
else
    ICON_KEY=""
fi

echo "generate Info.plist"
cat > "${CONTENTS_DIR}/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleName</key>
    <string>${APP_NAME_UPPER}</string>
    <key>CFBundleDisplayName</key>
    <string>${APP_NAME_UPPER}</string>
    <key>CFBundleIdentifier</key>
    <string>${IDENTIFIER}</string>
    <key>CFBundleVersion</key>
    <string>${BUNDLE_VERSION}</string>
    <key>CFBundleShortVersionString</key>
    <string>${BUNDLE_SHORT_VERSION}</string>
    <key>CFBundleExecutable</key>
    <string>${APP_NAME_UPPER}</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    ${ICON_KEY}
    <key>LSMinimumSystemVersion</key>
    <string>${MACOS_MIN_VERSION}</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>NSCameraUsageDescription</key>
    <string>Ứng dụng cần truy cập camera</string>
    <key>NSMicrophoneUsageDescription</key>
    <string>Ứng dụng cần truy cập micro</string>
    <key>NSAppleEventsUsageDescription</key>
    <string>Ứng dụng cần gửi sự kiện Apple</string>
    <key>NSScreenCaptureUsageDescription</key>
    <string>Ứng dụng cần quyền ghi màn hình để thu nội dung màn hình</string>
</dict>
</plist>
EOF

xattr -cr "${APP_BUNDLE}" 2>/dev/null || true
find "${APP_BUNDLE}" -name '._*' -delete

"${SCRIPT_DIR}/bundle_app.sh" "${APP_BUNDLE}" "${BINARY_ARCH}" "$(dirname "${EXECUTABLE_PATH}")/libslint_cpp.dylib"
"${SCRIPT_DIR}/verify_app.sh" "${APP_BUNDLE}" "${BINARY_ARCH}"

echo ".app created successfully."

PKG_SCRIPT_ARGS=()
if [[ "${CROSSDESK_MACOS_SIGN_IDENTITY:--}" != "-" ]]; then
    mkdir -p build_pkg_scripts
    cp "${SCRIPT_DIR}/tcc_migration_preinstall.sh" build_pkg_scripts/preinstall
    chmod +x build_pkg_scripts/preinstall
    PKG_SCRIPT_ARGS=(--scripts build_pkg_scripts)
fi

mkdir -p build_pkg_resources
cat > build_pkg_resources/welcome.html <<EOF
<!doctype html>
<html lang="vi">
<head>
    <meta charset="utf-8">
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Helvetica Neue", sans-serif;
            font-size: 13px;
            line-height: 1.5;
            color: #222;
            margin: 0;
            padding: 0;
        }
        h1 {
            font-size: 20px;
            font-weight: 600;
            margin: 0 0 12px;
        }
        p {
            margin: 0 0 10px;
        }
    </style>
</head>
<body>
    <h1>Chào mừng bạn cài đặt ${INSTALLER_TITLE}</h1>
    <p>CrossDesk yêu cầu macOS ${MACOS_MIN_VERSION} trở lên và sẽ được cài vào thư mục “Ứng dụng”.</p>
    <p>Khi khởi động lần đầu, CrossDesk sẽ hướng dẫn bạn cấp các quyền cần thiết trong cài đặt hệ thống, gồm trợ năng, ghi màn hình và ghi âm thanh hệ thống.</p>
    <p>Nâng cấp bình thường từ bản đã ký chính thức sẽ giữ quyền hiện có. Khi nâng cấp từ bản cũ chưa ký hoặc ký ad-hoc, hãy thoát bản cũ rồi mở bản mới. Bản mới sẽ di chuyển quyền ghi màn hình và trợ năng của người dùng hiện tại, đồng thời hướng dẫn bạn cấp lại quyền. Nếu gặp bất thường, bạn có thể thử lại trong “Cài đặt → Quyền hệ thống” hoặc xử lý sau.</p>
    <p>Sau khi cài xong, hãy mở CrossDesk từ thư mục “Ứng dụng”.</p>
</body>
</html>
EOF

echo "building pkg..."
pkgbuild \
  --identifier "${IDENTIFIER}" \
  --version "${BUNDLE_VERSION}" \
  --install-location "/Applications" \
  --component "${APP_BUNDLE}" \
  ${PKG_SCRIPT_ARGS[@]+"${PKG_SCRIPT_ARGS[@]}"} \
  build_pkg_temp/${APP_NAME}-component.pkg

cat > build_pkg_temp/Distribution <<EOF
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="2">
    <title>${INSTALLER_TITLE}</title>
    <welcome file="welcome.html" mime-type="text/html"/>
    <options customize="never" require-scripts="false" hostArchitectures="arm64"/>
    <volume-check>
        <allowed-os-versions>
            <os-version min="${MACOS_MIN_VERSION}"/>
        </allowed-os-versions>
    </volume-check>
    <choices-outline>
        <line choice="default">
            <line choice="${IDENTIFIER}"/>
        </line>
    </choices-outline>
    <choice id="default" title="${INSTALLER_TITLE}"/>
    <choice id="${IDENTIFIER}" title="${INSTALLER_TITLE}" visible="false">
        <pkg-ref id="${IDENTIFIER}"/>
    </choice>
    <pkg-ref id="${IDENTIFIER}" version="${BUNDLE_VERSION}" onConclusion="none">crossdesk-component.pkg</pkg-ref>
</installer-gui-script>
EOF

productbuild \
  ${PRODUCT_SIGN_ARGS[@]+"${PRODUCT_SIGN_ARGS[@]}"} \
  --distribution build_pkg_temp/Distribution \
  --package-path build_pkg_temp \
  --resources build_pkg_resources \
  "${PKG_NAME}"

echo "PKG package created: ${PKG_NAME}"

if [[ -n "${CROSSDESK_MACOS_INSTALLER_IDENTITY:-}" ]]; then
    pkgutil --check-signature "${PKG_NAME}"
fi
if [[ -n "${CROSSDESK_MACOS_NOTARY_PROFILE:-}" ]]; then
    bash "${SCRIPT_DIR}/notarize_pkg.sh" "${PKG_NAME}"
elif [[ -n "${CROSSDESK_MACOS_INSTALLER_IDENTITY:-}" ]]; then
    echo "Package is signed but not notarized. Set CROSSDESK_MACOS_NOTARY_PROFILE before public distribution."
fi

# Set custom icon for PKG file
if [ -f "${ICON_PATH}" ]; then
    echo "Setting custom icon for PKG file..."
    # Create a temporary iconset from icns
    TEMP_ICON_DIR=$(mktemp -d)
    cp "${ICON_PATH}" "${TEMP_ICON_DIR}/icon.icns"
    
    # Use sips to create a png from icns for the icon
    sips -s format png "${TEMP_ICON_DIR}/icon.icns" --out "${TEMP_ICON_DIR}/icon.png" 2>/dev/null || true
    
    # Method: Use osascript to set file icon (works on macOS)
    osascript <<APPLESCRIPT
use framework "Foundation"
use framework "AppKit"

set iconPath to POSIX file "${TEMP_ICON_DIR}/icon.icns"
set targetPath to POSIX file "$(pwd)/${PKG_NAME}"

set iconImage to current application's NSImage's alloc()'s initWithContentsOfFile:(POSIX path of iconPath)
set workspace to current application's NSWorkspace's sharedWorkspace()
workspace's setIcon:iconImage forFile:(POSIX path of targetPath) options:0
APPLESCRIPT

    if [ $? -eq 0 ]; then
        echo "Custom icon set successfully for ${PKG_NAME}"
    else
        echo "Warning: Failed to set custom icon (this is optional)"
    fi
    
    rm -rf "${TEMP_ICON_DIR}"
fi
echo "Set icon finished"

rm -rf build_pkg_temp build_pkg_scripts build_pkg_resources "${APP_BUNDLE}"

echo "PKG package created successfully."
echo "installer ${PKG_NAME}"
