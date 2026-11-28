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
import unicodedata
from bisect import bisect_left
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
# Dòng mở định nghĩa chú thích cuối trang (footnote) của GitHub như "[^note]: chữ"; nhãn không có dấu
# cách. Như cmark-gfm, khối này đứng sau đường kẻ và trước mục list trong thứ tự thử mở khối. Nhóm 1
# là nhãn.
FOOTNOTE_DEFINITION = re.compile(r"^\[\^([^\] \t\r\n\x00]+)\]:[ \t]*")
# Khối chứa của một định nghĩa footnote trong ngăn xếp của scan: nội dung thụt bốn cột.
FOOTNOTE = "footnote"
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
# Khoảng trắng theo cmark_isspace của cmark-gfm: nhãn link và footnote gộp, cắt theo tập này, và đích
# link không có ngoặc nhọn dừng ở đây. Khác chỗ cách giữa các phần của link nội dòng hay thẻ HTML,
# "\v" và "\f" ở đây là chữ thường, nên "[\v]" là một nhãn được và "b\vc" là một đích.
CMARK_SPACE = " \t\r\n"
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
# Tiện ích autolink của GitHub biến URL trần như "https://example.com/a" hay "www.example.com" thành link.
ASCII_LETTERS = frozenset("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")
BARE_URL_SCHEMES = ("http://", "https://", "ftp://")
# Khoảng trắng Unicode theo cmark_utf8proc_is_space của cmark-gfm; "\v" không nằm trong đó.
UNICODE_SPACE = frozenset("\t\n\f\r \u00a0\u1680\u202f\u205f\u3000" + "".join(map(chr, range(0x2000, 0x200b))))
# Ký tự trước "www." được mở link: ngoài đầu nội dung và khoảng trắng của cmark_isspace.
WWW_PREFIXES = frozenset("*_~(" + CMARK_SPACE)
# Khoảng trắng theo cmark_utf8proc_is_space mà check_domain dừng ở đó; "\v" vẫn là ký tự của tên miền.
DOMAIN_SPACE = frozenset(" \t\n\f\r")
# Ký tự cuối link mà autolink_delim bỏ ra khỏi link.
URL_TRAILING = frozenset("?!.,:*_~'\"")
# Dòng mở mục task list của GitHub như "- [x] Done", xét trên cả dòng gốc như tiện ích tasklist của
# cmark-gfm, nên "> - [x]" hay "- - [x]" không phải task.
TASK_LIST_ITEM = re.compile(r"[ \t\v\f]*(?:[-+*]|[0-9]+[.)])[ \t\v\f]+\[[ xX]\][ \t\v\f]")
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

    Bỏ một dấu '|' mở đầu, còn '|' cuối dòng chỉ đóng ô cuối. Như cmark-gfm, '|' đứng ngay sau '\\'
    là chữ trong ô, kể cả khi '\\' đó lại đứng sau một '\\' khác như trong `D:\\\\|x`.
    """
    text = text.rstrip(" \t\v\f\r\n")
    if text.startswith("|"):
        text = text[1:]
    if not text.strip(" \t\v\f"):
        return []
    cells = []
    start = index = 0
    while index < len(text):
        if text[index] == "|" and text[index - 1:index] != "\\":
            cells.append(text[start:index])
            start = index + 1
        index += 1
    if start < len(text):
        cells.append(text[start:])
    return cells


def table_row_end(text, width):
    """Vị trí ngay sau dấu '|' đóng ô thứ width của hàng bảng text, hay None khi hàng không có ô thừa.

    Tách ô y như table_cells, nên hàng có nhiều hơn width ô khi và chỉ khi sau dấu '|' đó còn chữ.
    """
    text = text.rstrip(" \t\v\f\r\n")
    count = 0
    index = 1 if text.startswith("|") else 0
    while index < len(text):
        if text[index] == "|" and text[index - 1:index] != "\\":
            count += 1
            if count == width:
                return index + 1 if index + 1 < len(text) else None
        index += 1
    return None


def raw_text_at(raw, column):
    """Phần của dòng gốc raw bắt đầu ở cột column của raw.expandtabs(4).

    cmark-gfm chỉ mở rộng tab để đo thụt lề, còn nội dung đoạn văn giữ nguyên tab, nên độ dài nhãn
    link phải đếm trên chữ gốc. Khi column nằm giữa một tab mà khối chứa đã dùng mất một phần, như
    add_line của cmark-gfm, phần còn lại của tab thành dấu cách.
    """
    position = 0
    for index, char in enumerate(raw):
        if position >= column:
            return raw[index:]
        position += 4 - position % 4 if char == "\t" else 1
        if position > column:
            return " " * (position - column) + raw[index + 1:]
    return ""


def raw_column_after(raw, column, count):
    """Cột của raw.expandtabs(4) sau count ký tự gốc, tính từ ký tự gốc chứa cột column.

    Như cmark_parser_advance_offset với columns là False, mỗi ký tự gốc tính là một, kể cả tab mà
    khối chứa đã dùng mất một phần.
    """
    position = 0
    for index, char in enumerate(raw):
        width = 4 - position % 4 if char == "\t" else 1
        if position + width > column:
            return len(raw[:index + count].expandtabs(4))
        position += width
    return position


def match_containers(line, containers):
    """Bỏ tiền tố của các khối chứa mà line còn thuộc về; trả về (phần còn lại, số khối khớp).

    Trích dẫn (None) cần dấu '>' thụt tối đa ba dấu cách. Mục list ([cột nội dung, rỗng]) nhận dòng
    trống và dòng thụt ít nhất bằng cột nội dung; mục mở bằng dòng chỉ có dấu list thì kết thúc ở
    dòng trống đầu tiên. Footnote (FOOTNOTE) nhận dòng thụt ít nhất bốn cột, còn dòng trống thì chỉ
    nhận khi cả dòng rỗng: như cmark-gfm, dòng chỉ có vài dấu cách hay chỉ có '>' kết thúc footnote.
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
        elif container == FOOTNOTE:
            if indent >= 4:
                position += 4
            elif line:
                return rest, matched
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

    Trả về (kinds, unclosed, inline, quotes, trimmed). kinds[i] là "code" cho dòng nằm trong code fence cấp cao nhất
    (kể cả dòng đóng, trừ dòng mở), "footnote" cho dòng mở định nghĩa footnote ở mọi cấp, "html" cho
    các dòng khác của khối HTML ở mọi cấp, "setext" cho dòng gạch dưới biến đoạn văn ở mọi cấp thành
    tiêu đề, và "text" cho mọi dòng khác. unclosed là (chỉ số dòng mở, tên khối) của fence hay khối
    HTML loại 1 tới 5 cấp cao nhất vẫn mở khi hết notes, hoặc None. inline là kết quả của
    inline_lines cho nội dung nội dòng của đoạn văn, tiêu đề và ô bảng ở mọi cấp, trừ ô thừa mà
    GitHub bỏ đi. quotes là tập chỉ số các dòng mở trích dẫn ở mọi cấp. trimmed là dict từ chỉ số
    dòng của hàng thân bảng có ô thừa, ở mọi cấp, tới dòng gốc đã cắt sau dấu '|' đóng ô cuối mà
    GitHub giữ lại.

    Fence thụt một tới ba dấu cách vẫn ở cấp cao nhất, trừ khi nó thuộc một mục list hay trích dẫn,
    như dòng đóng "  ```" của mục "- ```sh". Vì vậy scan giữ ngăn xếp khối chứa đang mở cùng khối lá
    trong cùng (fence, HTML, đoạn văn hay bảng), để biết dòng ít thụt lề hơn có nối lười vào đoạn văn
    của khối chứa hay kết thúc nó.
    """
    kinds = []
    # Nội dung nội dòng cần tìm HTML: (loại, các (chỉ số dòng, chữ)). Loại "paragraph" có thể mở đầu
    # bằng định nghĩa link tham chiếu; "text" là đoạn văn đứng trước bảng, "heading" và "cell" chỉ
    # một dòng.
    units = []
    trimmed = {}
    # Khối chứa đang mở, từ ngoài vào trong: None là trích dẫn, list là mục list, FOOTNOTE là footnote.
    containers = []
    fence = None  # (ký tự, độ dài) của code fence đang mở trong khối chứa trong cùng
    html_end = None  # điều kiện đóng của khối HTML đang mở trong khối chứa trong cùng
    paragraph = None  # các (chỉ số dòng, chữ) của đoạn văn đang mở; dòng sau có thể nối lười vào nó
    header = ""  # dòng cuối của đoạn văn đang mở, thành hàng tiêu đề khi dòng sau là dòng phân cách
    table_visited = False  # đoạn văn đang mở đã gặp dòng phân cách lệch số ô nên không thành bảng nữa
    table = False  # khối lá đang mở là bảng GFM
    table_width = 0  # số ô của hàng tiêu đề của bảng đang mở; GitHub bỏ các ô thừa của hàng thân bảng
    quotes = set()
    unclosed = None
    footnote_labels = []  # khoá label_key của nhãn mọi định nghĩa footnote
    for index, raw in enumerate(lines):
        expanded = raw.expandtabs(4)
        rest, matched = match_containers(expanded, containers)
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
        # Khối mà cmark-gfm đưa cho tiện ích tasklist khi dòng không mở khối chứa mới có phải mục list
        # không: mục list trong cùng còn khớp, khi nó không có đoạn văn hay bảng đang mở. Khi một mục
        # list lồng bên trong không khớp, khối đó là list chứa mục lồng chứ không phải mục list.
        if all_matched:
            item_container = (bool(containers) and isinstance(containers[-1], list)
                              and not in_paragraph and not table)
        else:
            item_container = (matched > 0 and isinstance(containers[matched - 1], list)
                              and not isinstance(containers[matched], list))
        opened = []
        leaf = None
        while body:
            indent = len(rest) - len(body)
            # GitHub hiện ô đánh dấu thay cho "[x]" ở đầu mục task list, nên đó không phải link tham
            # chiếu. Tiện ích tasklist của cmark-gfm chỉ chạy khi không khối nào khác mở được ở đây,
            # rồi bỏ đúng ba ký tự gốc tính từ chỗ đang đọc, kể cả khi đó là "\v[x" của "- \v[x] Done"
            # hay "-\v[" của dòng nối tiếp mục "-\v[x] Done". Phần còn lại luôn là chữ của đoạn văn.
            task = (isinstance(opened[-1], list) if opened else item_container) and TASK_LIST_ITEM.match(raw)
            if indent > 3:
                # Khối code thụt lề, trừ khi dòng là chữ nối tiếp đoạn văn.
                if not maybe_lazy:
                    leaf = "code"
                elif task:
                    rest = expanded[raw_column_after(raw, len(expanded) - len(rest), 3):]
                    body = rest.lstrip(" ")
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
                definitions = []
                while dropped > 0:
                    definitions.append(paragraph.pop(0))
                    dropped -= len(definitions[-1][1]) + 1
                # Nhãn của các định nghĩa đã bỏ vẫn mở được link tham chiếu ở chỗ khác trong notes. Phần
                # đứng trước đoạn văn để đoạn văn vẫn là phần cuối khi nó thành bảng.
                if definitions:
                    units.insert(-1, ("paragraph", definitions))
                if definitions_end < len(text):
                    leaf = "setext"
                break
            elif THEMATIC_BREAK.match(body):
                leaf = "break"
                break
            elif FOOTNOTE_DEFINITION.match(body):
                opened.append(FOOTNOTE)
                footnote_labels.append(label_key(FOOTNOTE_DEFINITION.match(body).group(1)))
                rest = body[FOOTNOTE_DEFINITION.match(body).end():]
            else:
                item = list_item(body, indent, in_paragraph)
                if item is None:
                    if task:
                        rest = expanded[raw_column_after(raw, len(expanded) - len(rest), 3):]
                        body = rest.lstrip(" ")
                    # Dòng phân cách cùng số ô với dòng cuối của đoạn văn biến dòng đó thành bảng.
                    elif in_paragraph and not table_visited and TABLE_DELIMITER_ROW.match(body):
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
            # Dòng nối lười: chữ tiếp tục đoạn văn của khối chứa mà nó không thuộc về. Khác dòng nối
            # thường, cmark-gfm giữ khoảng trắng đầu dòng này, nên "   [b]: /v" nối lười không còn là
            # định nghĩa link tham chiếu.
            paragraph.append((index, raw_text_at(raw, len(expanded) - len(rest))))
            header = rest
            kinds.append("text")
            continue
        continues_paragraph = paragraph is not None and all_matched and not opened and leaf is None
        table_row = table_row and not opened and leaf is None
        del containers[matched:]
        containers.extend(opened)
        fence = html_end = None
        if continues_paragraph and body:
            paragraph.append((index, raw_text_at(raw, len(expanded) - len(body))))
            header = body
        elif table_row:
            units.extend(("cell", [(index, cell)]) for cell in table_cells(body)[:table_width])
            # GitHub bỏ các ô vượt quá số ô của hàng tiêu đề, còn hộp thoại đưa cả dòng cho Markdown
            # của Slint: chữ của các ô đó vẫn hiện, và ảnh hay HTML trong đó làm Slint từ chối cả dòng
            # nên link ở các ô còn lại không bấm được. Cắt chúng khỏi notes nhúng để hộp thoại hiện
            # đúng các ô GitHub hiện. Tab không phải '|' hay '\', nên tách ô trên chữ gốc cũng cho
            # cùng các ô, và phần sau chỗ cắt là đuôi của dòng gốc.
            raw_body = raw_text_at(raw, len(expanded) - len(body))
            end = table_row_end(raw_body, table_width)
            if end is not None:
                trimmed[index] = raw[:len(raw) - (len(raw_body) - end)]
        else:
            if leaf == "table":
                # Dòng cuối của đoạn văn thành hàng tiêu đề của bảng, các dòng trước vẫn là đoạn văn.
                header_index, header_text = paragraph.pop()
                # cmark-gfm tạo đoạn văn đứng trước bảng mà không tách định nghĩa link tham chiếu.
                units[-1] = ("text", paragraph)
                table_width = len(table_cells(header_text))
                units.extend(("cell", [(header_index, cell)]) for cell in table_cells(header_text))
            paragraph = None
            table = leaf == "table"
            if body and leaf is None:
                paragraph = [(index, raw_text_at(raw, len(expanded) - len(body)))]
                units.append(("paragraph", paragraph))
                header = body
                table_visited = False
            elif leaf == "heading":
                units.append(("heading", [(index, heading_text(body))]))
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
        elif leaf == "setext":
            kind = "setext"
        if FOOTNOTE in opened:
            kind = "footnote"
        if None in opened:
            quotes.add(index)
        kinds.append(kind)
    return kinds, unclosed, inline_lines(units, frozenset(footnote_labels)), quotes, trimmed


def link_destination_end(text, position):
    """Vị trí ngay sau đích link bắt đầu tại position, hoặc None khi không có đích hợp lệ.

    Đích "<...>" không chứa xuống dòng hay '<' chưa thoát; đích không có ngoặc nhọn dừng ở khoảng
    trắng CMARK_SPACE hay ')' không cặp, như manual_scan_link_url của cmark-gfm.
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
        elif char in CMARK_SPACE:
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


