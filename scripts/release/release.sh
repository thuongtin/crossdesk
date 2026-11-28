#!/usr/bin/env bash
# Điểm vào của quy trình release CrossDesk Postcard cho macOS arm64, Windows x64 và Linux amd64.
# Mặc định là dry-run: build, đóng gói, chạy fork-conformance gate và tạo checksum trong
# dist/<version>/, không tạo tag, release hay sửa feed. Chỉ --publish mới công bố.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
# shellcheck source=scripts/release/lib.sh
source "$SCRIPT_DIR/lib.sh"

usage() {
    cat >&2 <<EOF
Usage: $0 <version> [--publish] [--ref <ref>] [--platforms macos,windows,linux]
          [--notes <file>] [--skip-build]

  <version>     v<base>-<build>-<YYYYMMDD>, for example v1.7.7-3-20261126.
                Dry-runs may use a word instead of the date (v1.7.7-3-dryrun).
  --publish     Create the GitHub release and update the desktop-updates feed.
  --ref         Branch, tag or SHA of thuongtin/crossdesk to build (default: mcp).
  --platforms   Comma-separated subset of macos,windows,linux (default: all).
  --notes       Release notes (Markdown). Required with --publish.
  --skip-build  Reuse the artifacts already in dist/<version>/.
EOF
    exit 2
}

[[ $# -ge 1 ]] || usage
VERSION="$1"
shift
MODE="dry-run"
REF="mcp"
PLATFORMS="macos,windows,linux"
NOTES=""
SKIP_BUILD=0
while [[ $# -gt 0 ]]; do
    case "$1" in
        --publish) MODE="publish" ;;
        --ref) REF="${2:?}"; shift ;;
        --platforms) PLATFORMS="${2:?}"; shift ;;
        --notes) NOTES="${2:?}"; shift ;;
        --skip-build) SKIP_BUILD=1 ;;
        -h | --help) usage ;;
        *) die "Unknown argument: $1" ;;
    esac
    shift
done

validate_version "$VERSION" "$MODE"
for platform in ${PLATFORMS//,/ }; do
    case " $ALL_PLATFORMS " in
        *" $platform "*) ;;
        *) die "Unknown platform: $platform" ;;
    esac
done
if [[ "$MODE" == "publish" ]]; then
    [[ "$REF" == "mcp" ]] || die "--publish only releases the head of mcp"
    [[ -n "$NOTES" ]] || die "--publish requires --notes <file>"
fi
[[ -z "$NOTES" || -s "$NOTES" ]] || die "Release notes file is missing or empty: $NOTES"
require_cmd gh git python3 shasum
load_release_config

COMMIT="$(gh api "repos/$FORK_REPO/commits/$REF" --jq '.sha')"
[[ "$COMMIT" =~ ^[0-9a-f]{40}$ ]] || die "Cannot resolve $REF on $FORK_REPO"
log "Release $VERSION ($MODE) of $FORK_REPO@$REF = $COMMIT, platforms: $PLATFORMS"

mkdir -p "$REPO_ROOT/dist"
[[ -f "$REPO_ROOT/dist/.gitignore" ]] || printf '*\n' >"$REPO_ROOT/dist/.gitignore"
DIST_DIR="$REPO_ROOT/dist/$VERSION"

if [[ "$SKIP_BUILD" == 1 ]]; then
    [[ -f "$DIST_DIR/build-info.json" ]] || die "No previous build in $DIST_DIR"
    python3 - "$DIST_DIR/build-info.json" "$COMMIT" <<'PY' || die "$DIST_DIR was built from another commit"
import json
import sys

sys.exit(0 if json.load(open(sys.argv[1]))["commit"] == sys.argv[2] else 1)
PY
else
    rm -rf "$DIST_DIR"
    mkdir -p "$DIST_DIR/gate" "$DIST_DIR/logs"
    python3 - "$DIST_DIR/build-info.json" "$VERSION" "$COMMIT" "$REF" "$MODE" "$PLATFORMS" \
        "$(git -C "$REPO_ROOT" rev-parse HEAD)" <<'PY'
import datetime
import json
import sys

path, version, commit, ref, mode, platforms, pipeline = sys.argv[1:]
json.dump({"version": version, "commit": commit, "ref": ref, "mode": mode,
           "platforms": platforms.split(","), "pipeline_commit": pipeline,
           "started_at": datetime.datetime.now(datetime.timezone.utc).isoformat(timespec="seconds")},
          open(path, "w"), indent=2)
PY
fi

if [[ -n "$NOTES" ]]; then
    cp "$NOTES" "$DIST_DIR/RELEASE_NOTES.md"
elif [[ ! -f "$DIST_DIR/RELEASE_NOTES.md" ]]; then
    # Bản nháp theo khuôn release Postcard trước; người phát hành sửa phần Changes.
    cat >"$DIST_DIR/RELEASE_NOTES.md" <<'EOF'
## Downloads

