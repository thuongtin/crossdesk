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
    (re.compile(r"<![A-Z]"), re.compile(r">")),
    (re.compile(r"<!\[CDATA\["), re.compile(r"\]\]>")),
)
# Khối HTML loại 6 (thẻ khối) và 7 (một thẻ đứng riêng trên dòng) kết thúc trước dòng trống kế tiếp.
HTML_BLOCK_TAGS = ("address|article|aside|base|basefont|blockquote|body|caption|center|col|colgroup|dd|"
                   "details|dialog|dir|div|dl|dt|fieldset|figcaption|figure|footer|form|frame|frameset|"
                   "h[1-6]|head|header|hr|html|iframe|legend|li|link|main|menu|menuitem|nav|noframes|ol|"
                   "optgroup|option|p|param|section|source|summary|table|tbody|td|tfoot|th|thead|"
                   "title|tr|track|ul")
HTML_TAG_BLOCK = re.compile(rf"</?(?:{HTML_BLOCK_TAGS})(?:[ >]|/>|$)", re.I)
HTML_ATTRIBUTE = r"""(?: +[A-Za-z_:][A-Za-z0-9_.:-]*(?: *= *(?:[^ "'=<>`]+|'[^']*'|"[^"]*"))?)"""
HTML_LONE_TAG = re.compile(rf"(?:<[A-Za-z][A-Za-z0-9-]*{HTML_ATTRIBUTE}* */?>|</[A-Za-z][A-Za-z0-9-]* *>) *$")
ENDS_AT_BLANK_LINE = "blank line"
# Dòng phân cách của bảng GFM như "| --- | :-: |": dòng cuối của đoạn văn ngay trên thành hàng tiêu đề
# khi hai dòng có cùng số ô. cmark-gfm chỉ thử mẫu này khi dòng không mở khối nào khác.
TABLE_DELIMITER_ROW = re.compile(r"^\|?[ \t\v\f]*:?-+:?[ \t\v\f]*(?:\|[ \t\v\f]*:?-+:?[ \t\v\f]*)*(?:\|[ \t\v\f]*)?$")

# Các mẫu dưới đây chạy trên nội dung nội dòng của một đoạn văn, tiêu đề hay ô bảng, theo cmark-gfm.
ASCII_PUNCTUATION = frozenset("!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~")
SPACE = r"[ \t\v\f\r\n]"
SPACES = re.compile(f"{SPACE}*")
# Khoảng trắng của định nghĩa link tham chiếu: dấu cách hay tab, tối đa một lần xuống dòng.
LINK_SPACE = re.compile(r"[ \t]*(?:\n[ \t]*)?")
# Autolink như <https://example.com> hay <team@example.com> là link, không phải HTML.
AUTOLINK = re.compile(r"<[A-Za-z][A-Za-z0-9.+-]{1,31}:[^\x00-\x20<>]*>"
                      r"|<[A-Za-z0-9.!#$%&'*+/=?^_`{|}~-]+@[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?"
                      r"(?:\.[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)*>")
# HTML thô: thẻ mở, thẻ đóng, comment, chỉ thị xử lý, khai báo và CDATA; thẻ có thể sang dòng sau.
TAG_NAME = r"[A-Za-z][A-Za-z0-9-]*"
ATTRIBUTE = (rf"{SPACE}+[A-Za-z_:][A-Za-z0-9:._-]*"
             rf"""(?:{SPACE}*={SPACE}*(?:[^ \t\r\n\v\f"'=<>`\x00]+|'[^'\x00]*'|"[^"\x00]*"))?""")
RAW_HTML = re.compile(rf"<{TAG_NAME}(?:{ATTRIBUTE})*{SPACE}*/?>|</{TAG_NAME}{SPACE}*>"
                      r"|<!---?>|<!--(?:[^-\x00]|-[^-\x00]|--[^>\x00])*-->"
                      r"|<\?(?:[^?>\x00]|\?[^>\x00]|>)*\?>"
                      rf"|<![A-Z]+{SPACE}[^>\x00]*>"
                      r"|<!\[CDATA\[(?:[^\]\x00]|\][^\]\x00]|\]\][^>\x00])*\]\]>")