def label_key(label):
    """Khoá tra cứu của nhãn link tham chiếu: gộp khoảng trắng và bỏ khác biệt hoa thường như cmark-gfm."""
    return re.sub(f"[{CMARK_SPACE}]+", " ", label).strip(" ").casefold()


def reference_definitions_end(text, labels=None):
    """Vị trí sau các định nghĩa link tham chiếu như "[guide]: <docs/a b.md>" ở đầu đoạn văn text.

    cmark-gfm bỏ các định nghĩa này khỏi đoạn văn, nên đích link trong ngoặc nhọn không phải HTML.
    Khi có labels, khoá label_key của nhãn từng định nghĩa được thêm vào list đó.
    """
    position = 0
    while True:
        label_end = link_label_end(text, position)
        label = None if label_end is None else text[position + 1:label_end - 1]
        if label is None or not text.startswith(":", label_end) or not label.strip(CMARK_SPACE):
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
        if labels is not None:
            labels.append(label_key(label))
        position = end


def link_end(text, close, content_start, bracket_after, definitions):
    """Vị trí ngay sau link mà ']' tại close đóng, hoặc None khi chữ trong ngoặc không thành link.

    Như handle_close_bracket của cmark-gfm: thử phần "(...)" của link nội dòng, rồi nhãn "[...]" ngay
    sau ']', rồi chính chữ trong ngoặc khi nhãn đó rỗng hay không có. Nhãn chỉ mở link khi khoá của
    nó có trong definitions; chữ trong ngoặc chỉ làm nhãn được khi không chứa '[' mở ngoặc khác
    (bracket_after là False).
    """
    after = close + 1
    tail = link_tail_end(text, after)
    if tail is not None:
        return tail
    label_end = link_label_end(text, after)
    label = None if label_end is None else text[after + 1:label_end - 1]
    if (label is None or not label.strip(CMARK_SPACE)) and not bracket_after:
        label = text[content_start:close]
        if label_end is None:
            label_end = after
    if label is not None and label_key(label) in definitions:
        return label_end
    return None


