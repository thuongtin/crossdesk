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

# Ranh giới mục chỉ tính tiêu đề ATX ở cột 0 nằm ngoài code fence và khối HTML ở cấp cao nhất. Dòng
# như vậy luôn kết thúc mọi mục list và trích dẫn đang mở, nên chỉ hai loại khối đó giấu được nó; scan
# theo dõi chúng theo cách cmark-gfm của GitHub dựng khối.
# Tiêu đề mở mục, có thể có chuỗi '#' đóng như "## Changes ##".
CHANGES_HEADING = re.compile(r"^##[ \t]+Changes(?:[ \t]+#+)?[ \t]*$")
# Mục Changes kết thúc ở tiêu đề cấp 1 hoặc 2 kế tiếp; tiêu đề cấp 3 trở xuống thuộc về mục.
SECTION_END = re.compile(r"^#{1,2}(?:[ \t]|$)")
# Tiêu đề cấp 1 hoặc 2 thụt một tới ba dấu cách có thể là tiêu đề thật, tiêu đề trong mục list hay chữ
# trong code fence của mục list; trong mục Changes nó bị từ chối để ranh giới mục không phải đoán.
INDENTED_SECTION_HEADING = re.compile(r"^ {1,3}#{1,2}(?:[ \t]|$)")
# Dòng mở code fence, tính từ ký tự đầu của text đã bỏ phần thụt lề; dòng trông như tiêu đề bên
# trong fence chỉ là chữ, ví dụ chú thích shell.
OPENING_FENCE = re.compile(r"^(`{3,}|~{3,})(.*)$")
# Dòng đóng code fence: thụt tối đa ba dấu cách, sau chuỗi fence chỉ còn dấu cách hoặc tab.
CLOSING_FENCE = re.compile(r"^ {0,3}(`{3,}|~{3,})[ \t]*$")
# Phần thụt lề, dấu trích dẫn '>' và dấu list đứng trước nội dung của dòng, ví dụ "  ", "- ", "> 1. ".
CONTAINER_PREFIX = re.compile(r"^[ \t>]*(?:(?:[-+*]|[0-9]{1,9}[.)])[ \t]+[ \t>]*)*")
# Đầu thẻ mở, thẻ đóng, comment, chỉ thị xử lý, khai báo hay CDATA của HTML trên dòng gốc. Thẻ chưa
# đóng trên dòng (thuộc tính có thể sang dòng sau) cũng tính là HTML.
INLINE_HTML = re.compile(r"<(?:[A-Za-z][A-Za-z0-9-]*(?:[ \t/>]|$)|/[A-Za-z][A-Za-z0-9-]*[ \t]*(?:>|$)"
                         r"|!--|\?|![A-Za-z]|!\[CDATA\[)")

# Các mẫu dưới đây chạy trên dòng đã đổi tab thành dấu cách và bỏ phần thụt lề.
ATX_HEADING = re.compile(r"^#{1,6}(?: |$)")
THEMATIC_BREAK = re.compile(r"^(?:(?:\* *){3,}|(?:- *){3,}|(?:_ *){3,})$")
# Dòng '=' hay '-' dưới một đoạn văn biến đoạn đó thành tiêu đề cấp 1 hoặc 2 (setext).
SETEXT_UNDERLINE = re.compile(r"^(?:=+|-+) *$")
# Dấu list theo sau là dấu cách hoặc hết dòng; nhóm 1 là số thứ tự của list có thứ tự.
LIST_MARKER = re.compile(r"(?:[-+*]|([0-9]{1,9})[.)])(?= |$)")
# Khối HTML loại 1 tới 5 của CommonMark: kéo dài tới hết dòng chứa chuỗi đóng, kể cả qua dòng trống.
HTML_CLOSED_BLOCKS = (
    (re.compile(r"<(?:pre|script|style|textarea)(?:[ >]|$)", re.I),
     re.compile(r"</(?:pre|script|style|textarea)>", re.I)),
    (re.compile(r"<!--"), re.compile(r"-->")),
    (re.compile(r"<\?"), re.compile(r"\?>")),
    (re.compile(r"<![A-Za-z]"), re.compile(r">")),
    (re.compile(r"<!\[CDATA\["), re.compile(r"\]\]>")),
)
# Khối HTML loại 6 (thẻ khối) và 7 (một thẻ đứng riêng trên dòng) kết thúc trước dòng trống kế tiếp.
HTML_BLOCK_TAGS = ("address|article|aside|base|basefont|blockquote|body|caption|center|col|colgroup|dd|"
                   "details|dialog|dir|div|dl|dt|fieldset|figcaption|figure|footer|form|frame|frameset|"
                   "h[1-6]|head|header|hr|html|iframe|legend|li|link|main|menu|menuitem|nav|noframes|ol|"
                   "optgroup|option|p|param|search|section|source|summary|table|tbody|td|tfoot|th|thead|"
                   "title|tr|track|ul")
