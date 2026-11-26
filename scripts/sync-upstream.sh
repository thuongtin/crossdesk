#!/usr/bin/env bash
# Báo cáo trước một lần đồng bộ upstream: fetch nhánh upstream rồi merge thử bằng
# `git merge-tree`, không đụng tới working tree, index hay bất kỳ ref nào của fork.
#
# Script này KHÔNG push, KHÔNG đổi URL remote và KHÔNG tạo commit. Quy trình merge thật
# nằm trong docs/UPSTREAM_SYNC.md.
#
# Cách dùng:
#   scripts/sync-upstream.sh [FORK_REF]
#
# Biến môi trường:
#   UPSTREAM_REMOTE   remote của upstream (mặc định: upstream)
#   UPSTREAM_BRANCH   nhánh upstream cần đồng bộ (mặc định: mcp)
#   SKIP_FETCH=1      dùng ref upstream đã fetch sẵn, không gọi mạng
#   SKIP_CATALOG=1    bỏ qua bước chạy thử merge_localization_catalog.py
set -euo pipefail

upstream_remote="${UPSTREAM_REMOTE:-upstream}"
upstream_branch="${UPSTREAM_BRANCH:-mcp}"
fork_ref="${1:-HEAD}"
upstream_ref="refs/remotes/${upstream_remote}/${upstream_branch}"

repo_root="$(git rev-parse --show-toplevel)"
cd "$repo_root"

if ! git remote get-url "$upstream_remote" >/dev/null 2>&1; then
  echo "error: remote '$upstream_remote' does not exist; add it as a fetch-only remote first" >&2
  exit 2
fi

# Remote upstream chỉ được dùng để fetch; push URL phải bị vô hiệu hoá để tránh push nhầm.
push_url="$(git remote get-url --push "$upstream_remote")"
case "$push_url" in
  DISABLED*) ;;
  *)
    echo "warning: push URL of '$upstream_remote' is '$push_url'; it should start with DISABLED" >&2
    ;;
esac

if [[ "${SKIP_FETCH:-0}" != "1" ]]; then
  echo "Fetching $upstream_remote/$upstream_branch ..."
  git fetch --no-tags "$upstream_remote" "$upstream_branch:$upstream_ref"
fi

fork_commit="$(git rev-parse --verify "$fork_ref^{commit}")"
upstream_commit="$(git rev-parse --verify "$upstream_ref^{commit}")"
merge_base="$(git merge-base "$fork_commit" "$upstream_commit")"
pending_count="$(git rev-list --count "$merge_base..$upstream_commit")"
fork_only_count="$(git rev-list --count "$merge_base..$fork_commit")"

echo
echo "Fork ref:        $fork_ref ($(git rev-parse --short "$fork_commit"))"
echo "Upstream ref:    $upstream_remote/$upstream_branch ($(git rev-parse --short "$upstream_commit"))"
echo "Merge base:      $(git rev-parse --short "$merge_base")"
echo "Upstream range:  $(git rev-parse --short "$merge_base")..$(git rev-parse --short "$upstream_commit") ($pending_count commits)"
echo "Fork-only:       $fork_only_count commits"

if [[ "$pending_count" == "0" ]]; then
  echo
  echo "Fork already contains every upstream commit; nothing to sync."
  exit 0
fi

echo
echo "Upstream commits not in the fork (newest first, at most 50):"
git log --no-merges --format='  %h %ad %s' --date=short -n 50 "$merge_base..$upstream_commit"

# merge-tree trả mã 0 khi merge sạch, 1 khi có xung đột; mã khác là lỗi thật.
set +e
merge_output="$(git merge-tree --write-tree --name-only --no-messages "$fork_commit" "$upstream_commit")"
merge_status=$?
set -e
if [[ $merge_status -gt 1 ]]; then
  echo "error: git merge-tree failed with status $merge_status" >&2
  exit "$merge_status"
fi