def domain_length(text, begin, limit, short):
    """Số ký tự của tên miền bắt đầu tại begin như check_domain của cmark-gfm, hoặc 0 khi không nhận.

    Tên miền dừng ở dấu câu ASCII trừ '-', ở khoảng trắng hay ký tự ngoài ASCII; '\\' bỏ qua ký tự
    ngay sau nó. Tên miền có '_' trong hai phần cuối bị từ chối, và khi short là False nó phải có ít
    nhất một dấu chấm. limit là cuối nội dung, ký tự ngay trước limit không được xét. Trả về cặp
    (số ký tự, vị trí dấu chấm áp chót mà lần quét đi qua hoặc -1).
    """
    if text[begin] >= "\x80":
        # cmark-gfm không xét ký tự đầu nhưng đọc từng byte, nên ký tự đầu nhiều byte như "é" dừng
        # vòng lặp ngay ở byte thứ hai của nó.
        return (1 if short else 0), -1
    size = limit - begin
    offset = 1
    dots = underscores_before = underscores_last = 0
    previous_dot = last_dot = -1
    while offset < size - 1:
        if text[begin + offset] == "\\" and offset < size - 2:
            offset += 1
        char = text[begin + offset]
        if char == "_":
            underscores_last += 1
        elif char == ".":
            underscores_before, underscores_last = underscores_last, 0
            previous_dot, last_dot = last_dot, begin + offset
            dots += 1
        elif char != "-" and (char in ASCII_PUNCTUATION or char in DOMAIN_SPACE
                              or not "\x00" < char < "\x80"):
            # cmark-gfm đọc từng byte, nên ký tự nhiều byte dừng vòng lặp ở byte thứ hai; NUL đã thành
            # U+FFFD từ trước.
            break
        offset += 1
    # Như cmark-gfm, tên miền có hơn mười dấu chấm được nhận dù có '_' để tránh quét bậc hai.
    if (underscores_before or underscores_last) and dots <= 10:
        return 0, previous_dot
    return (offset if short or dots else 0), previous_dot