BACKTICK_RUN = re.compile(r"`+")
MAX_BACKTICKS = 1000
# Độ dài tối đa tính bằng byte UTF-8 của nhãn "[...]" mà cmark-gfm nhận cho định nghĩa link.
MAX_LINK_LABEL_BYTES = 1000
# Ký tự đóng của từng kiểu tiêu đề link: "...", '...' và (...).
LINK_TITLE_CLOSERS = {'"': '"', "'": "'", "(": ")"}

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


def table_cells(text):
    """Các ô của text khi đọc như một hàng bảng GFM; list rỗng khi text không phải hàng bảng.

    Bỏ một dấu '|' mở đầu. Dấu '|' sau '\\' là chữ trong ô, còn '|' cuối dòng chỉ đóng ô cuối.
    """
    text = text.rstrip(" \t\v\f\r\n")
    if text.startswith("|"):
        text = text[1:]
    if not text.strip(" \t\v\f"):
        return []
    cells = []
    start = index = 0
    while index < len(text):
        if text[index] == "\\":
            index += 2
            continue
        if text[index] == "|":
            cells.append(text[start:index])
            start = index + 1
        index += 1
    if start < len(text):
        cells.append(text[start:])
    return cells


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
    """Phân loại từng dòng theo khối của cmark-gfm để tìm ranh giới mục và HTML.

    Trả về (kinds, unclosed, html_lines). kinds[i] là "code" cho dòng nằm trong code fence cấp cao
    nhất (kể cả dòng đóng, trừ dòng mở), "html" cho dòng của khối HTML ở mọi cấp, "setext" cho dòng
    gạch dưới biến đoạn văn cấp cao nhất thành tiêu đề, và "text" cho mọi dòng khác. unclosed là (chỉ
    số dòng mở, tên khối) của fence hay khối HTML loại 1 tới 5 cấp cao nhất vẫn mở khi hết notes, hoặc
    None. html_lines là tập chỉ số dòng nơi HTML thô nội dòng bắt đầu, trong đoạn văn, tiêu đề hay ô
    bảng ở mọi cấp.

    Fence thụt một tới ba dấu cách vẫn ở cấp cao nhất, trừ khi nó thuộc một mục list hay trích dẫn,
    như dòng đóng "  ```" của mục "- ```sh". Vì vậy scan giữ ngăn xếp khối chứa đang mở cùng khối lá
    trong cùng (fence, HTML, đoạn văn hay bảng), để biết dòng ít thụt lề hơn có nối lười vào đoạn văn
    của khối chứa hay kết thúc nó.
    """
    kinds = []
    # Nội dung nội dòng cần tìm HTML: (loại, các (chỉ số dòng, chữ)). Loại "paragraph" có thể mở đầu
    # bằng định nghĩa link tham chiếu; "text" là đoạn văn đứng trước bảng, "heading" và "cell" chỉ một dòng.
    units = []
    containers = []  # khối chứa đang mở, từ ngoài vào trong: None là trích dẫn, list là mục list
    fence = None  # (ký tự, độ dài) của code fence đang mở trong khối chứa trong cùng
    html_end = None  # điều kiện đóng của khối HTML đang mở trong khối chứa trong cùng
    paragraph = None  # các (chỉ số dòng, chữ) của đoạn văn đang mở; dòng sau có thể nối lười vào nó
    header = ""  # dòng cuối của đoạn văn đang mở, thành hàng tiêu đề khi dòng sau là dòng phân cách
    table_visited = False  # đoạn văn đang mở đã gặp dòng phân cách lệch số ô nên không thành bảng nữa
    table = False  # khối lá đang mở là bảng GFM
    unclosed = None
    for index, raw in enumerate(lines):
        rest, matched = match_containers(raw.expandtabs(4), containers)
        blank = not rest.strip(" ")
        all_matched = matched == len(containers)
        if all_matched:
            if fence is not None:
                kinds.append("text" if containers else "code")
                if closes_fence(rest, fence):
                    fence = unclosed = None
                continue
            if html_end is not None and not (html_end is ENDS_AT_BLANK_LINE and blank):
                kinds.append("html")
                if html_end is not ENDS_AT_BLANK_LINE and html_end.search(rest):
                    html_end = unclosed = None
                continue
        body = rest.lstrip(" ")
        # Bảng chỉ nhận tiếp dòng là một hàng bảng; dòng mở khối khác kết thúc bảng.
        table_row = table and all_matched and bool(table_cells(body))
        # Mở các khối mới ở đầu phần còn lại, theo thứ tự ưu tiên của cmark-gfm.
        in_paragraph = paragraph is not None and all_matched
        maybe_lazy = paragraph is not None
        opened = []
        leaf = None
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
                # Như cmark-gfm, bỏ ngay các định nghĩa link tham chiếu ở đầu đoạn văn, kể cả khi
                # sau đó đoạn văn thành bảng. Đoạn chỉ có định nghĩa thì rỗng, nên dòng gạch dưới là
                # chữ của nó.
                text = "".join(content + "\n" for _, content in paragraph)
                dropped = definitions_end = reference_definitions_end(text)
                while dropped:
                    dropped -= len(paragraph.pop(0)[1]) + 1
                if definitions_end < len(text):
                    leaf = "setext"
                break
            elif THEMATIC_BREAK.match(body):
                leaf = "break"
                break
            else:
                item = list_item(body, indent, in_paragraph)
                if item is None:
                    # Dòng phân cách cùng số ô với dòng cuối của đoạn văn biến dòng đó thành bảng.
                    if in_paragraph and not table_visited and TABLE_DELIMITER_ROW.match(body):
                        if len(table_cells(body)) == len(table_cells(header)):
                            leaf = "table"
                        else:
                            table_visited = True
                    break
                opened.append(item)
                rest = rest[item[0]:]
            in_paragraph = maybe_lazy = False
            body = rest.lstrip(" ")
        if body and paragraph is not None and not all_matched and not opened and leaf is None:
            # Dòng nối lười: chữ tiếp tục đoạn văn của khối chứa mà nó không thuộc về.
            paragraph.append((index, body))
            header = rest
            kinds.append("text")
            continue
        continues_paragraph = paragraph is not None and all_matched and not opened and leaf is None
        table_row = table_row and not opened and leaf is None
        del containers[matched:]
        containers.extend(opened)
        fence = html_end = None
        if continues_paragraph and body:
            paragraph.append((index, body))
            header = body
        elif table_row:
            units.extend(("cell", [(index, cell)]) for cell in table_cells(body))
        else:
            if leaf == "table":
                # Dòng cuối của đoạn văn thành hàng tiêu đề của bảng, các dòng trước vẫn là đoạn văn.
                header_index, header_text = paragraph.pop()
                # cmark-gfm tạo đoạn văn đứng trước bảng mà không tách định nghĩa link tham chiếu.
                units[-1] = ("text", paragraph)
                units.extend(("cell", [(header_index, cell)]) for cell in table_cells(header_text))
            paragraph = None
            table = leaf == "table"
            if body and leaf is None:
                paragraph = [(index, body)]
                units.append(("paragraph", paragraph))
                header = body
                table_visited = False
            elif leaf == "heading":
                units.append(("heading", [(index, body.lstrip("#"))]))
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
            kind = "html"
        elif leaf == "setext" and not containers:
            kind = "setext"
        kinds.append(kind)
    return kinds, unclosed, inline_html_lines(units)