# Dòng đầu là OID của tree kết quả, các dòng sau (nếu có) là tệp bị xung đột.
conflicts="$(printf '%s\n' "$merge_output" | tail -n +2 | sed '/^$/d')"

is_generated() {
  case "$1" in
    apps/desktop/src/gui/assets/localization/localization_data.h) return 0 ;;
    apps/android/app/src/main/res/values*/strings.xml) return 0 ;;
    apps/android/app/src/main/res/xml/locales_config.xml) return 0 ;;
    apps/android/app/src/main/java/cn/crossdesk/mobile/LocalizationKeys.java) return 0 ;;
    apps/ios/CrossDeskMobile/Resources/*.lproj/Localizable.strings) return 0 ;;
    apps/ios/CrossDeskMobile/Resources/*.lproj/InfoPlist.strings) return 0 ;;
    apps/harmony/entry/src/main/ets/mobile/LocalizationData.ets) return 0 ;;
  esac
  return 1
}

catalog_files=()
generated_files=()
doc_files=()
code_files=()
if [[ -n "$conflicts" ]]; then
  while IFS= read -r path; do
    if [[ "$path" == "localization/catalog.json" ]]; then
      catalog_files+=("$path")
    elif is_generated "$path"; then
      generated_files+=("$path")
    elif [[ "$path" == *.md ]]; then
      doc_files+=("$path")
    else
      code_files+=("$path")
    fi
  done <<< "$conflicts"
fi

print_group() {
  local title="$1"
  shift
  echo "  $title ($#):"
  if [[ $# -eq 0 ]]; then
    echo "    (none)"
    return
  fi
  local item
  for item in "$@"; do
    echo "    $item"
  done
}

echo
if [[ $merge_status -eq 0 ]]; then
  echo "Trial merge: clean (no conflicts)."
else
  echo "Trial merge: conflicts in $(printf '%s\n' "$conflicts" | wc -l | tr -d ' ') files."
  # `${arr[@]+...}` giữ cho bash 3.2 của macOS không báo lỗi mảng rỗng khi bật `set -u`.
  print_group "Catalog (use scripts/merge_localization_catalog.py)" ${catalog_files[@]+"${catalog_files[@]}"}
  print_group "Generated resources (rerun python3 localization/generate.py)" ${generated_files[@]+"${generated_files[@]}"}
  print_group "Docs (translate upstream changes into Vietnamese)" ${doc_files[@]+"${doc_files[@]}"}
  print_group "Code (take upstream behavior, reapply Vietnamese strings)" ${code_files[@]+"${code_files[@]}"}
fi

echo
echo "Markdown files changed upstream (review for new content to translate):"
changed_docs="$(git diff --name-only "$merge_base" "$upstream_commit" -- '*.md')"
if [[ -n "$changed_docs" ]]; then
  printf '%s\n' "$changed_docs" | sed 's/^/  /'
else
  echo "  (none)"
fi

if [[ "${SKIP_CATALOG:-0}" != "1" ]] && git diff --quiet "$merge_base" "$upstream_commit" -- localization/catalog.json; then
  echo
  echo "Upstream did not change localization/catalog.json."
elif [[ "${SKIP_CATALOG:-0}" != "1" ]]; then
  echo
  echo "Catalog dry run (scripts/merge_localization_catalog.py --dry-run):"
  # Chạy thử chỉ đọc catalog từ git và in báo cáo, không ghi tệp nào.
  set +e
  python3 scripts/merge_localization_catalog.py --dry-run --ours "$fork_commit" --theirs "$upstream_commit" --base "$merge_base" \
    | sed 's/^/  /'
  catalog_status=${PIPESTATUS[0]}
  set -e
  if [[ $catalog_status -ne 0 ]]; then
    echo "  catalog merge needs manual work (exit status $catalog_status)"
  fi
fi

echo
echo "Next steps: see docs/UPSTREAM_SYNC.md. This script never pushes or edits remotes."
exit "$merge_status"