def url_delimiter_end(text, begin, end):
    """Cuối link sau khi bỏ dấu câu ở cuối như autolink_delim của cmark-gfm; begin là đầu dữ liệu.

    Bỏ các ký tự trong URL_TRAILING, bỏ cả thực thể "&name;" ở cuối, và bỏ ')' khi số ')' từ begin tới
    cuối link nhiều hơn số '('.
    """
    opening = text.count("(", begin, end)
    closing = text.count(")", begin, end)
    while end > begin:
        last = text[end - 1]
        if last in URL_TRAILING:
            end -= 1
        elif last == ";":
            entity = end - 2
            while entity > begin and text[entity] in ASCII_LETTERS:
                entity -= 1
            end = entity if entity < end - 2 and text[entity] == "&" else end - 1
        elif last == ")" and closing > opening:
            end -= 1
            closing -= 1
        else:
            break
    return end


def starts_host(char):
    """Ký tự ngay sau "://" có mở được tên miền của URL trần không, như is_valid_hostchar của cmark-gfm.

    Ký tự đó không được là khoảng trắng hay dấu câu Unicode, nên "http://\v" hay "http://é" vẫn mở
    link còn "http://_" hay "http://\u00a0" thì không. Bảng dấu câu của cmark-gfm cũ hơn unicodedata,
    nên khoảng 170 ký tự hiếm mới được Unicode xếp vào (hay chuyển khỏi) nhóm dấu câu bị xét khác
    cmark-gfm.
    """
    return (char not in UNICODE_SPACE and char not in ASCII_PUNCTUATION
            and not unicodedata.category(char).startswith("P"))


