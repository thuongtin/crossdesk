#!/usr/bin/env bash
# Build, ký và đóng gói bản macOS arm64 ngay trên máy Mac đang chạy release.
# Usage: build_macos.sh <version> <commit> <dist-dir>
#
# Quy tắc ký (xem docs/RELEASE.md):
# - Tên identity lấy lúc chạy từ keychain (giá trị "auto"), không ghi vào file commit nào.
# - Họ tên và Team ID không được lọt vào log: mọi output có thể chứa chúng đều qua redact.
# - Notarize bằng khoá API App Store Connect; profile notarytool chỉ là phương án dự phòng.
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
APP_PREFIX="Developer ID Application: "
INSTALLER_PREFIX="Developer ID Installer: "

[[ "$(uname -s)" == "Darwin" && "$(uname -m)" == "arm64" ]] ||
    die "The macOS arm64 build must run on an Apple Silicon Mac"
# pkg-config phải có sẵn như trên runner GitHub: meson của gói libsoup dùng nó để tìm glib.
require_cmd git xmake xcrun pkgutil productbuild codesign security pkg-config plutil file awk

# Thông tin notarize chỉ cần trong shell này, không truyền xuống xmake hay pkg_arm64.sh.
export -n CROSSDESK_MACOS_NOTARY_KEY CROSSDESK_MACOS_NOTARY_KEY_ID CROSSDESK_MACOS_NOTARY_ISSUER \
    CROSSDESK_MACOS_NOTARY_PROFILE 2>/dev/null || true

KEYCHAIN_ARGS=()
if [[ -n "${CROSSDESK_MACOS_KEYCHAIN:-}" ]]; then
    KEYCHAIN_ARGS+=("$CROSSDESK_MACOS_KEYCHAIN")
fi

