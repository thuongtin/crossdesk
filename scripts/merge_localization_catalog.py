#!/usr/bin/env python3
"""Three-way merge of localization/catalog.json for upstream syncs.

Đọc ba phiên bản catalog từ git (merge base, nhánh fork, nhánh upstream) và ghi
kết quả vào localization/catalog.json. Quy tắc:

- Thứ tự key và mọi thay đổi của upstream được giữ; key chỉ có ở fork được chèn
  ngay sau key đứng trước nó trong bản fork.
- Mỗi ngôn ngữ của một chuỗi được merge riêng: bên nào đổi so với merge base thì
  lấy bên đó; hai bên đổi khác nhau thì báo xung đột.
- Danh sách `slint` là bảng tra theo chỉ số: giữ nguyên toàn bộ danh sách upstream,
  nối các literal chỉ có ở fork vào cuối, rồi đánh lại chỉ số
  `GalleryStrings.texts[N]` trong những tệp .slint mà chỉ fork dùng chỉ số mới.

Script không đụng tới remote và không commit. In ra các chuỗi còn thiếu bản dịch
của ngôn ngữ mặc định để người đồng bộ dịch tiếp.
"""
import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CATALOG_PATH = 'localization/catalog.json'
SLINT_DIR = 'apps/desktop/src/gui/ui'
SLINT_REF = re.compile(r'GalleryStrings\.texts\[(\d+)\]')


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT).decode()


def load_catalog(ref):
    return json.loads(git('show', f'{ref}:{CATALOG_PATH}'))


def merge_values(key, base, ours, theirs, conflicts):
    merged = {}
    for language in list(theirs) + [lang for lang in ours if lang not in theirs]:
        base_value = base.get(language)
        our_value = ours.get(language)
        their_value = theirs.get(language)
        if their_value == base_value:
            value = our_value
        elif our_value == base_value or our_value == their_value:
            value = their_value
        else:
            conflicts.append((key, language, our_value, their_value))
            value = their_value
        if value is not None:
            merged[language] = value
    return merged


def deleted_in_fork_but_changed_upstream(base, ours, theirs):
    """Key fork đã xóa trong khi upstream vẫn sửa giá trị: xung đột xóa/sửa, phải giữ bản upstream."""
    return [key for key in theirs if key in base and key not in ours and theirs[key] != base[key]]


def merge_sequence(base, ours, theirs, keep=()):
    """Upstream order first; fork-only items follow their fork predecessor.

    Mục fork đã xóa bị bỏ khỏi kết quả, trừ các mục trong `keep` (xung đột xóa/sửa giữ bản upstream).
    Mục upstream đã xóa cũng bị bỏ dù fork từng sửa: với catalog, phần fork sửa trên key upstream chỉ
    là bản dịch, không còn ý nghĩa khi upstream thôi dùng chuỗi đó.
    """
    base_set, our_set, keep = set(base), set(ours), set(keep)
    result = [item for item in theirs if item in our_set or item not in base_set or item in keep]
    present = set(result)
    for index, item in enumerate(ours):
        if item in present or item in base_set:
            continue
        position = 0
        for previous in reversed(ours[:index]):
            if previous in present:
                position = result.index(previous) + 1
                break
        result.insert(position, item)
        present.add(item)
    return result


def merge_mapping(section, base, ours, theirs, conflicts, delete_conflicts):
    kept = deleted_in_fork_but_changed_upstream(base, ours, theirs)
    delete_conflicts.extend((section, key) for key in kept)
    keys = merge_sequence(list(base), list(ours), list(theirs), keep=kept)
    merged = {}
    for key in keys:
        if key in theirs and key in ours and theirs[key] != ours[key]:
            # Fork chỉ được phép đổi giá trị khi upstream giữ nguyên; hai bên cùng đổi khác nhau là xung đột.
            if theirs[key] == base.get(key):
                merged[key] = ours[key]
            else:
                if ours[key] != base.get(key):
                    conflicts.append((key, section, ours[key], theirs[key]))
                merged[key] = theirs[key]
        else:
            merged[key] = theirs[key] if key in theirs else ours[key]
    return merged