HTML_TAG_BLOCK = re.compile(rf"</?(?:{HTML_BLOCK_TAGS})(?:[ >]|/>|$)", re.I)
HTML_ATTRIBUTE = r"""(?: +[A-Za-z_:][A-Za-z0-9_.:-]*(?: *= *(?:[^ "'=<>`]+|'[^']*'|"[^"]*"))?)"""
HTML_LONE_TAG = re.compile(rf"(?:<[A-Za-z][A-Za-z0-9-]*{HTML_ATTRIBUTE}* */?>|</[A-Za-z][A-Za-z0-9-]* *>) *$")
ENDS_AT_BLANK_LINE = "blank line"

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


def closes_fence(text, fence):
    """True khi text đóng fence (ký tự, độ dài) đang mở: cùng ký tự và dài ít nhất bằng dòng mở."""
    match = CLOSING_FENCE.match(text)
    return bool(match) and match.group(1)[0] == fence[0] and len(match.group(1)) >= fence[1]


def html_block(text, paragraph):
    """Điều kiện đóng khi text mở khối HTML theo CommonMark, ngược lại None.

    Trả về mẫu chuỗi đóng cho khối loại 1 tới 5 và ENDS_AT_BLANK_LINE cho loại 6 và 7. Khối loại 7
    không ngắt được đoạn văn, nên khi paragraph là True dòng đó là chữ nối tiếp đoạn văn.
    """
    for start, end in HTML_CLOSED_BLOCKS:
        if start.match(text):
            return end
    if HTML_TAG_BLOCK.match(text) or (not paragraph and HTML_LONE_TAG.match(text)):
        return ENDS_AT_BLANK_LINE
    return None


def list_item(body, indent, paragraph):
    """[cột nội dung, rỗng] khi body mở một mục list, ngược lại None.

    body là phần dòng sau indent dấu cách thụt lề; cột nội dung tính từ đầu phần thụt lề đó, và
    "rỗng" là True khi sau dấu list không còn gì. Khi paragraph là True, mục rỗng hay mục có số thứ
    tự khác 1 không ngắt được đoạn văn.
    """
    match = LIST_MARKER.match(body)
    if not match:
        return None
    rest = body[match.end():]
    empty = not rest.strip(" ")
    if paragraph and (empty or (match.group(1) is not None and int(match.group(1)) != 1)):
        return None
    spaces = len(rest) - len(rest.lstrip(" "))
    # Năm dấu cách trở lên sau dấu list mở khối code thụt lề trong mục, nội dung bắt đầu sau một cột.
    if empty or spaces > 4:
        return [indent + match.end() + 1, empty]
    return [indent + match.end() + spaces, empty]


def match_containers(line, containers):
    """Bỏ tiền tố của các khối chứa mà line còn thuộc về; trả về (phần còn lại, số khối khớp).

    Trích dẫn (None) cần dấu '>' thụt tối đa ba dấu cách. Mục list ([cột nội dung, rỗng]) nhận dòng
    trống và dòng thụt ít nhất bằng cột nội dung; mục mở bằng dòng chỉ có dấu list thì kết thúc ở
    dòng trống đầu tiên.
    """
    position = 0
    for matched, container in enumerate(containers):
        rest = line[position:]
        indent = len(rest) - len(rest.lstrip(" "))
        blank = not rest.strip(" ")
        if container is None:
            if blank or indent > 3 or rest[indent] != ">":
                return rest, matched
            position += indent + 1
            if line[position:position + 1] == " ":
                position += 1
        elif blank:
            if container[1]:
                return rest, matched
        elif indent >= container[0]:
            position += container[0]
            container[1] = False
        else:
            return rest, matched
    return line[position:], len(containers)