def url_start(text, index, start):
    """Vị trí đầu tên giao thức đứng ngay trước ':' tại index, không lùi quá start."""
    while index > start and text[index - 1] in ASCII_LETTERS:
        index -= 1
    return index


def bare_url_end(text, index, start, limit, rejected_before=-1):
    """Cặp (vị trí ngay sau URL trần mà tiện ích autolink của GitHub mở tại index hoặc None, mốc mới).

    index là ':' của "http://", "https://" hay "ftp://" (tên giao thức đứng ngay trước, không phân
    biệt hoa thường), hoặc 'w' của "www." đứng sau đầu nội dung, khoảng trắng hay một trong "*_~(".
    Như cmark-gfm, link chạy tới khoảng trắng hay '<' rồi bỏ dấu câu ở cuối, nên '[', '`' và '\\'
    bên trong là chữ của link. start là đầu nội dung, limit là cuối nội dung sau khi cắt khoảng trắng.
    rejected_before là mốc mà lần gọi trước trên cùng nội dung trả về: "www." mở trước mốc đó chắc
    chắn không thành link.
    """
    if limit - index < 4:
        return None, rejected_before
    if text[index] == ":":
        if not text.startswith("//", index + 1):
            return None, rejected_before
        scheme = url_start(text, index, start)
        if text[scheme:index + 3].lower() not in BARE_URL_SCHEMES or not starts_host(text[index + 3]):
            return None, rejected_before
        domain, _ = domain_length(text, index + 3, limit, True)
        if domain == 0:
            return None, rejected_before
        end = index + 3 + domain
    elif text.startswith("www.", index) and (index == start or text[index - 1] in WWW_PREFIXES):
        if index < rejected_before:
            # Lần quét từ một "www." trước trong cùng tên miền đã đi qua hai dấu chấm cuối và bị từ
            # chối, nên lần quét từ đây cũng dừng ở cùng chỗ, có cùng hai phần cuối và cũng bị từ chối.
            # Bỏ qua nó để chuỗi như "_www._www._www." không bị quét lại tới cuối ở mỗi "www.".
            return None, rejected_before
        domain, previous_dot = domain_length(text, index, limit, False)
        if domain == 0:
            return None, previous_dot
        end = index + domain
    else:
        return None, rejected_before
    while end < limit and text[end] not in CMARK_SPACE and text[end] != "<":
        end += 1
    end = url_delimiter_end(text, index, end)
    return (end if end > index else None), rejected_before