def link_destination_end(text, position):
    """Vị trí ngay sau đích link bắt đầu tại position, hoặc None khi không có đích hợp lệ.

    Đích "<...>" không chứa xuống dòng hay '<' chưa thoát; đích không có ngoặc nhọn dừng ở khoảng
    trắng hay ')' không cặp, như manual_scan_link_url của cmark-gfm.
    """
    index = position
    if text.startswith("<", position):
        index += 1
        while index < len(text):
            if text[index] == ">":
                return index + 1 if index + 1 < len(text) else None
            if text[index] in "\n<":
                return None
            index += 2 if text[index] == "\\" else 1
        return None
    depth = 0
    while index < len(text):
        char = text[index]
        if char == "\\" and text[index + 1:index + 2] in ASCII_PUNCTUATION:
            index += 2
            continue
        if char == "(":
            depth += 1
            if depth > 32:
                return None
        elif char == ")":
            if depth == 0:
                break
            depth -= 1
        elif char in " \t\v\f\r\n":
            break
        index += 1
    return index if index < len(text) else None


def link_title_end(text, position):
    """Vị trí ngay sau tiêu đề link mở tại position, hoặc None, như scan_link_title của cmark-gfm.

    Máy quét của cmark-gfm lấy cách đọc dài nhất, nên ký tự đóng (và '(' trong tiêu đề kiểu ngoặc)
    chỉ nằm bên trong tiêu đề khi đứng ngay sau '\\', còn '\\' đứng trước ký tự đóng cuối cùng là chữ
    thường. Quét một lượt từ trái sang để thời gian luôn tuyến tính.
    """
    closer = LINK_TITLE_CLOSERS.get(text[position:position + 1])
    if closer is None:
        return None
    stops = "()" if closer == ")" else closer
    end = None
    index = position + 1
    while index < len(text) and text[index] != "\x00":
        if text[index] in stops:
            if text[index] == closer:
                end = index + 1
            if text[index - 1] != "\\":
                break
        index += 1
    return end