def scan(lines):
    """Phân loại từng dòng theo khối cấp cao nhất của CommonMark để tìm ranh giới mục.

    Trả về (kinds, unclosed). kinds[i] là "code" cho dòng nằm trong code fence cấp cao nhất (kể cả
    dòng đóng, trừ dòng mở), "html" cho dòng của khối HTML cấp cao nhất, "setext" cho dòng gạch dưới
    biến đoạn văn cấp cao nhất thành tiêu đề, và "text" cho mọi dòng khác. unclosed là (chỉ số dòng
    mở, tên khối) của fence hay khối HTML loại 1 tới 5 cấp cao nhất vẫn mở khi hết notes, hoặc None.

    Fence thụt một tới ba dấu cách vẫn ở cấp cao nhất, trừ khi nó thuộc một mục list hay trích dẫn,
    như dòng đóng "  ```" của mục "- ```sh". Vì vậy scan giữ ngăn xếp khối chứa đang mở cùng khối lá
    trong cùng (fence, HTML hay đoạn văn), để biết dòng ít thụt lề hơn có nối lười vào đoạn văn của
    khối chứa hay kết thúc nó.
    """
    kinds = []
    containers = []  # khối chứa đang mở, từ ngoài vào trong: None là trích dẫn, list là mục list
    fence = None  # (ký tự, độ dài) của code fence đang mở trong khối chứa trong cùng
    html_end = None  # điều kiện đóng của khối HTML đang mở trong khối chứa trong cùng
    paragraph = False  # khối lá đang mở là đoạn văn, nên dòng sau có thể nối lười vào nó
    unclosed = None
    for index, raw in enumerate(lines):
        rest, matched = match_containers(raw.expandtabs(4), containers)
        blank = not rest.strip(" ")
        if matched == len(containers):
            if fence is not None:
                kinds.append("text" if containers else "code")
                if closes_fence(rest, fence):
                    fence = unclosed = None
                continue
            if html_end is not None and not (html_end is ENDS_AT_BLANK_LINE and blank):
                kinds.append("text" if containers else "html")
                if html_end is not ENDS_AT_BLANK_LINE and html_end.search(rest):
                    html_end = unclosed = None
                continue
        # Mở các khối mới ở đầu phần còn lại, theo thứ tự ưu tiên của cmark.
        in_paragraph = paragraph and matched == len(containers)
        maybe_lazy = paragraph
        opened = []
        leaf = None
        body = rest.lstrip(" ")
        while body:
            indent = len(rest) - len(body)
            if indent > 3:
                # Khối code thụt lề, trừ khi dòng là chữ nối tiếp đoạn văn.
                if not maybe_lazy:
                    leaf = "code"
                break
            if body.startswith(">"):
                opened.append(None)
                rest = body[2:] if body.startswith("> ") else body[1:]
            elif ATX_HEADING.match(body):
                leaf = "heading"
                break
            elif opening_fence(body):
                run = opening_fence(body)
                leaf = ("fence", (run[0], len(run)))
                break
            elif html_block(body, in_paragraph) is not None:
                leaf = ("html", html_block(body, in_paragraph))
                break
            elif in_paragraph and SETEXT_UNDERLINE.match(body):
                leaf = "setext"
                break
            elif THEMATIC_BREAK.match(body):
                leaf = "break"
                break
            else:
                item = list_item(body, indent, in_paragraph)
                if item is None:
                    break
                opened.append(item)
                rest = rest[item[0]:]
            in_paragraph = maybe_lazy = False
            body = rest.lstrip(" ")
        if body and paragraph and matched < len(containers) and not opened and leaf is None:
            # Dòng nối lười: chữ tiếp tục đoạn văn của khối chứa mà nó không thuộc về.
            kinds.append("text")
            continue
        del containers[matched:]
        containers.extend(opened)
        fence = html_end = None
        paragraph = bool(body) and leaf is None
        kind = "text"
        if isinstance(leaf, tuple) and leaf[0] == "fence":
            fence = leaf[1]
            if not containers:
                unclosed = (index, "code fence")
        elif isinstance(leaf, tuple):
            end = leaf[1]
            if end is ENDS_AT_BLANK_LINE or not end.search(body):
                html_end = end
                if not containers and end is not ENDS_AT_BLANK_LINE:
                    unclosed = (index, "HTML block")
            kind = "text" if containers else "html"
        elif leaf == "setext" and not containers:
            kind = "setext"
        kinds.append(kind)
    return kinds, unclosed