# Thay họ tên và Team ID trong output bằng <redacted>. Dùng index() của awk để so khớp
# nguyên văn, giá trị lấy qua ENVIRON nên không bị awk diễn giải ký tự escape.
REDACT_WORDS=()
redact() {
    if [[ ${#REDACT_WORDS[@]} -eq 0 ]]; then
        cat
        return
    fi
    REDACT_LIST="$(printf '%s\n' "${REDACT_WORDS[@]}")" awk '
        BEGIN { count = split(ENVIRON["REDACT_LIST"], words, "\n") }
        {
            line = $0
            for (k = 1; k <= count; k++) {
                if (words[k] == "") continue
                out = ""
                while ((at = index(line, words[k])) > 0) {
                    out = out substr(line, 1, at - 1) "<redacted>"
                    line = substr(line, at + length(words[k]))
                }
                line = out line
            }
            print line
            fflush()
        }'
}

# In ra identity cần dùng, hoặc chuỗi rỗng khi keychain không có. Chỉ đọc tên certificate,
# không đọc hay xuất khoá. "auto" lấy identity hợp lệ duy nhất có tiền tố cho trước; nhiều
# identity khác nhau thì dừng, vì chọn bừa có thể ký nhầm team, khi đó ghi tên đầy đủ vào
# release.env (file ngoài repo). Tên đầy đủ thì chỉ xác nhận nó có trong keychain.
resolve_identity() {
    local configured="$1" policy="$2" prefix="$3" matches count
    matches="$(security find-identity -v -p "$policy" ${KEYCHAIN_ARGS[@]+"${KEYCHAIN_ARGS[@]}"} 2>/dev/null |
        sed -n "s/.*\"\\($prefix.*\\)\"\$/\\1/p" | sort -u || true)"
    if [[ "$configured" != "auto" ]]; then
        if grep -Fxq -- "$configured" <<<"$matches"; then
            printf '%s' "$configured"
        fi
        return 0
    fi
    count="$(awk 'NF { n++ } END { print n + 0 }' <<<"$matches")"
    [[ "$count" -le 1 ]] ||
        die "Found $count '$prefix' identities; set the full name in the local release.env"
    printf '%s' "$matches"
}

SIGN_IDENTITY=""
INSTALLER_IDENTITY=""
TEAM=""
SIGN_CONFIG="${CROSSDESK_MACOS_SIGN_IDENTITY:-}"
INSTALLER_CONFIG="${CROSSDESK_MACOS_INSTALLER_IDENTITY:-}"
[[ "$SIGN_CONFIG" == "-" ]] && SIGN_CONFIG=""
for configured in "$SIGN_CONFIG" "$INSTALLER_CONFIG"; do
    [[ -z "$configured" || "$configured" == "auto" ||
        "$configured" == "$APP_PREFIX"* || "$configured" == "$INSTALLER_PREFIX"* ]] ||
        die "Signing identities must be 'auto' or a full Developer ID name"
done
if [[ -n "$SIGN_CONFIG" ]]; then
    SIGN_IDENTITY="$(resolve_identity "$SIGN_CONFIG" codesigning "$APP_PREFIX")"
fi
if [[ -n "$INSTALLER_CONFIG" ]]; then
    INSTALLER_IDENTITY="$(resolve_identity "$INSTALLER_CONFIG" basic "$INSTALLER_PREFIX")"
fi
for identity in "$SIGN_IDENTITY" "$INSTALLER_IDENTITY"; do
    [[ -n "$identity" ]] || continue
    name="${identity#"$APP_PREFIX"}"
    name="${name#"$INSTALLER_PREFIX"}"
    REDACT_WORDS+=("${name% (*}")
    team="${identity##* (}"
    REDACT_WORDS+=("${team%)}")
done
if [[ -n "$SIGN_IDENTITY" ]]; then
    TEAM="${SIGN_IDENTITY##* (}"
    TEAM="${TEAM%)}"
fi

# pkg_arm64.sh chỉ ký khi có đủ cả hai identity cùng team. Thiếu một cái thì --publish dừng,
# dry-run cảnh báo rồi build app ad-hoc trong gói chưa ký.
SIGNED=0
SIGN_STATUS="unsigned (no Developer ID identities configured)"
if [[ -n "$SIGN_CONFIG" || -n "$INSTALLER_CONFIG" ]]; then
    missing=()
    [[ -n "$SIGN_IDENTITY" ]] || missing+=("Developer ID Application (security find-identity -v -p codesigning)")
    [[ -n "$INSTALLER_IDENTITY" ]] || missing+=("Developer ID Installer (security find-identity -v -p basic)")
    if [[ ${#missing[@]} -eq 0 ]]; then
        installer_team="${INSTALLER_IDENTITY##* (}"
        [[ "${installer_team%)}" == "$TEAM" ]] ||
            die "The Developer ID Application and Installer identities belong to different teams"
        SIGNED=1
        SIGN_STATUS="signed"
    else
        SIGN_STATUS="unsigned (missing: ${missing[*]})"
        [[ "$MODE" == "publish" ]] && die "--publish needs both Developer ID identities; missing: ${missing[*]}"
        log "WARNING: $SIGN_STATUS; this dry-run builds an ad-hoc signed app in an unsigned pkg"
        SIGN_IDENTITY=""
        INSTALLER_IDENTITY=""
    fi
else
    [[ "$MODE" == "publish" ]] &&
        die "--publish needs CROSSDESK_MACOS_SIGN_IDENTITY and CROSSDESK_MACOS_INSTALLER_IDENTITY (use 'auto')"
    log "No Developer ID identities configured: building an ad-hoc signed app in an unsigned pkg"
fi

# Cách chính: khoá API App Store Connect (đường dẫn .p8, Key ID, Issuer ID). Dự phòng: profile
# notarytool trong keychain, vì profile từng mất sau khi khởi động lại máy.
NOTARY_ARGS=()
NOTARY_METHOD="none"
if [[ -n "${CROSSDESK_MACOS_NOTARY_KEY:-}${CROSSDESK_MACOS_NOTARY_KEY_ID:-}${CROSSDESK_MACOS_NOTARY_ISSUER:-}" ]]; then
    for variable in CROSSDESK_MACOS_NOTARY_KEY CROSSDESK_MACOS_NOTARY_KEY_ID CROSSDESK_MACOS_NOTARY_ISSUER; do
        [[ -n "${!variable:-}" ]] || die "Set all of CROSSDESK_MACOS_NOTARY_KEY, _KEY_ID and _ISSUER ($variable is empty)"
    done
    [[ -r "$CROSSDESK_MACOS_NOTARY_KEY" ]] ||
        die "CROSSDESK_MACOS_NOTARY_KEY does not point to a readable App Store Connect .p8 file"
    NOTARY_ARGS=(--key "$CROSSDESK_MACOS_NOTARY_KEY" --key-id "$CROSSDESK_MACOS_NOTARY_KEY_ID"
        --issuer "$CROSSDESK_MACOS_NOTARY_ISSUER")
    NOTARY_METHOD="api-key"
elif [[ -n "${CROSSDESK_MACOS_NOTARY_PROFILE:-}" ]]; then
    NOTARY_ARGS=(--keychain-profile "$CROSSDESK_MACOS_NOTARY_PROFILE")
    if [[ -n "${CROSSDESK_MACOS_KEYCHAIN:-}" ]]; then
        NOTARY_ARGS+=(--keychain "$CROSSDESK_MACOS_KEYCHAIN")
    fi
    NOTARY_METHOD="keychain-profile"
fi
[[ "$MODE" != "publish" || "$NOTARY_METHOD" != "none" ]] ||
    die "--publish needs notarization credentials: CROSSDESK_MACOS_NOTARY_KEY/_KEY_ID/_ISSUER (or a notary profile)"
NOTARIZED=0
if [[ "$SIGNED" == 0 && "$NOTARY_METHOD" != "none" ]]; then
    log "Notarization skipped: the pkg is not signed"
fi

# Keychain chứa khoá ký có thể khoá lại sau mỗi lần khởi động máy. Chủ máy phải tự mở, script
# không bao giờ hỏi hay gõ mật khẩu. Trong phiên GUI, codesign trên keychain đang khoá không báo
# lỗi mà bật hộp thoại mở khoá, nên kiểm trạng thái bằng SecKeychainGetStatus (không tương tác)
# trước khi gọi codesign. In ra đường dẫn các keychain đang khoá; mã thoát 2 khi không kiểm được.
keychain_lock_check() {
    python3 -I - "$@" <<'PY'
import ctypes
import sys

security = ctypes.cdll.LoadLibrary("/System/Library/Frameworks/Security.framework/Security")
UNLOCKED = 1  # kSecUnlockStateStatus
locked = False
for path in sys.argv[1:]:
    keychain = ctypes.c_void_p()
    status = ctypes.c_uint32()
    if security.SecKeychainOpen(path.encode(), ctypes.byref(keychain)) != 0:
        sys.exit(2)
    if security.SecKeychainGetStatus(keychain, ctypes.byref(status)) != 0:
        sys.exit(2)
    if not status.value & UNLOCKED:
        print(path)
        locked = True
sys.exit(1 if locked else 0)
PY
}

# Thử ký một bản sao /usr/bin/true trước khi build lâu.
if [[ "$SIGNED" == 1 ]]; then
    require_cmd python3 perl
    signing_keychains=()
    while IFS= read -r keychain; do
        [[ -n "$keychain" ]] && signing_keychains+=("$keychain")
    done < <(for identity in "$SIGN_IDENTITY" "$INSTALLER_IDENTITY"; do
        security find-certificate -a -c "$identity" ${KEYCHAIN_ARGS[@]+"${KEYCHAIN_ARGS[@]}"} 2>/dev/null |
            sed -n 's/^keychain: "\(.*\)"$/\1/p'
    done | sort -u)
    [[ ${#signing_keychains[@]} -gt 0 ]] || die "Cannot find the keychain that holds the Developer ID certificates"
    lock_status=0
    locked_keychains="$(keychain_lock_check "${signing_keychains[@]}")" || lock_status=$?
    case "$lock_status" in
        0) ;;
        1) die "The signing keychain is locked: the owner must run 'security unlock-keychain $(printf '%s' "$locked_keychains" | head -1)', then rerun" ;;
        *) die "Cannot read the lock state of the signing keychain" ;;
    esac
    probe_dir="$(mktemp -d "${TMPDIR:-/tmp}/crossdesk-sign-probe.XXXXXX")"
    cp /usr/bin/true "$probe_dir/probe"
    probe_keychain=()
    if [[ -n "${CROSSDESK_MACOS_KEYCHAIN:-}" ]]; then
        probe_keychain+=(--keychain "$CROSSDESK_MACOS_KEYCHAIN")
    fi
    probe_status=0
    # Dự phòng khi keychain vẫn đòi quyền truy cập: perl alarm dừng codesign sau 90 giây
    # (SIGALRM, mã 142) thay vì để nó chờ hộp thoại mãi.
    probe_output="$(perl -e 'alarm shift; exec @ARGV' 90 codesign --force --sign "$SIGN_IDENTITY" \
        --options runtime --timestamp ${probe_keychain[@]+"${probe_keychain[@]}"} "$probe_dir/probe" 2>&1)" ||
        probe_status=$?
    rm -rf "$probe_dir"
    [[ "$probe_status" -ne 142 ]] ||
        die "codesign timed out waiting for keychain access: the owner must unlock the signing keychain and dismiss any open dialog"
    if [[ "$probe_status" -ne 0 ]]; then
        [[ "$probe_output" != *errSecInternalComponent* ]] ||
            die "The signing keychain is locked (errSecInternalComponent): the owner must unlock it with 'security unlock-keychain <keychain>', then rerun"
        printf '%s\n' "$probe_output" | redact >&2
        die "Developer ID signing probe failed"
    fi
    log "Developer ID signing probe passed"
    if [[ "$NOTARY_METHOD" != "none" ]]; then
        xcrun notarytool history "${NOTARY_ARGS[@]}" --output-format json >/dev/null 2>&1 ||
            die "notarytool rejected the $NOTARY_METHOD credentials"
        log "Notarization credentials accepted ($NOTARY_METHOD)"
    fi
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

# pkg_arm64.sh nhận tên identity đầy đủ đã tìm được. Profile notarize bị gỡ khỏi môi trường của
# nó để chỉ script này notarize, sau khi đã kiểm Team ID.
PACKAGE_ENV=(env -u CROSSDESK_MACOS_NOTARY_PROFILE)
if [[ "$SIGNED" == 1 ]]; then
    PACKAGE_ENV+=(CROSSDESK_MACOS_SIGN_IDENTITY="$SIGN_IDENTITY" CROSSDESK_MACOS_INSTALLER_IDENTITY="$INSTALLER_IDENTITY")
else
    PACKAGE_ENV+=(-u CROSSDESK_MACOS_SIGN_IDENTITY -u CROSSDESK_MACOS_INSTALLER_IDENTITY)
fi
log "Packaging ($SIGN_STATUS, notarization=$NOTARY_METHOD)"
"${PACKAGE_ENV[@]}" bash ./apps/desktop/scripts/macos/pkg_arm64.sh "$VERSION" 2>&1 | redact

PACKAGE="$SRC_DIR/$(asset_name macos "$VERSION")"
[[ -s "$PACKAGE" ]] || die "Missing package: $PACKAGE"

VERIFY_DIR="$(mktemp -d "${TMPDIR:-/tmp}/crossdesk-release-pkg.XXXXXX")"
trap 'rm -rf "$VERIFY_DIR"' EXIT
pkgutil --expand-full "$PACKAGE" "$VERIFY_DIR/expanded"
APP_PATH="$(find "$VERIFY_DIR/expanded" -type d -name 'CrossDesk.app' -print -quit)"
[[ -n "$APP_PATH" ]] || die "CrossDesk.app is missing from $PACKAGE"
"${PACKAGE_ENV[@]}" bash ./apps/desktop/scripts/macos/verify_app.sh "$APP_PATH" arm64 2>&1 | redact

# macOS Installer so phiên bản theo CFBundleVersion và pkg-ref, nên tag v<base>-<build>-<date>
# phải thành <base>.<date>.<build> (giống bản 1.7.7.20261125.2 đã phát hành). Kiểm ngay sau
# khi đóng gói để --publish dừng trước khi tải gì lên, thay vì đợi tới bước sinh feed.
BUNDLE_VERSION="$(plutil -extract CFBundleVersion raw "$APP_PATH/Contents/Info.plist")"
SHORT_VERSION="$(plutil -extract CFBundleShortVersionString raw "$APP_PATH/Contents/Info.plist")"
BUNDLE_CHECK="skipped (dry-run version without a date)"
if [[ "$VERSION" =~ ^v([0-9]+\.[0-9]+\.[0-9]+)-([0-9]+)-([0-9]{8})$ ]]; then
    EXPECTED_BUNDLE_VERSION="${BASH_REMATCH[1]}.${BASH_REMATCH[3]}.${BASH_REMATCH[2]}"
    EXPECTED_SHORT_VERSION="${BASH_REMATCH[1]}"
    BUNDLE_CHECK="pass"
    if [[ "$BUNDLE_VERSION" != "$EXPECTED_BUNDLE_VERSION" || "$SHORT_VERSION" != "$EXPECTED_SHORT_VERSION" ]] ||
        ! grep -Fq "version=\"$EXPECTED_BUNDLE_VERSION\"" "$VERIFY_DIR/expanded/Distribution"; then
        BUNDLE_CHECK="fail (expected CFBundleVersion and pkg-ref $EXPECTED_BUNDLE_VERSION, CFBundleShortVersionString $EXPECTED_SHORT_VERSION)"
        [[ "$MODE" == "publish" ]] && die "macOS bundle version check $BUNDLE_CHECK; got $BUNDLE_VERSION / $SHORT_VERSION"
        log "WARNING: macOS bundle version check $BUNDLE_CHECK; got $BUNDLE_VERSION / $SHORT_VERSION"
    fi
fi

# Trước khi gửi Apple: mọi Mach-O trong app phải mang Team ID của identity và Hardened Runtime.
# Mã ad-hoc hay của team khác làm notarize trượt. Log chỉ ghi ok/MISMATCH, không ghi Team ID.
TEAM_CHECK="skipped (unsigned)"
if [[ "$SIGNED" == 1 ]]; then
    team_failures=0
    while IFS= read -r -d '' binary; do
        [[ "$(file -b "$binary")" == Mach-O* ]] || continue
        # Lưu output trước rồi mới so khớp: codesign | grep -q dưới pipefail có thể báo sai.
        signature="$(codesign -dv --verbose=2 "$binary" 2>&1 || true)"
        binary_team="$(sed -n 's/^TeamIdentifier=//p' <<<"$signature")"
        if [[ "$binary_team" == "$TEAM" ]] && grep -q '^CodeDirectory .*flags=.*runtime' <<<"$signature"; then
            log "team-check ok       ${binary#"$APP_PATH/"}"
        else
            log "team-check MISMATCH ${binary#"$APP_PATH/"}"
            team_failures=$((team_failures + 1))
        fi
    done < <(find "$APP_PATH/Contents" -type f \( -perm -u+x -o -name '*.dylib' -o -name '*.so' \) -print0)
    [[ "$team_failures" -eq 0 ]] ||
        die "$team_failures executables are not signed by the Developer ID team with Hardened Runtime"
    TEAM_CHECK="pass"
    pkgutil --check-signature "$PACKAGE" 2>&1 | redact
fi

# Notarize chính file .pkg (Apple kiểm cả app bên trong), rồi staple vé vào .pkg.
if [[ "$SIGNED" == 1 && "$NOTARY_METHOD" != "none" ]]; then
    NOTARY_DIR="$BUILD_ROOT/notary"
    mkdir -p "$NOTARY_DIR"
    submit_file="$NOTARY_DIR/$(basename "$PACKAGE").submit.json"
    log "Submitting $(basename "$PACKAGE") for notarization ($NOTARY_METHOD)"
    submit_status=0
    xcrun notarytool submit "$PACKAGE" "${NOTARY_ARGS[@]}" --wait \
        --timeout "${CROSSDESK_MACOS_NOTARY_TIMEOUT:-30m}" --output-format json >"$submit_file" ||
        submit_status=$?
    submission_id="$(plutil -extract id raw -o - "$submit_file" 2>/dev/null || true)"
    notary_status="$(plutil -extract status raw -o - "$submit_file" 2>/dev/null || true)"
    if [[ -n "$submission_id" ]]; then
        xcrun notarytool log "$submission_id" "${NOTARY_ARGS[@]}" \
            "$NOTARY_DIR/$(basename "$PACKAGE").log.json" >/dev/null 2>&1 || true
    fi
    log "Notarization ${submission_id:-without a submission id}: ${notary_status:-unknown} (details in $NOTARY_DIR)"
    [[ "$submit_status" -eq 0 && "$notary_status" == "Accepted" ]] ||
        die "Notarization did not complete (status ${notary_status:-unknown}); check $NOTARY_DIR before resubmitting"
    xcrun stapler staple "$PACKAGE"
    xcrun stapler validate "$PACKAGE"
    spctl --assess --type install --verbose=2 "$PACKAGE" 2>&1 | redact
    NOTARIZED=1
fi
[[ "$MODE" != "publish" || "$NOTARIZED" == 1 ]] || die "--publish needs a notarized and stapled pkg"

cp "$PACKAGE" "$DIST_DIR/"
printf 'signed=%s\nsign_status=%s\nteam_check=%s\nnotarized=%s\nnotary_method=%s\nsdk=%s\nbundle_version=%s\nshort_version=%s\nbundle_check=%s\n' \
    "$SIGNED" "$SIGN_STATUS" "$TEAM_CHECK" "$NOTARIZED" "$NOTARY_METHOD" \
    "$(xcrun --sdk macosx --show-sdk-version)" "$BUNDLE_VERSION" "$SHORT_VERSION" "$BUNDLE_CHECK" \
    >"$DIST_DIR/gate/macos-signing.txt"
log "macOS package ready: $DIST_DIR/$(basename "$PACKAGE") ($(file_size "$PACKAGE") bytes)"
