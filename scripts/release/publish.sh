#!/usr/bin/env bash
# Công bố một release đã build xong trong dist/<version>/ lên fork thuongtin/crossdesk:
# sinh feed đã ký, tạo GitHub release dạng draft, đọc lại asset, công bố release,
# kiểm tag và bản tải công khai, rồi mới cập nhật feed trên nhánh desktop-updates.
# Chỉ release.sh --publish gọi script này.
# Usage: publish.sh <version> <commit> <dist-dir> <notes-file> <platforms>
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/release/lib.sh
source "$SCRIPT_DIR/lib.sh"

[[ $# -eq 5 ]] || die "Usage: $0 <version> <commit> <dist-dir> <notes-file> <platforms>"
VERSION="$1"
COMMIT="$2"
DIST_DIR="$(cd "$3" && pwd)"
NOTES_FILE="$4"
PLATFORMS="$5"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

validate_version "$VERSION" publish
[[ "$COMMIT" =~ ^[0-9a-f]{40}$ ]] || die "Commit must be a full SHA"
[[ -s "$NOTES_FILE" ]] || die "Release notes file is missing or empty: $NOTES_FILE"
require_cmd gh git python3 openssl curl shasum

PRIVATE_KEY_FILE="${CROSSDESK_UPDATE_PRIVATE_KEY_FILE:-}"
[[ -n "$PRIVATE_KEY_FILE" ]] || die "Set CROSSDESK_UPDATE_PRIVATE_KEY_FILE to the Ed25519 feed signing key (PEM)"
[[ -f "$PRIVATE_KEY_FILE" ]] || die "CROSSDESK_UPDATE_PRIVATE_KEY_FILE does not point at a file"

# Chỉ ghi vào fork. Remote upstream (nếu có) phải giữ push URL bị vô hiệu hoá.
ORIGIN_PUSH="$(git -C "$REPO_ROOT" remote get-url --push origin)"
case "$ORIGIN_PUSH" in
    https://github.com/thuongtin/crossdesk | https://github.com/thuongtin/crossdesk.git | \
        git@github.com:thuongtin/crossdesk.git) ;;
    *) die "origin push URL must be the fork, got: $ORIGIN_PUSH" ;;
esac
if git -C "$REPO_ROOT" remote get-url upstream >/dev/null 2>&1; then
    [[ "$(git -C "$REPO_ROOT" remote get-url --push upstream)" == "DISABLED_no_push_to_upstream" ]] ||
        die "The upstream remote must keep its push URL disabled"
fi
gh auth status -h github.com >/dev/null 2>&1 || die "gh is not authenticated for github.com"

# Tag và release chưa được tồn tại: không bao giờ ghi đè asset đã công bố.
if gh api "repos/$FORK_REPO/git/ref/tags/$VERSION" >/dev/null 2>&1; then
    die "Tag $VERSION already exists on $FORK_REPO; choose a new build number"
fi
if gh release view "$VERSION" -R "$FORK_REPO" >/dev/null 2>&1; then
    die "Release $VERSION already exists on $FORK_REPO"
fi