def link_tail_end(text, position):
    """Vị trí ngay sau phần "(đích "tiêu đề")" của link nội dòng mở tại position, hoặc None."""
    if not text.startswith("(", position):
        return None
    url_end = link_destination_end(text, SPACES.match(text, position + 1).end())
    if url_end is None:
        return None
    title_start = SPACES.match(text, url_end).end()
    title_end = link_title_end(text, title_start) if title_start > url_end else None
    end = SPACES.match(text, title_end or title_start).end()
    return end + 1 if text.startswith(")", end) else None


def line_end_after(text, position):
    """Vị trí sau dấu xuống dòng khi từ position chỉ còn dấu cách hay tab tới hết dòng, ngược lại None."""
    while text[position:position + 1] in (" ", "\t"):
        position += 1
    if text.startswith("\n", position):
        return position + 1
    return position if position == len(text) else None


def link_label_end(text, position):
    """Vị trí ngay sau nhãn "[...]" mở tại position, hoặc None, như link_label của cmark-gfm.

    Nhãn không chứa '[' hay ']' chưa thoát và dài tối đa MAX_LINK_LABEL_BYTES, tính cả ký tự thoát.
    Quét một lượt từ trái sang để thời gian luôn tuyến tính.
    """
    if not text.startswith("[", position):
        return None
    index = position + 1
    while index < len(text) and text[index] not in "[]":
        index += 2 if text[index] == "\\" and text[index + 1:index + 2] in ASCII_PUNCTUATION else 1
    if index >= len(text) or text[index] != "]":
        return None
    if len(text[position + 1:index].encode("utf-8")) > MAX_LINK_LABEL_BYTES:
        return None
    return index + 1


def reference_definitions_end(text):
    """Vị trí sau các định nghĩa link tham chiếu như "[guide]: <docs/a b.md>" ở đầu đoạn văn text.

    cmark-gfm bỏ các định nghĩa này khỏi đoạn văn, nên đích link trong ngoặc nhọn không phải HTML.
    """
    position = 0
    while True:
        label_end = link_label_end(text, position)
        if (label_end is None or not text.startswith(":", label_end)
                or not text[position + 1:label_end - 1].strip(" \t\v\f\r\n")):
            return position
        url_start = LINK_SPACE.match(text, label_end + 1).end()
        url_end = link_destination_end(text, url_start)
        if url_end is None or url_end == url_start:
            return position
        title_start = LINK_SPACE.match(text, url_end).end()
        title_end = link_title_end(text, title_start) if title_start > url_end else None
        end = None if title_end is None else line_end_after(text, title_end)
        if end is None:
            end = line_end_after(text, url_end)
        if end is None:
            return position
        position = end


