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
# Dòng mở code fence, chỉ nhận ở cột 0 (xem nested_fence); dòng trông như tiêu đề bên trong fence
# chỉ là chữ, ví dụ chú thích shell.
OPENING_FENCE = re.compile(r"^(`{3,}|~{3,})(.*)$")
# Dòng đóng code fence: thụt tối đa ba dấu cách, sau chuỗi fence chỉ còn dấu cách hoặc tab.
CLOSING_FENCE = re.compile(r"^ {0,3}(`{3,}|~{3,})[ \t]*$")
# Phần thụt lề, dấu trích dẫn '>' và dấu list đứng trước nội dung của dòng, ví dụ "  ", "- ", "> 1. ".
CONTAINER_PREFIX = re.compile(r"^[ \t>]*(?:(?:[-+*]|[0-9]{1,9}[.)])[ \t]+[ \t>]*)*")
# Dòng giữ chỗ trong bản nháp release.sh tự viết khi chưa có --notes.
PLACEHOLDER = "- TODO"
# Khớp string:trim() của xmake (lớp %s của Lua) để gate so được từng byte với bản nhúng.
WHITESPACE = " \t\n\v\f\r"


def normalize(text):
    """Đổi CRLF thành LF và bỏ khoảng trắng hai đầu, giống rule about_resources.lua."""
    return text.replace("\r\n", "\n").strip(WHITESPACE)


def opening_fence(text):
    """Trả về chuỗi fence khi text mở code fence ngay từ ký tự đầu, ngược lại None."""
    match = OPENING_FENCE.match(text)
    # Dòng mở fence bằng backtick không được chứa backtick trong phần info.
    if not match or (match.group(1)[0] == "`" and "`" in match.group(2)):
        return None
    return match.group(1)


def nested_fence(line):
    """True khi dòng mở code fence sau phần thụt lề, tab, dấu list hay dấu trích dẫn.

    Theo CommonMark, fence như vậy thuộc mục list hay trích dẫn chứa nó và kết thúc cùng mục đó,
    tính theo cột có tab. Extractor và InstalledBlocks chỉ hiểu fence ở cột 0, nên notes có fence
    lồng bị từ chối thay vì bị tách sai ranh giới mục.
    """
    prefix = CONTAINER_PREFIX.match(line).end()
    return prefix > 0 and opening_fence(line[prefix:]) is not None


def outside_fences(lines):
    """Trả về chỉ số các dòng nằm ngoài code fence mở ở cột 0 (bản thân dòng fence cũng bị loại)."""
    indexes = []
    fence = None
    for index, line in enumerate(lines):
        if fence is None:
            fence = opening_fence(line)
            if fence is None:
                indexes.append(index)
            continue
        match = CLOSING_FENCE.match(line)
        if match and match.group(1)[0] == fence[0] and len(match.group(1)) >= len(fence):
            fence = None
    return indexes


def extract(text, publish=False):
    """Trả về nội dung mục `## Changes` đã chuẩn hoá; báo ValueError khi notes không dùng được.

    Mọi kiểm tra chạy trên dòng gốc, trước khi normalize bỏ thụt lề của dòng đầu mục. Với publish,
    dòng giữ chỗ của bản nháp cũng bị từ chối.
    """
    lines = text.replace("\r\n", "\n").split("\n")
    candidates = outside_fences(lines)
    start = next((index for index in candidates if CHANGES_HEADING.match(lines[index])), None)
    if start is None:
        raise ValueError("release notes have no '## Changes' section")
    end = next((index for index in candidates if index > start and SECTION_END.match(lines[index])),
               len(lines))
    # Fence lồng từ đầu notes tới hết mục có thể giấu hay tạo ra tiêu đề quyết định ranh giới mục.
    nested = next((index for index in candidates if index < end and nested_fence(lines[index])), None)
    if nested is not None:
        raise ValueError(f"line {nested + 1}: start code fences at column 0, "
                         "not indented or inside a list item or quote")
    # Chỉ dòng ngoài code fence mới là chỗ giữ chỗ; ví dụ trong khối code được phép chứa "- TODO".
    if publish and any(lines[index].strip() == PLACEHOLDER for index in candidates if start < index < end):
        raise ValueError(f"the '## Changes' section still has the '{PLACEHOLDER}' placeholder")
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
        section = extract(args.notes.read_text(encoding="utf-8-sig"), args.publish)
    except (OSError, UnicodeDecodeError, ValueError) as error:
        print(f"ERROR: {args.notes}: {error}", file=sys.stderr)
        return 1
    # Ghi byte trực tiếp để có LF trên mọi hệ điều hành (Python 3.9 chưa có write_text(newline=)).
    args.output.write_bytes((section + "\n").encode("utf-8"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