ASSETS=()
for platform in ${PLATFORMS//,/ }; do
    while IFS= read -r asset; do
        [[ -s "$DIST_DIR/$asset" ]] || die "Missing asset $DIST_DIR/$asset"
        ASSETS+=("$asset")
    done < <(platform_assets "$platform" "$VERSION")
done
(cd "$DIST_DIR" && shasum -a 256 -c SHA256SUMS.txt) || die "SHA256SUMS.txt does not match dist files"

# Chạy lại đúng gate của release.sh (kể cả --notes) để gate/CONFORMANCE.txt giữ kết quả kiểm notes.
INSTALLED_NOTES="$DIST_DIR/INSTALLED_RELEASE_NOTES.md"
[[ -s "$INSTALLED_NOTES" ]] || die "Missing $INSTALLED_NOTES; run the release through release.sh"
log "Running strict fork-conformance gate"
python3 "$SCRIPT_DIR/check_conformance.py" --dist "$DIST_DIR" --version "$VERSION" \
    --platforms "$PLATFORMS" --public-key "$FORK_UPDATE_PUBLIC_KEY" \
    --notes "$INSTALLED_NOTES" --strict

WORK="$(mktemp -d "${TMPDIR:-/tmp}/crossdesk-publish.XXXXXX")"
FEED_WORKTREE="$WORK/feed"
cleanup() {
    if [[ -d "$FEED_WORKTREE" ]]; then
        git -C "$REPO_ROOT" worktree remove --force "$FEED_WORKTREE" >/dev/null 2>&1 || true
    fi
    rm -rf "$WORK"
}
trap cleanup EXIT

# Sinh feed trước khi đụng tới GitHub để lỗi khoá hoặc gói macOS dừng sớm.
log "Generating signed feed from origin/$FEED_BRANCH"
git -C "$REPO_ROOT" fetch --quiet origin "+refs/heads/$FEED_BRANCH:refs/remotes/origin/$FEED_BRANCH"
FEED_BASE="$(git -C "$REPO_ROOT" rev-parse "origin/$FEED_BRANCH")"
git -C "$REPO_ROOT" worktree add --quiet --detach "$FEED_WORKTREE" "$FEED_BASE"
mkdir -p "$DIST_DIR/feed"
FEED_OUT="$DIST_DIR/feed/$FEED_FILE"
rm -f "$FEED_OUT"
python3 "$SCRIPT_DIR/make_release_feed.py" --dist "$DIST_DIR" --tag "$VERSION" \
    --platforms "$PLATFORMS" --notes "$NOTES_FILE" --current "$FEED_WORKTREE/$FEED_FILE" \
    --public-key "$FORK_UPDATE_PUBLIC_KEY" --private-key "$PRIVATE_KEY_FILE" --output "$FEED_OUT"

log "Creating draft release $VERSION on $FORK_REPO at $COMMIT"
UPLOADS=()
for asset in "${ASSETS[@]}"; do
    UPLOADS+=("$DIST_DIR/$asset")
done
UPLOADS+=("$DIST_DIR/SHA256SUMS.txt")
gh release create "$VERSION" -R "$FORK_REPO" --draft --target "$COMMIT" \
    --title "CrossDesk Postcard $VERSION" --notes-file "$NOTES_FILE" "${UPLOADS[@]}"

log "Reading back draft assets"
gh release view "$VERSION" -R "$FORK_REPO" --json assets \
    --jq '.assets[] | "\(.name) \(.size)"' | sort >"$WORK/remote-sizes.txt"
for path in "${UPLOADS[@]}"; do
    printf '%s %s\n' "$(basename "$path")" "$(file_size "$path")"
done | sort >"$WORK/local-sizes.txt"
diff -u "$WORK/local-sizes.txt" "$WORK/remote-sizes.txt" ||
    die "Draft release assets differ from dist; the draft is left for inspection"
mkdir "$WORK/download"
gh release download "$VERSION" -R "$FORK_REPO" -D "$WORK/download"
(cd "$WORK/download" && shasum -a 256 -c SHA256SUMS.txt) ||
    die "Downloaded draft assets do not match SHA256SUMS.txt; the draft is left for inspection"

log "Publishing release $VERSION"
gh release edit "$VERSION" -R "$FORK_REPO" --draft=false --latest >/dev/null
TAG_SHA="$(gh api "repos/$FORK_REPO/git/ref/tags/$VERSION" --jq '.object.sha')"
[[ "$TAG_SHA" == "$COMMIT" ]] || die "Tag $VERSION points at $TAG_SHA, expected $COMMIT; feed not updated"

log "Checking public downloads"
for asset in "${ASSETS[@]}"; do
    url="$FORK_URL/releases/download/$VERSION/$asset"
    curl -fsSL --retry 5 --retry-delay 5 -o "$WORK/public-$asset" "$url"
    [[ "$(sha256_of "$WORK/public-$asset")" == "$(sha256_of "$DIST_DIR/$asset")" ]] ||
        die "Public download of $asset does not match; feed not updated"
    rm -f "$WORK/public-$asset"
done

log "Updating $FEED_BRANCH"
cp "$FEED_OUT" "$FEED_WORKTREE/$FEED_FILE"
python3 - "$FEED_WORKTREE/$FEED_FILE" "$VERSION" "$PLATFORMS" <<'PY'
import json
import sys

feed = json.load(open(sys.argv[1], encoding="utf-8"))
tag = sys.argv[2]
expected = {"macos": ["macos-arm64"], "windows": ["windows-x64"],
            "linux": ["linux-amd64", "linux-amd64-virtual-desktop-xfce"]}
assert feed["tagName"] == tag and feed["version"] == tag[1:], "feed version mismatch"
for platform in sys.argv[3].split(","):
    for key in expected[platform]:
        entry = feed["downloads"][key]
        payload = json.loads(entry["update"]["payload"])
        url = f"https://github.com/thuongtin/crossdesk/releases/download/{tag}/{entry['filename']}"
        assert entry["url"] == url == payload["url"], f"{key} url mismatch"
        assert payload["version"] == feed["version"] and payload["platform"] == key, f"{key} payload mismatch"
PY
git -C "$FEED_WORKTREE" add "$FEED_FILE"
git -C "$FEED_WORKTREE" commit --quiet -m "Publish signed desktop updates $VERSION" \
    -m "Platforms: $PLATFORMS. Release: $FORK_URL/releases/tag/$VERSION" \
    -m "Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
FEED_COMMIT="$(git -C "$FEED_WORKTREE" rev-parse HEAD)"
# Push thường (không force); nếu nhánh đã đổi kể từ lúc sinh feed thì dừng để làm lại.
git -C "$FEED_WORKTREE" push origin "HEAD:refs/heads/$FEED_BRANCH"

REMOTE_BLOB="$(gh api "repos/$FORK_REPO/contents/$FEED_FILE?ref=$FEED_BRANCH" --jq '.sha')"
LOCAL_BLOB="$(git -C "$FEED_WORKTREE" rev-parse "HEAD:$FEED_FILE")"
[[ "$REMOTE_BLOB" == "$LOCAL_BLOB" ]] || die "Remote feed blob $REMOTE_BLOB does not match $LOCAL_BLOB"

printf 'release=%s\nfeed_commit=%s\nfeed_base=%s\n' \
    "$FORK_URL/releases/tag/$VERSION" "$FEED_COMMIT" "$FEED_BASE" >"$DIST_DIR/PUBLISHED.txt"
log "Published $FORK_URL/releases/tag/$VERSION; feed commit $FEED_COMMIT"