def raw_html_offsets(text, position):
    """Sinh vị trí bắt đầu của từng đoạn HTML thô trong nội dung nội dòng text, tính từ position.

    Đọc từ trái sang như cmark-gfm: ký tự thoát, code span, autolink và phần "(đích "tiêu đề")" của
    link không chứa HTML, còn '<' không mở được thẻ nào như trong "a < b" hay "x<limit" là chữ.
    """
    brackets = []  # [là ảnh, còn mở được link] của từng '[' hay '![' chưa đóng, trong cùng ở cuối
    # Vị trí chuỗi backtick cuối cùng theo độ dài mà các lần tìm dấu đóng code span đã gặp. Như
    # scan_to_closing_backticks của cmark-gfm, sau một lần tìm tới hết mà không thấy dấu đóng, chuỗi
    # mở nằm sau vị trí đã ghi của độ dài đó là chữ thường dù phía sau vẫn có dấu đóng; GitHub cũng
    # hiện y như vậy.
    last_runs = {}
    scanned_to_end = False
    index = position
    while index < len(text):
        char = text[index]
        if char == "\\" and text[index + 1:index + 2] in ASCII_PUNCTUATION:
            index += 2
        elif char == "`":
            run_end = index
            while run_end < len(text) and text[run_end] == "`":
                run_end += 1
            # Code span đóng ở chuỗi backtick kế tiếp dài đúng bằng chuỗi mở, kể cả ở dòng sau.
            length = run_end - index
            index = run_end
            if length <= MAX_BACKTICKS and not (scanned_to_end and last_runs.get(length, 0) <= run_end):
                for run in BACKTICK_RUN.finditer(text, run_end):
                    if len(run.group()) <= MAX_BACKTICKS:
                        last_runs[len(run.group())] = run.start()
                    if len(run.group()) == length:
                        index = run.end()
                        break
                else:
                    scanned_to_end = True
        elif char == "<":
            link_or_html = AUTOLINK.match(text, index) or RAW_HTML.match(text, index)
            if link_or_html is not None and link_or_html.re is RAW_HTML:
                yield index
            index = index + 1 if link_or_html is None else link_or_html.end()
        elif char == "[" or text.startswith("![", index):
            brackets.append([char == "!", True])
            index += 1 if char == "[" else 2
        elif char == "]" and brackets:
            image, active = brackets.pop()
            tail = link_tail_end(text, index + 1) if active else None
            index = index + 1 if tail is None else tail
            if tail is not None and not image:
                # Link không lồng link, nên các '[' bao quanh nó không còn mở được link.
                for bracket in brackets:
                    if not bracket[0]:
                        bracket[1] = False
        else:
            index += 1


def inline_html_lines(units):
    """Chỉ số các dòng nơi HTML thô bắt đầu, trong các nội dung nội dòng mà scan gom được."""
    found = set()
    for unit_kind, entries in units:
        # Mỗi dòng kết thúc bằng xuống dòng như nội dung đoạn văn của cmark-gfm.
        text = "".join(content + "\n" for _, content in entries)
        if unit_kind == "cell":
            # cmark-gfm đổi "\|" trong ô thành "|" trước khi đọc nội dòng; ô chỉ có một dòng.
            text = text.replace("\\|", "|")
        start = reference_definitions_end(text) if unit_kind == "paragraph" else 0
        for offset in raw_html_offsets(text, start):
            for line_index, content in entries:
                offset -= len(content) + 1
                if offset < 0:
                    found.add(line_index)
                    break
    return found


def nested_fence(line):
    """True khi dòng mở code fence sau phần thụt lề, tab, dấu list hay dấu trích dẫn.

    InstalledBlocks chỉ hiểu fence ở cột 0, nên mục Changes có dòng như vậy bị từ chối thay vì hiện
    sai trong hộp thoại. Dòng giống fence trong khối code thụt lề cũng bị từ chối cho đơn giản.
    """
    prefix = CONTAINER_PREFIX.match(line).end()
    return prefix > 0 and opening_fence(line[prefix:]) is not None


def extract(text, publish=False):
    """Trả về nội dung mục `## Changes` đã chuẩn hoá; báo ValueError khi notes không dùng được.

    Mọi kiểm tra chạy trên dòng gốc, trước khi normalize bỏ thụt lề của dòng đầu mục. Với publish,
    dòng giữ chỗ của bản nháp cũng bị từ chối.
    """
    lines = text.replace("\r\n", "\n").split("\n")
    kinds, unclosed, html_lines = scan(lines)
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
        if kinds[index] == "html" or index in html_lines:
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