- macOS 14 or later, Apple Silicon: signed, notarized, stapled Developer ID installer.
- Windows x64 installer.
- Linux amd64 (Ubuntu 20.04 or later): `crossdesk` and `crossdesk-virtual-desktop-xfce` .deb packages.
- Use `SHA256SUMS.txt` to verify the downloads.
- Installs of v1.7.7-1 or v1.7.7-2 must download and install this release manually once: the update signing key changed in v1.7.7-3, so those builds cannot verify the new update feed. Builds from v1.7.7-3 on update from the app.

## Changes

- TODO
EOF
fi
# Mục Changes được nhúng vào app để hộp thoại Release Notes hiện đúng ghi chú của bản này, nên nó
# phải có trước khi build. Với --skip-build, tệp được sinh lại từ notes hiện tại để gate phát hiện
# notes đã đổi sau lần build trước.
notes_args=("$DIST_DIR/RELEASE_NOTES.md" "$DIST_DIR/INSTALLED_RELEASE_NOTES.md")
[[ "$MODE" == "publish" ]] && notes_args+=(--publish)
python3 "$SCRIPT_DIR/installed_release_notes.py" "${notes_args[@]}" ||
    die "Cannot take the '## Changes' section from $DIST_DIR/RELEASE_NOTES.md"

if [[ "$SKIP_BUILD" != 1 ]]; then
    export CROSSDESK_RELEASE_MODE="$MODE"
    # Mảng thường (không dùng mảng kết hợp) để chạy được với bash 3.2 của macOS.
    BUILD_PLATFORMS=()
    BUILD_PIDS=()
    for platform in ${PLATFORMS//,/ }; do
        log "Starting $platform build (log: dist/$VERSION/logs/$platform.log)"
        bash "$SCRIPT_DIR/build_$platform.sh" "$VERSION" "$COMMIT" "$DIST_DIR" \
            >"$DIST_DIR/logs/$platform.log" 2>&1 &
        BUILD_PLATFORMS+=("$platform")
        BUILD_PIDS+=($!)
    done
    FAILED=()
    for index in "${!BUILD_PLATFORMS[@]}"; do
        platform="${BUILD_PLATFORMS[$index]}"
        if wait "${BUILD_PIDS[$index]}"; then
            log "$platform build finished"
        else
            FAILED+=("$platform")
            log "$platform build FAILED, last lines of its log:"
            tail -n 20 "$DIST_DIR/logs/$platform.log" >&2 || true
        fi
    done
    if [[ ${#FAILED[@]} -gt 0 ]]; then
        printf '%s\n' "${FAILED[@]}" >"$DIST_DIR/FAILED_PLATFORMS.txt"
        [[ "$MODE" == "dry-run" ]] || die "Builds failed: ${FAILED[*]}; nothing was published"
        # Dry-run vẫn chạy gate và checksum cho các nền tảng build được.
        remaining=()
        for platform in ${PLATFORMS//,/ }; do
            [[ " ${FAILED[*]} " == *" $platform "* ]] || remaining+=("$platform")
        done
        PLATFORMS=""
        if [[ ${#remaining[@]} -gt 0 ]]; then
            PLATFORMS="$(IFS=,; echo "${remaining[*]}")"
        fi
    fi
fi

if [[ -n "$PLATFORMS" ]]; then
    gate_args=(--dist "$DIST_DIR" --version "$VERSION" --platforms "$PLATFORMS"
        --public-key "$FORK_UPDATE_PUBLIC_KEY" --notes "$DIST_DIR/INSTALLED_RELEASE_NOTES.md")
    [[ "$MODE" == "publish" ]] && gate_args+=(--strict)
    log "Running fork-conformance gate"
    python3 "$SCRIPT_DIR/check_conformance.py" "${gate_args[@]}"

    ASSETS=()
    for platform in ${PLATFORMS//,/ }; do
        while IFS= read -r asset; do
            [[ -s "$DIST_DIR/$asset" ]] || die "Missing asset $DIST_DIR/$asset"
            ASSETS+=("$asset")
        done < <(platform_assets "$platform" "$VERSION")
    done
    (cd "$DIST_DIR" && shasum -a 256 "${ASSETS[@]}") >"$DIST_DIR/SHA256SUMS.txt"
    log "Artifacts in $DIST_DIR:"
    for asset in "${ASSETS[@]}"; do
        log "  $asset $(file_size "$DIST_DIR/$asset") bytes"
    done
fi

if [[ -f "$DIST_DIR/FAILED_PLATFORMS.txt" ]]; then
    die "Dry-run finished with failed platforms: $(paste -sd, "$DIST_DIR/FAILED_PLATFORMS.txt")"
fi
if [[ "$MODE" != "publish" ]]; then
    log "Dry-run complete. Nothing was tagged, released or pushed."
    exit 0
fi

bash "$SCRIPT_DIR/publish.sh" "$VERSION" "$COMMIT" "$DIST_DIR" "$DIST_DIR/RELEASE_NOTES.md" "$PLATFORMS"