def inline_offsets(text, position, definitions=frozenset(), footnotes=frozenset()):
    """Các (vị trí, loại) mà hộp thoại hiện khác GitHub trong nội dung nội dòng text, tính từ position.

    Loại "html" là chỗ bắt đầu một đoạn HTML thô. Loại "reference" là dấu '[' hay '![' mở link hoặc ảnh
    lấy đích từ một định nghĩa (khoá label_key nằm trong definitions): hộp thoại đọc từng dòng riêng
    nên hiện nguyên chữ "[text][ref]", còn GitHub hiện link "text". Loại "footnote" là dấu '[' của chỗ
    gọi footnote có định nghĩa (khoá nằm trong footnotes): GitHub hiện số chú thích và đưa ghi chú
    xuống cuối trang, còn hộp thoại hiện nguyên chữ "[^note]". Loại "split" là dấu '[' của chỗ gọi
    footnote sang dòng sau, có định nghĩa hay không: cmark-gfm không lấy chữ trong ngoặc làm nhãn mà
    lấy số byte bằng hiệu số cột của ']' và '[' trừ hai, nên GitHub hiện số chú thích hay chữ khác
    hẳn, như "See [^compat" rồi "]" thành "See [^]". Loại "url" là đầu một URL trần mà tiện ích
    autolink của GitHub biến thành link ("https://...", "http://...", "ftp://..." hay "www...."): hộp
    thoại hiện nó thành chữ thường, không bấm được. Địa chỉ email không bị tính vì app chỉ mở link
    https, nên link mailto của GitHub cũng không mở được trong hộp thoại. Loại "image" là dấu '![' của
    ảnh nội dòng "![alt](src)": Markdown của Slint không hiện ảnh nên hộp thoại hiện cả dòng thành chữ
    thô, còn GitHub hiện ảnh. Loại "wrapped" là dấu '[' của link nội dòng "[text](url)" kéo sang dòng
    sau: hộp thoại đọc từng dòng riêng nên hiện cú pháp thô không bấm được, còn GitHub hiện link.

    Đọc từ trái sang như cmark-gfm: ký tự thoát, code span, autolink, URL trần, phần "(đích "tiêu đề")"
    của link và nhãn "[...]" của link tham chiếu không chứa HTML hay link tham chiếu, còn '<' không mở
    được thẻ nào như trong "a < b" hay "x<limit" là chữ. Như tiện ích footnotes của GitHub, "[^...]"
    không thành link là chỗ gọi footnote, và mọi thứ bên trong nó chỉ còn là chữ của nhãn.
    """
    found = []
    newlines = [match.start() for match in re.finditer("\n", text)]
    # [là ảnh, vị trí dấu mở, vị trí đầu chữ trong ngoặc, đã có '[' mở sau nó, số mục đã thấy lúc mở]
    # của từng '[' hay '![' chưa đóng, trong cùng ở cuối.
    brackets = []
    # Như no_link_openers của cmark-gfm: sau khi một link khớp, mọi '[' còn mở không thành link được
    # nữa, cho tới khi một '[' mới mở ra thì tất cả lại mở được link, như GitHub hiện
    # "[x [a](/u) [b] ](/v)" thành link lồng link. '![' không đổi cờ này.
    no_link_openers = False
    # Vị trí chuỗi backtick cuối cùng theo độ dài mà các lần tìm dấu đóng code span đã gặp. Như
    # scan_to_closing_backticks của cmark-gfm, sau một lần tìm tới hết mà không thấy dấu đóng, chuỗi
    # mở nằm sau vị trí đã ghi của độ dài đó là chữ thường dù phía sau vẫn có dấu đóng; GitHub cũng
    # hiện y như vậy.
    last_runs = {}
    scanned_to_end = False
    # Như cmark-gfm, URL trần chỉ dài tới cuối nội dung sau khi cắt khoảng trắng.
    limit = len(text.rstrip(CMARK_SPACE))
    # "www." mở trước vị trí này chắc chắn không thành link, xem bare_url_end.
    www_rejected_before = -1
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
                found.append((index, "html"))
            index = index + 1 if link_or_html is None else link_or_html.end()
        elif char == "[" or (text.startswith("![", index) and not text.startswith("![^", index)):
            # Như tiện ích footnotes của cmark-gfm, "![^" không mở ảnh: '!' là chữ, còn "[^" mở như
            # một '[' thường.
            if brackets:
                brackets[-1][3] = True
            if char == "[":
                no_link_openers = False
            opener = index
            index += 1 if char == "[" else 2
            brackets.append([char == "!", opener, index, False, len(found)])
        elif char == "]" and brackets:
            image, opener, content_start, bracket_after, seen = brackets.pop()
            active = image or not no_link_openers
            end = link_end(text, index, content_start, bracket_after, definitions) if active else None
            if end is not None:
                # Link nội dòng kết thúc bằng ')', còn link tham chiếu kết thúc bằng ']' của nhãn.
                if text[end - 1] == "]":
                    found.append((opener, "reference"))
                elif image:
                    found.append((opener, "image"))
                elif bisect_left(newlines, opener) < bisect_left(newlines, end - 1):
                    found.append((opener, "wrapped"))
                index = end
                if not image:
                    no_link_openers = True
            else:
                if active and text.startswith("^", content_start) and index > content_start + 1:
                    # Chỗ gọi footnote thay mọi thứ trong ngoặc bằng chữ của nhãn. GitHub chỉ giữ nó
                    # là chỗ gọi khi có định nghĩa cùng nhãn, ngược lại hiện lại đúng chữ gốc.
                    del found[seen:]
                    if bisect_left(newlines, content_start) < bisect_left(newlines, index):
                        found.append((opener, "split"))
                    elif footnotes and label_key(text[content_start + 1:index]) in footnotes:
                        found.append((opener, "footnote"))
                index += 1
        elif char in ":w" and not brackets:
            # Tiện ích autolink không chạy khi còn '[' hay '![' mở, kể cả '[' không còn mở link được.
            url_end, www_rejected_before = bare_url_end(text, index, position, limit, www_rejected_before)
            if url_end is None:
                index += 1
            else:
                # Ghi ở đầu tên giao thức hay "www." để dòng báo lỗi là dòng có URL.
                found.append((index if text[index] == "w" else url_start(text, index, position), "url"))
                index = url_end
        else:
            index += 1
    return found


