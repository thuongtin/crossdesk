#!/usr/bin/env python3
"""Tách mục `## Changes` của release notes để nhúng vào bản build.

Hộp thoại Release Notes mà app mở sau khi cài bản mới hiển thị đúng mục này, nên mọi bản build
của release.sh nhận nó qua tuỳ chọn xmake CROSSDESK_RELEASE_NOTES và fork-conformance gate kiểm
file thực thi chính chứa nguyên văn các byte đó (xem docs/RELEASE.md).

Usage: installed_release_notes.py <release-notes.md> <output.md> [--publish]
"""

import argparse
import re
import sys
from pathlib import Path

# Tiêu đề ATX hợp lệ: thụt tối đa ba dấu cách, có thể có chuỗi '#' đóng như "## Changes ##".
CHANGES_HEADING = re.compile(r"^ {0,3}##[ \t]+Changes(?:[ \t]+#+)?[ \t]*$")
# Mục Changes kết thúc ở tiêu đề cấp 1 hoặc 2 kế tiếp; tiêu đề cấp 3 trở xuống thuộc về mục.
SECTION_END = re.compile(r"^ {0,3}#{1,2}(?:[ \t]|$)")
# Dòng mở hoặc đóng code fence; dòng trông như tiêu đề bên trong fence chỉ là chữ, ví dụ chú thích shell.
FENCE = re.compile(r"^ {0,3}(`{3,}|~{3,})(.*)$")
# Dòng giữ chỗ trong bản nháp release.sh tự viết khi chưa có --notes.
PLACEHOLDER = "- TODO"
# Khớp string:trim() của xmake (lớp %s của Lua) để gate so được từng byte với bản nhúng.
WHITESPACE = " \t\n\v\f\r"


def normalize(text):
    """Đổi CRLF thành LF và bỏ khoảng trắng hai đầu, giống rule about_resources.lua."""
    return text.replace("\r\n", "\n").strip(WHITESPACE)


def outside_fences(lines):
    """Trả về chỉ số các dòng nằm ngoài code fence (bản thân dòng fence cũng bị loại)."""
    indexes = []
    fence = None
    for index, line in enumerate(lines):
        match = FENCE.match(line)
        if fence is None:
            if match and not (match.group(1)[0] == "`" and "`" in match.group(2)):
                fence = match.group(1)
            else:
                indexes.append(index)
        elif match and match.group(1)[0] == fence[0] and len(match.group(1)) >= len(fence) \
                and not match.group(2).strip():
            fence = None
    return indexes


def extract(text):
    """Trả về nội dung mục `## Changes` đã chuẩn hoá; báo ValueError khi thiếu hoặc rỗng."""
    lines = text.replace("\r\n", "\n").split("\n")
    candidates = outside_fences(lines)
    start = next((index for index in candidates if CHANGES_HEADING.match(lines[index])), None)
    if start is None:
        raise ValueError("release notes have no '## Changes' section")
    end = next((index for index in candidates if index > start and SECTION_END.match(lines[index])),
               len(lines))
    section = normalize("\n".join(lines[start + 1:end]))
    if not section:
        raise ValueError("the '## Changes' section of the release notes is empty")
    return section


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("notes", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--publish", action="store_true",
                        help="reject the dry-run placeholder so a release never ships it")
    args = parser.parse_args()
    try:
        section = extract(args.notes.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeDecodeError, ValueError) as error:
        print(f"ERROR: {args.notes}: {error}", file=sys.stderr)
        return 1
    if args.publish and any(line.strip() == PLACEHOLDER for line in section.split("\n")):
        print(f"ERROR: {args.notes}: the '## Changes' section still has the '{PLACEHOLDER}' placeholder",
              file=sys.stderr)
        return 1
    # Ghi byte trực tiếp để có LF trên mọi hệ điều hành (Python 3.9 chưa có write_text(newline=)).
    args.output.write_bytes((section + "\n").encode("utf-8"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