def nested_fence(line):
    """True khi dòng mở code fence sau phần thụt lề, tab, dấu list hay dấu trích dẫn.

    InstalledBlocks chỉ hiểu fence ở cột 0, nên mục Changes có dòng như vậy bị từ chối thay vì hiện
    sai trong hộp thoại. Dòng giống fence trong khối code thụt lề cũng bị từ chối cho đơn giản.
    """
    prefix = CONTAINER_PREFIX.match(line).end()
    return prefix > 0 and opening_fence(line[prefix:]) is not None


def strip_code_spans(line):
    """Thay code span và ký tự thoát bằng '\\' trên line bằng dấu cách, giữ nguyên độ dài dòng.

    Code span tính trong từng dòng: chuỗi backtick mở khớp chuỗi backtick kế tiếp dài đúng bằng nó.
    """
    parts = []
    position = 0
    while position < len(line):
        if line[position] == "\\":
            parts.append("  "[:len(line) - position])
            position += 2
            continue
        if line[position] != "`":
            parts.append(line[position])
            position += 1
            continue
        run_end = position
        while run_end < len(line) and line[run_end] == "`":
            run_end += 1
        closing = re.compile(rf"(?<!`){line[position:run_end]}(?!`)").search(line, run_end)
        if closing is None:
            parts.append(line[position:run_end])
            position = run_end
        else:
            parts.append(" " * (closing.end() - position))
            position = closing.end()
    return "".join(parts)


def contains_html(line):
    """True khi line có thẻ HTML, comment hay chỉ thị HTML nằm ngoài code span.

    Slint chỉ hiểu vài thẻ nội dòng, nên hộp thoại hiện cả dòng có HTML thành chữ thô, kể cả comment
    mà GitHub giấu đi. Autolink như <https://example.com> và dấu '<' trong "a < b" không phải HTML.
    Dòng như vậy trong khối code thụt lề cũng bị từ chối cho đơn giản.
    """
    return INLINE_HTML.search(strip_code_spans(line)) is not None


def extract(text, publish=False):
    """Trả về nội dung mục `## Changes` đã chuẩn hoá; báo ValueError khi notes không dùng được.

    Mọi kiểm tra chạy trên dòng gốc, trước khi normalize bỏ thụt lề của dòng đầu mục. Với publish,
    dòng giữ chỗ của bản nháp cũng bị từ chối.
    """
    lines = text.replace("\r\n", "\n").split("\n")
    kinds, unclosed = scan(lines)
    candidates = [index for index, kind in enumerate(kinds) if kind == "text"]
    start = next((index for index in candidates if CHANGES_HEADING.match(lines[index])), None)
    end = len(lines) if start is None else next(
        (index for index in candidates if index > start and SECTION_END.match(lines[index])), len(lines))
    # Khối chưa đóng nuốt hết phần sau nó, kể cả tiêu đề của mục và các mục kế tiếp.
    if unclosed is not None and unclosed[0] < end:
        raise ValueError(f"line {unclosed[0] + 1}: close the {unclosed[1]} that starts here")
    if start is None:
        raise ValueError("release notes have no '## Changes' heading at column 0")
    # Dòng trong code fence ở cột 0 là chữ, kể cả "- TODO" làm ví dụ.
    section_indexes = [index for index in range(start + 1, end) if kinds[index] != "code"]
    for index in section_indexes:
        if INDENTED_SECTION_HEADING.match(lines[index]):
            raise ValueError(f"line {index + 1}: start level 1 and 2 headings at column 0")
        if kinds[index] == "setext":
            raise ValueError(f"line {index + 1}: a line of '=' or '-' under text makes it a level 1 "
                             "or 2 heading, so put a blank line before this line")
        if nested_fence(lines[index]):
            raise ValueError(f"line {index + 1}: start code fences at column 0, "
                             "not indented or inside a list item or quote")
        if contains_html(lines[index]):
            raise ValueError(f"line {index + 1}: the Release Notes dialog shows HTML as plain text, "
                             "so remove the HTML or put it in backticks")
    if publish and any(lines[index].strip() == PLACEHOLDER for index in section_indexes):
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