def inline_lines(units, footnotes=frozenset()):
    """Chỉ số các dòng có thứ hộp thoại hiện khác GitHub, trong các nội dung nội dòng mà scan gom được.

    Trả về dict từ loại tới tập chỉ số dòng: "html", "reference", "footnote", "split", "url", "image"
    và "wrapped" như inline_offsets (footnotes là khoá nhãn của mọi định nghĩa footnote), cùng
    "definition" cho các dòng của định nghĩa link tham chiếu mà GitHub bỏ khỏi trang. Như cmark-gfm,
    mọi định nghĩa trong notes được gom trước, nên link dùng được định nghĩa nằm ở bất kỳ đâu, kể cả
    phía sau nó.
    """
    labels = []
    contents = []
    for unit_kind, entries in units:
        # Mỗi dòng kết thúc bằng xuống dòng như nội dung đoạn văn của cmark-gfm.
        text = "".join(content + "\n" for _, content in entries)
        if unit_kind == "cell":
            # cmark-gfm đổi "\|" trong ô thành "|" trước khi đọc nội dòng; ô chỉ có một dòng.
            text = text.replace("\\|", "|")
        start = reference_definitions_end(text, labels) if unit_kind == "paragraph" else 0
        contents.append((text, start, entries))
    definitions = frozenset(labels)
    found = {"html": set(), "reference": set(), "footnote": set(), "split": set(), "url": set(),
             "image": set(), "wrapped": set(), "definition": set()}

    def line_of(offset, entries):
        """Chỉ số dòng chứa vị trí offset trong nội dung ghép từ entries."""
        for line_index, content in entries:
            offset -= len(content) + 1
            if offset < 0:
                return line_index
        return entries[-1][0]

    for text, start, entries in contents:
        # Các dòng nằm trước start chỉ chứa định nghĩa.
        consumed = 0
        for line_index, content in entries:
            if consumed >= start:
                break
            found["definition"].add(line_index)
            consumed += len(content) + 1
        for offset, kind in inline_offsets(text, start, definitions, footnotes):
            found[kind].add(line_of(offset, entries))
    return found