def merge_slint(base, ours, theirs):
    if theirs[:len(base)] != base or ours[:len(base)] != base:
        raise SystemExit('slint literal prefix was reordered; merge it by hand')
    merged = list(theirs)
    for key in ours[len(base):]:
        if key not in merged:
            merged.append(key)
    return merged


def slint_files_with_new_refs(ref, base_count):
    files = set()
    listing = git('ls-tree', '-r', '--name-only', ref, '--', SLINT_DIR).split()
    for path in listing:
        if not path.endswith('.slint'):
            continue
        content = git('show', f'{ref}:{path}')
        if any(int(n) >= base_count for n in SLINT_REF.findall(content)):
            files.add(path)
    return files


def fork_slint_refs(text, base_count):
    """Chỉ số GalleryStrings do fork thêm (>= base_count), theo thứ tự xuất hiện trong tệp."""
    return [int(number) for number in SLINT_REF.findall(text) if int(number) >= base_count]


def remap_fork_slint_files(base, ours, theirs, merged_slint, our_ref, their_ref, dry_run):
    base_count = len(base['slint'])
    fork_files = slint_files_with_new_refs(our_ref, base_count)
    upstream_files = slint_files_with_new_refs(their_ref, base_count)
    shared = sorted(fork_files & upstream_files)
    if shared:
        print('Both sides add new slint indexes in these files; remap them by hand:', file=sys.stderr)
        print('\n'.join(shared), file=sys.stderr)
    position = {key: index for index, key in enumerate(merged_slint)}
    def target(index):
        return position[ours['slint'][index]]
    def remap(text):
        # Chỉ số < base_count giống nhau ở cả ba phía (merge_slint đã kiểm tra tiền tố), nên chỉ đổi phần fork thêm.
        return SLINT_REF.sub(
            lambda match: match.group(0) if int(match.group(1)) < base_count
            else f'GalleryStrings.texts[{target(int(match.group(1)))}]', text)

    changed, manual = [], []
    for path in sorted(fork_files - upstream_files):
        file_path = ROOT / path
        if not file_path.exists():
            manual.append(path)
            continue
        content = file_path.read_text()
        # Tệp này phía upstream không có chỉ số >= base_count, nên mọi chỉ số như vậy trong working tree
        # đều đến từ fork. So dãy chỉ số đó với bản fork gốc và bản đã đánh lại, thay vì so cả tệp, để
        # lần chạy lại vẫn nhận ra tệp đã được đánh lại dù upstream có sửa phần khác của tệp.
        current = fork_slint_refs(content, base_count)
        original = fork_slint_refs(git('show', f'{our_ref}:{path}'), base_count)
        if current == [target(index) for index in original]:
            continue
        if current != original:
            manual.append(path)
            continue
        changed.append(path)
        if not dry_run:
            file_path.write_text(remap(content))
    if manual:
        print('Fork slint indexes in these files match neither the fork nor the remapped version; '
              'remap them by hand:', file=sys.stderr)
        print('\n'.join(manual), file=sys.stderr)
    return changed, shared + manual


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--base', help='merge base commit (default: git merge-base OURS THEIRS)')
    parser.add_argument('--ours', default='HEAD', help='fork side (default: HEAD)')
    parser.add_argument('--theirs', default='upstream/mcp', help='upstream side (default: upstream/mcp)')
    parser.add_argument('--language', default='vi', help='language that must cover every string (default: vi)')
    parser.add_argument('--dry-run', action='store_true', help='report only, do not write files')
    args = parser.parse_args()

    base_ref = args.base or git('merge-base', args.ours, args.theirs).strip()
    base, ours, theirs = (load_catalog(ref) for ref in (base_ref, args.ours, args.theirs))
    # Script chỉ biết merge năm section này; section mới ở bất kỳ phía nào sẽ bị bỏ mất âm thầm
    # nếu không dừng lại ở đây, nên phải kiểm tra trên catalog nguồn chứ không phải bản đã merge.
    known_sections = {'languages', 'strings', 'desktop', 'slint', 'platforms'}
    for side, catalog in (('base', base), ('ours', ours), ('theirs', theirs)):
        unknown_sections = sorted(set(catalog) - known_sections)
        if unknown_sections:
            raise SystemExit(f'unknown catalog section in {side}: {", ".join(unknown_sections)}')

    conflicts = []
    delete_conflicts = []
    kept_strings = deleted_in_fork_but_changed_upstream(base['strings'], ours['strings'], theirs['strings'])
    delete_conflicts.extend(('strings', key) for key in kept_strings)
    string_keys = merge_sequence(list(base['strings']), list(ours['strings']), list(theirs['strings']),
                                 keep=kept_strings)
    strings = {}
    for key in string_keys:
        if key in kept_strings:
            # Fork đã xóa key nhưng upstream vẫn sửa: giữ nguyên bản upstream, không merge từng ngôn ngữ.
            strings[key] = dict(theirs['strings'][key])
            continue
        strings[key] = merge_values(key, base['strings'].get(key, {}), ours['strings'].get(key, {}),
                                    theirs['strings'].get(key, {}), conflicts)

    merged = {
        'languages': merge_sequence(base['languages'], ours['languages'], theirs['languages']),
        'strings': strings,
        'desktop': merge_mapping('desktop', base['desktop'], ours['desktop'], theirs['desktop'],
                                 conflicts, delete_conflicts),
        'slint': merge_slint(base['slint'], ours['slint'], theirs['slint']),
        'platforms': {
            name: merge_sequence(base['platforms'].get(name, []), ours['platforms'].get(name, []),
                                 theirs['platforms'].get(name, []))
            for name in merge_sequence(list(base['platforms']), list(ours['platforms']), list(theirs['platforms']))
        },
    }

    referenced = set(merged['slint']) | set(merged['desktop'].values())
    for keys in merged['platforms'].values():
        referenced.update(keys)
    unknown = sorted(key for key in referenced if key not in merged['strings'])
    missing = [key for key in merged['strings'] if args.language not in merged['strings'][key]]

    changed_slint, shared_slint = remap_fork_slint_files(
        base, ours, theirs, merged['slint'], args.ours, args.theirs, args.dry_run)

    if not args.dry_run:
        (ROOT / CATALOG_PATH).write_text(json.dumps(merged, ensure_ascii=False, indent=2) + '\n')

    print(f'merge base: {base_ref}')
    print(f'strings: {len(merged["strings"])}, slint literals: {len(merged["slint"])} '
          f'(upstream {len(theirs["slint"])}, fork-only appended {len(merged["slint"]) - len(theirs["slint"])})')
    if changed_slint:
        print('remapped fork slint indexes in:\n  ' + '\n  '.join(changed_slint))
    if conflicts:
        print(f'{len(conflicts)} value conflicts (upstream value kept):')
        for key, language, our_value, their_value in conflicts:
            print(f'  [{language}] {key!r}: fork={our_value!r} upstream={their_value!r}')
    if delete_conflicts:
        print(f'{len(delete_conflicts)} keys deleted in the fork but changed upstream (upstream value kept):')
        for section, key in delete_conflicts:
            print(f'  [{section}] {key!r}')
    if unknown:
        print(f'{len(unknown)} referenced keys missing from strings:')
        print('\n'.join(f'  {key!r}' for key in unknown))
    print(f'{len(missing)} strings missing "{args.language}":')
    for key in missing:
        english = merged['strings'][key].get('en', '')
        print(f'  {key!r}  # en: {english!r}')
    return 1 if conflicts or delete_conflicts or unknown or shared_slint else 0


if __name__ == '__main__':
    raise SystemExit(main())