def heading_text(body):
    """Chữ của tiêu đề ATX body (đã bỏ thụt lề) mà cmark-gfm đọc nội dòng.

    Như chop_trailing_hashtags của cmark-gfm, chuỗi '#' đóng chỉ bị bỏ khi đứng sau dấu cách, tab hay
    ngay sau chuỗi '#' mở, nên "### a \\###" và "# a#" giữ nguyên dấu '#'. Không bỏ nó thì "www." ở cuối
    "### Remove the www. ###" không còn là ký tự cuối và bị đọc nhầm thành URL trần.
    """
    text = body.lstrip("#").rstrip(" \t")
    stripped = text.rstrip("#")
    if len(stripped) < len(text) and (not stripped or stripped[-1] in " \t"):
        text = stripped.rstrip(" \t")
    return text


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
    dòng giữ chỗ của bản nháp cũng bị từ chối. Hàng thân bảng có ô vượt quá số ô của hàng tiêu đề
    vào kết quả ở dạng đã cắt bỏ các ô đó, như GitHub bỏ chúng khi hiện trang release.
    """
    lines = text.replace("\r\n", "\n").split("\n")
    kinds, unclosed, inline, quotes, trimmed = scan(lines)
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
        # GitHub bỏ định nghĩa footnote không ai gọi và đưa định nghĩa còn lại xuống cuối trang, còn
        # hộp thoại hiện nó như chữ ngay tại chỗ, kể cả comment HTML bên trong mà GitHub giấu đi. Chỗ
        # gọi một định nghĩa thành số chú thích trên GitHub, còn hộp thoại hiện nguyên chữ "[^note]".
        if kinds[index] == "footnote" or index in inline["footnote"]:
            raise ValueError(f"line {index + 1}: the Release Notes dialog shows footnotes as plain "
                             "text, so put this note in the text instead")
        if index in inline["split"]:
            raise ValueError(f"line {index + 1}: GitHub changes the text of a [^...] that spans lines, "
                             "so keep it on one line")
        if kinds[index] == "html" or index in inline["html"]:
            raise ValueError(f"line {index + 1}: the Release Notes dialog shows HTML as plain text, "
                             "so remove the HTML or put it in backticks")
        # Hộp thoại đọc từng dòng riêng nên không nối được link tham chiếu với định nghĩa của nó.
        if index in inline["definition"]:
            raise ValueError(f"line {index + 1}: the Release Notes dialog cannot use link reference "
                             "definitions, so write links inline as [text](url) and remove this line")
        if index in inline["reference"]:
            raise ValueError(f"line {index + 1}: the Release Notes dialog cannot resolve reference "
                             "links, so write this link inline as [text](url)")
        # GitHub tự biến URL trần thành link, còn Markdown của Slint thì không, nên hộp thoại hiện
        # nó thành chữ không bấm được. "<https://...>" hiện giống hệt trên GitHub và bấm được.
        if index in inline["url"]:
            raise ValueError(f"line {index + 1}: the Release Notes dialog shows bare URLs as plain "
                             "text, so write this link as <https://...> or [text](https://...), or put an "
                             "address that has no https form in backticks")
        # Markdown của Slint báo lỗi với ảnh, nên hộp thoại hiện cả dòng thành chữ thô.
        if index in inline["image"]:
            raise ValueError(f"line {index + 1}: the Release Notes dialog cannot show images, so remove "
                             "this image or link to it as [text](https://...)")
        # Hộp thoại đọc từng dòng riêng, nên link kéo sang dòng sau hiện thành cú pháp thô.
        if index in inline["wrapped"]:
            raise ValueError(f"line {index + 1}: the Release Notes dialog reads each line on its own and "
                             "cannot show a link that wraps to the next line, so keep this link on one line")
        # Markdown của Slint báo lỗi với trích dẫn, nên hộp thoại hiện cả dòng thành chữ thô với dấu
        # ">", kể cả link trong đó mà người đọc không bấm được, còn GitHub hiện khối trích dẫn. Mọi
        # dòng khác của trích dẫn đứng sau dòng mở nó, nên chỉ cần chặn dòng mở.
        if index in quotes:
            raise ValueError(f"line {index + 1}: the Release Notes dialog cannot show block quotes, so "
                             "remove the '>' and write this as plain text or a list item")
    if publish and any(lines[index].strip() == PLACEHOLDER for index in section_indexes):
        raise ValueError(f"the '## Changes' section still has the '{PLACEHOLDER}' placeholder")
    # Hàng thân bảng có ô thừa vào notes nhúng ở dạng đã cắt, chỉ còn các ô GitHub hiện.
    section = normalize("\n".join(trimmed.get(index, lines[index]) for index in range(start + 1, end)))
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
