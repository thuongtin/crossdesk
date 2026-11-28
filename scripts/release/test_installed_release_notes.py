"""Kiểm thử installed_release_notes.py: ranh giới mục Changes, code fence, HTML và kiểm tra --publish.

Chạy: python3 -B -m unittest discover -s scripts/release -p test_installed_release_notes.py
"""

import contextlib
import importlib.util
import io
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock


SCRIPT = Path(__file__).resolve().with_name("installed_release_notes.py")
spec = importlib.util.spec_from_file_location("installed_release_notes", SCRIPT)
notes = importlib.util.module_from_spec(spec)
spec.loader.exec_module(notes)

FENCE = "```"
NESTED = "line {}: start code fences at column 0, not indented or inside a list item or quote"
HTML = "line {}: the Release Notes dialog shows HTML as plain text, so remove the HTML or put it in backticks"
SETEXT = "line {}: a line of '=' or '-' under text makes it a level 1 or 2 heading, so put a blank line before this line"
FOOTNOTE = "line {}: the Release Notes dialog shows footnotes as plain text, so put this note in the text instead"
# Ví dụ notes nằm trong một khối trước mục thật; mục giả không được thay mục thật.
FAKE_SECTION = "## Changes\n- Example only\n## Downloads"


def run_main(text, publish):
    """Chạy main() trên notes tạm; trả về (mã thoát, byte đầu ra hoặc None, stderr)."""
    with tempfile.TemporaryDirectory() as directory:
        source = Path(directory, "RELEASE_NOTES.md")
        output = Path(directory, "INSTALLED_RELEASE_NOTES.md")
        source.write_bytes(text.encode("utf-8"))
        argv = ["installed_release_notes.py", str(source), str(output)] + (["--publish"] if publish else [])
        errors = io.StringIO()
        with mock.patch.object(sys, "argv", argv), contextlib.redirect_stderr(errors):
            code = notes.main()
        return code, output.read_bytes() if output.exists() else None, errors.getvalue()


class ExtractTest(unittest.TestCase):
    def assert_rejected(self, text, message):
        with self.assertRaises(ValueError) as context:
            notes.extract(text)
        self.assertEqual(str(context.exception), message)

    def test_section_runs_to_next_level_one_or_two_heading(self):
        text = "# v1\n\n## Changes ##\n\n### Linux\n\n- A\n\n## Downloads\n\n- x"
        self.assertEqual(notes.extract(text), "### Linux\n\n- A")
        self.assertEqual(notes.extract("## Changes\n- A\n# Next"), "- A")
        self.assertEqual(notes.extract("## Changes\r\n- A\r\n- B\r\n"), "- A\n- B")

    def test_top_level_fence_keeps_heading_like_lines(self):
        text = f"## Changes\n\n{FENCE}sh\n## not a heading\n{FENCE}\n\n- A\n\n## Next"
        self.assertEqual(notes.extract(text), f"{FENCE}sh\n## not a heading\n{FENCE}\n\n- A")
        text = "## Downloads\n\n~~~\n## Changes\n~~~\n\n## Changes\n\n- A"
        self.assertEqual(notes.extract(text), "- A")

    def test_closing_fence_rules(self):
        # Dòng đóng thụt tối đa ba dấu cách, cùng ký tự, dài ít nhất bằng dòng mở, không có info.
        cases = [
            (f"{FENCE}\nx\n   {FENCE}\n## Next", f"{FENCE}\nx\n   {FENCE}"),
            (f"{FENCE}\n    {FENCE}\n## code\n{FENCE}\n## Next", f"{FENCE}\n    {FENCE}\n## code\n{FENCE}"),
            (f"{FENCE}\n\t{FENCE}\n## code\n{FENCE}\n## Next", f"{FENCE}\n\t{FENCE}\n## code\n{FENCE}"),
            (f"{FENCE}\n{FENCE} x\n## code\n{FENCE}\n## Next", f"{FENCE}\n{FENCE} x\n## code\n{FENCE}"),
            (f"`{FENCE}\n{FENCE}\n## code\n`{FENCE}\n## Next", f"`{FENCE}\n{FENCE}\n## code\n`{FENCE}"),
            (f"~~~\n{FENCE}\n## code\n~~~\n## Next", f"~~~\n{FENCE}\n## code\n~~~"),
            (f"{FENCE}md\n  {FENCE}\n- A\n## Next", f"{FENCE}md\n  {FENCE}\n- A"),
        ]
        for body, expected in cases:
            with self.subTest(body=body):
                self.assertEqual(notes.extract("## Changes\n\n" + body), expected)

    def test_backtick_info_with_backtick_is_not_a_fence(self):
        self.assertEqual(notes.extract(f"## Changes\n\n{FENCE}a`b\n## Next"), f"{FENCE}a`b")

    def test_literal_and_nested_fences_before_changes_are_allowed(self):
        # Trước mục Changes chỉ tiêu đề ở cột 0 quyết định ranh giới, nên khối code thụt lề hay fence
        # trong mục list ở Downloads không chặn bản build.
        text = (f"## Downloads\n\nMarkdown example:\n\n    {FENCE}sh\n\n- {FENCE}sh\n  crossdesk --version\n"
                f"  {FENCE}\n- Run:\n  {FENCE}sh\n  ## Changes\n  {FENCE}\n\n## Changes\n\n- A")
        self.assertEqual(notes.extract(text), "- A")

    def test_indented_top_level_fence_hides_headings(self):
        # Fence thụt một tới ba dấu cách ngoài mục list vẫn ở cấp cao nhất, nên giấu tiêu đề ở cột 0.
        for indent in (" ", "  ", "   "):
            with self.subTest(indent=indent):
                text = (f"## Examples\n\n{indent}{FENCE}markdown\n{FAKE_SECTION}\n{indent}{FENCE}\n\n"
                        "## Changes\n\n- Actual release fix")
                self.assertEqual(notes.extract(text), "- Actual release fix")

    def test_list_item_fences_do_not_open_top_level_fences(self):
        # Dòng đóng "  ```" của fence trong mục list không được đọc thành dòng mở fence cấp cao nhất.
        text = f"## Downloads\n\n- {FENCE}sh\n  x\n  {FENCE}\n\n## Changes\n\n- A"
        self.assertEqual(notes.extract(text), "- A")
        text = f"## Downloads\n\n1. Run:\n\n   {FENCE}sh\n   x\n   {FENCE}\n\n## Changes\n\n- A"
        self.assertEqual(notes.extract(text), "- A")
        text = f"## Downloads\n\n> {FENCE}\n> x\n> {FENCE}\n\n## Changes\n\n- A"
        self.assertEqual(notes.extract(text), "- A")

    def test_lines_that_leave_a_list_item_start_top_level_fences(self):
        cases = [
            # Sau fence của mục list lồng, dòng ở cột 0 không nối lười mà kết thúc mọi list.
            f"- a\n  - b\n    - c\n      {FENCE}sh\n      x\n      {FENCE}\nfoo\n  {FENCE}\n{FAKE_SECTION}\n  {FENCE}",
            # Mục list chỉ có dấu list kết thúc ở dòng trống đầu tiên.
            f"-\n\n  {FENCE}\n{FAKE_SECTION}\n  {FENCE}",
            # Fence ngắt đoạn văn, nên dòng mở thụt hai dấu cách sau trích dẫn không thuộc trích dẫn.
            f"> quote\n  {FENCE}\n{FAKE_SECTION}\n  {FENCE}",
        ]
        for body in cases:
            with self.subTest(body=body):
                text = f"## Downloads\n\n{body}\n\n## Changes\n\n- A"
                self.assertEqual(notes.extract(text), "- A")
        # Dòng nối lười giữ mục list mở, nên fence thụt hai dấu cách sau nó vẫn thuộc mục.
        text = f"## Downloads\n\n- a\nlazy\n  {FENCE}\n  x\n  {FENCE}\n\n## Changes\n\n- A"
        self.assertEqual(notes.extract(text), "- A")

    def test_unclosed_blocks_before_or_in_changes_are_rejected(self):
        self.assert_rejected(f"## Downloads\n\n  {FENCE}sh\nx\n\n## Changes\n\n- A",
                             "line 3: close the code fence that starts here")
        self.assert_rejected(f"## Changes\n\n- A\n\n{FENCE}sh\nx\n\n## Downloads",
                             "line 5: close the code fence that starts here")
        self.assert_rejected("<!-- draft\n\n## Changes\n\n- A",
                             "line 1: close the HTML block that starts here")
        self.assertEqual(notes.extract(f"## Changes\n\n- A\n\n## Downloads\n\n{FENCE}sh\nx"), "- A")

    def test_html_blocks_hide_headings(self):
        cases = [
            f"<!--\n{FAKE_SECTION}\n-->",
            f"<div>\n{FAKE_SECTION}\n</div>",
            f"  <pre>\n\n{FAKE_SECTION}\n</pre>",
            # Một thẻ đứng riêng sau dòng trống mở khối HTML loại 7.
            f"Intro\n\n<span>\n{FAKE_SECTION}",
        ]
        for body in cases:
            with self.subTest(body=body):
                text = f"## Examples\n\n{body}\n\n## Changes\n\n- A"
                self.assertEqual(notes.extract(text), "- A")
        # Thẻ đứng riêng ngay sau đoạn văn là chữ của đoạn văn; khối HTML loại 6 kết thúc ở dòng trống.
        self.assertEqual(notes.extract("Intro\n<span>\n## Changes\n\n- A"), "- A")
        self.assertEqual(notes.extract("<details>\n<summary>Checksums</summary>\n\n## Changes\n\n- A"), "- A")

    def test_only_github_html_block_starters_hide_headings(self):
        # GitHub không coi <search> là thẻ khối loại 6, và khai báo loại 4 cần chữ hoa sau "<!".
        for before in ("Compare the packages below.\n<search>Find a package</search>", "<!doctype html"):
            with self.subTest(before=before):
                self.assertEqual(notes.extract(f"## Downloads\n\n{before}\n## Changes\n\n- A"), "- A")
        self.assertEqual(notes.extract(f"<!DOCTYPE x\n{FAKE_SECTION}\n>\n\n## Changes\n\n- A"), "- A")
        self.assert_rejected("<!DOCTYPE html\n\n## Changes\n\n- A",
                             "line 1: close the HTML block that starts here")

    def test_html_in_changes_is_rejected(self):
        cases = [
            ("- A\n<!-- note -->", 4),
            ("<details>\n<summary>More</summary>\n\n- A\n\n</details>", 3),
            ("- A\n  - <br>", 4),
            ("> <img src=\"x.png\">", 3),
            # Comment nội dòng bị GitHub giấu nhưng hộp thoại hiện nguyên chữ.
            ("- A <!-- internal note -->", 3),
            ("- Press <kbd>Ctrl</kbd> + C", 3),
            ("- Fixed `x` for <package>", 3),
            ("- Link <a\n  href=\"x\">", 3),
            ("- Note <!X y>", 3),
            ("| a |\n| - |\n| <b>x</b> |", 5),
            # Thẻ đứng riêng ngay sau bảng mở khối HTML loại 7.
            ("| a |\n| - |\n<span>", 5),
            # Định nghĩa link trong đoạn văn đứng trước bảng không được GitHub đọc, nên <b c> là HTML.
            ("[a]: <b c>\n| x |\n| - |", 3),
            # Như GitHub, chuỗi ``` không có dấu đóng làm backtick đơn sau đó thành chữ thường dù có cặp.
            ("Use ``` `x`\n`<b>`", 4),
        ]
        for body, line in cases:
            with self.subTest(body=body):
                self.assert_rejected("## Changes\n\n" + body, HTML.format(line))
        allowed = ("- See <https://example.com> and <team@example.com>\n- a < b, c<5, x<limit, <a/ >\n"
                   "- Renamed `<package>` and ``a ` <b>`` in \\<div>\n"
                   "- See [the guide](<docs/Upgrade Guide.md>)\n\n[guide]: <docs/a b.md>")
        self.assertEqual(notes.extract("## Changes\n\n" + allowed), allowed)

    def test_long_escaped_links_finish_quickly(self):
        # Mỗi "\x" vừa là ký tự thoát vừa là hai ký tự thường; regex dò ngược sẽ treo bản build.
        for body in ("- [" + "pkg\\_" * 40 + "](x)", "- [" + "\\!" * 40 + " x", '- [a](b "' + '\\"' * 40 + " x"):
            with self.subTest(body=body[:12]):
                self.assertEqual(notes.extract("## Changes\n\n" + body), body)

    def test_link_label_length_limit(self):
        # Như cmark-gfm, nhãn định nghĩa link dài tối đa 1000 byte UTF-8, ký tự thoát tính hai byte
        # và tab tính một byte dù nó lấp tới cột bội số của 4. Quá giới hạn thì dòng là chữ thường và
        # <x y> trong đó là HTML.
        for label in ("a" * 1000, "\\!" * 500, "a\t" * 333):
            for prefix in ("", "> "):
                with self.subTest(label=label[:4], prefix=prefix):
                    body = f"{prefix}[{label}]: <x y>"
                    self.assertEqual(notes.extract("## Changes\n\n" + body), body)
        for label in ("a" * 1001, "\\!" * 500 + "a", "\u00e9" * 500 + "a", "a\t" * 500 + "a"):
            with self.subTest(label=label[:4]):
                self.assert_rejected(f"## Changes\n\n[{label}]: <x y>", HTML.format(3))

    def test_setext_heading_in_changes_is_rejected(self):
        for underline in ("---", "==="):
            with self.subTest(underline=underline):
                self.assert_rejected(f"## Changes\n\n- A\n\nDownloads\n{underline}\n- x", SETEXT.format(6))
        # Như cmark-gfm, dòng gạch dưới trong mục list hay trích dẫn cũng biến dòng trên thành tiêu đề.
        for body in ("- Linux\n  ---", "> a\n> ---", "- > a\n  > ==="):
            with self.subTest(body=body):
                self.assert_rejected("## Changes\n\n" + body, SETEXT.format(4))
        # Đường kẻ ngang sau dòng trống hay sau mục list không biến dòng trên thành tiêu đề.
        self.assertEqual(notes.extract("## Changes\n\nText\n\n---\n- A\n---"), "Text\n\n---\n- A\n---")

    def test_tables_end_at_a_dashed_line(self):
        table = "| OS | Fix |\n| --- | --- |\n| Linux | Reconnect |\n---"
        self.assertEqual(notes.extract("## Changes\n\n" + table), table)
        # Đoạn chỉ có định nghĩa link không thành tiêu đề; GitHub hiện "---" sau nó thành chữ thường.
        self.assertEqual(notes.extract("## Changes\n\n[a]: https://example.com\n---"), "[a]: https://example.com\n---")
        # cmark-gfm đọc định nghĩa link ngay khi gặp dòng giống gạch dưới, nên <x y> ở đây không phải
        # HTML dù dòng "-" sau đó thành tiêu đề cột của bảng.
        self.assertEqual(notes.extract("## Changes\n\n[a]: <x y>\n-\n|---|"), "[a]: <x y>\n-\n|---|")
        # Dòng phân cách lệch số cột khiến đoạn văn không bao giờ thành bảng, kể cả khi sau đó có cặp khớp.
        self.assert_rejected("## Changes\n\na | b\n-|-|-\nc | d\n-|-\n---", SETEXT.format(7))

    def test_escaped_pipes_stay_in_their_table_cell(self):
        # Như cmark-gfm, "\|" là chữ '|' của ô bảng, kể cả trong code span và khi '\' đứng sau một
        # '\' khác, nên <drive> ở đây nằm trong code chứ không phải HTML.
        for cell in ("`D:\\|<drive>`", "`D:\\\\|<drive>`"):
            table = f"| Path | Note |\n| --- | --- |\n| {cell} | x |"
            with self.subTest(cell=cell):
                self.assertEqual(notes.extract("## Changes\n\n" + table), table)
        self.assert_rejected("## Changes\n\n| a |\n| - |\n| b \\| <i> |", HTML.format(5))

    def test_reference_link_labels_are_not_html(self):
        # Nhãn của link tham chiếu có định nghĩa ở bất kỳ đâu trong notes không phải HTML; không có
        # định nghĩa thì GitHub hiện chữ trong ngoặc và giấu <profile> như một thẻ HTML.
        body = "- See the [profile option][<profile>]"
        defined = body + "\n\n[<profile>]: ./manual#profiles"
        self.assertEqual(notes.extract("## Changes\n\n" + defined), defined)
        self.assert_rejected("## Changes\n\n" + body, HTML.format(3))
        # Định nghĩa bị bỏ ra khỏi đoạn văn khi đoạn đó thành tiêu đề setext vẫn dùng được ở chỗ khác.
        self.assertEqual(notes.extract("[<p>]: /u\nTitle\n---\n\n## Changes\n\n- See [x][<p>]"),
                         "- See [x][<p>]")
        # Chữ trong ngoặc của link vẫn là nội dung nội dòng, nên <profile> ở đó là HTML.
        self.assert_rejected("## Changes\n\n- Fixed [<profile>][]\n\n[<profile>]: ./manual", HTML.format(3))

    def test_link_openers_follow_cmark_gfm(self):
        # Sau một link khớp, '[' bao quanh nó không thành link được, nên "(<x y>)" là chữ chứa HTML.
        self.assert_rejected("## Changes\n\n- [a [b][r] c](<x y>)\n\n[r]: /u", HTML.format(3))
        # Như cmark-gfm, một '[' mới mở lại mọi '[' cũ, và GitHub hiện ở đây link lồng link, không có HTML.
        allowed = "- [x [a](/u) [b] ](<x y>)"
        self.assertEqual(notes.extract("## Changes\n\n" + allowed), allowed)
        # "![^" là '!' cộng một '[' thường, nên link bên trong chặn nó thành chỗ gọi footnote; '![' sau
        # link không mở lại '[' cũ.
        for body in ("- ![^x [a](/u) <i>]", "- [^x <b> [a](/u) ![b] ]"):
            with self.subTest(body=body):
                self.assert_rejected("## Changes\n\n" + body, HTML.format(3))

    def test_footnote_calls_show_html_as_text(self):
        # Chỗ gọi footnote không có định nghĩa hiện nguyên chữ trên GitHub, kể cả HTML bên trong.
        body = "- x [^q<b>] and ![^y <i>]"
        self.assertEqual(notes.extract("## Changes\n\n" + body), body)

    def test_footnote_definitions_in_changes_are_rejected(self):
        # GitHub bỏ định nghĩa không ai gọi và đưa định nghĩa còn lại xuống cuối trang, kể cả khi nó
        # nằm trong mục list hay trích dẫn, và giấu comment HTML trong đó.
        cases = [
            ("- A[^compat]\n\n[^compat]: <!-- verify on 20.04 -->", 5),
            ("- A[^compat]\n\n[^compat]: Needs Ubuntu 20.04", 5),
            ("- Quoted\n\n> [^compat]: Needs Ubuntu 20.04", 5),
            ("- Item\n\n  [^compat]: Needs Ubuntu 20.04", 5),
            ("[^n]:\n\n    <i>b</i>", 3),
        ]
        for body, line in cases:
            with self.subTest(body=body):
                self.assert_rejected("## Changes\n\n" + body, FOOTNOTE.format(line))
        # Nhãn có dấu cách là định nghĩa link thường, không phải footnote.
        allowed = "- See [^a b]\n\n[^a b]: ./manual"
        self.assertEqual(notes.extract("## Changes\n\n" + allowed), allowed)
        # Footnote trước mục chỉ đi tiếp qua dòng trống hẳn. Như cmark-gfm, dòng chỉ có dấu cách đóng
        # nó, nên định nghĩa thụt bốn cột phía sau là code và <p> trong mục là HTML.
        template = "[^n]: a\n{}\n    [<p>]: /u\n\n## Changes\n\n- See [x][<p>]"
        self.assertEqual(notes.extract(template.format("")), "- See [x][<p>]")
        self.assert_rejected(template.format("  "), HTML.format(7))

    def test_fences_after_changes_are_ignored(self):
        text = f"## Changes\n\n- A\n\n## Downloads\n\n- {FENCE}sh\n  x\n  {FENCE}"
        self.assertEqual(notes.extract(text), "- A")

    def test_nested_fences_in_changes_are_rejected(self):
        cases = [
            (f"- Example:\n\n  ~~~sh\n  x\n## Downloads", 5),
            (f"- {FENCE}sh\n  ## comment\n  {FENCE}\n## Next", 3),
            (f"-\t{FENCE}\n      {FENCE}\n  ### Installation", 3),
            (f"  - {FENCE}text\n    example\n      {FENCE}\n  - TODO", 3),
            (f"> {FENCE}\n> x\n> {FENCE}", 3),
            ("1. ~~~\n   x\n   ~~~", 3),
            # Khối code thụt lề chứa dòng giống fence cũng bị từ chối, xem nested_fence.
            (f"Markdown example:\n\n    {FENCE}sh", 5),
            # Fence cấp cao nhất thụt lề hiện đúng trên GitHub nhưng InstalledBlocks chỉ hiểu cột 0.
            (f"- A\n\n  {FENCE}sh\n  x\n  {FENCE}", 5),
        ]
        for body, line in cases:
            with self.subTest(body=body):
                self.assert_rejected("## Changes\n\n" + body, NESTED.format(line))
        self.assert_rejected(f"## Changes\n  - {FENCE}text\n    x\n    {FENCE}", NESTED.format(2))

    def test_indented_level_one_or_two_heading_in_changes_is_rejected(self):
        self.assert_rejected("## Changes\n\n- A\n  ## Downloads\n- x",
                             "line 4: start level 1 and 2 headings at column 0")
        self.assertEqual(notes.extract("## Changes\n\n  ### Linux\n- A"), "### Linux\n- A")

    def test_missing_or_empty_section(self):
        self.assert_rejected("## Other\n- x", "release notes have no '## Changes' heading at column 0")
        self.assert_rejected("  ## Changes\n- x", "release notes have no '## Changes' heading at column 0")
        self.assert_rejected("## Changes\n\n \n## Next", "the '## Changes' section of the release notes is empty")


class PublishTest(unittest.TestCase):
    def test_placeholder_is_rejected_only_when_publishing(self):
        for text in ["## Changes\n\n- TODO\n", "## Changes\n\n- A\n  - TODO \n", "## Changes\n  - TODO\n",
                     f"## Changes\n\n{FENCE}\nx\n  {FENCE}\n- TODO\n"]:
            with self.subTest(text=text):
                code, output, errors = run_main(text, publish=True)
                self.assertEqual((code, output), (1, None))
                self.assertIn("still has the '- TODO' placeholder", errors)
                self.assertEqual(run_main(text, publish=False)[0], 0)

    def test_placeholder_inside_column_zero_fence_is_allowed(self):
        text = f"## Changes\n\n- A\n\n{FENCE}md\n- TODO\n{FENCE}\n"
        self.assertEqual(run_main(text, publish=True)[:2], (0, f"- A\n\n{FENCE}md\n- TODO\n{FENCE}\n".encode()))

    def test_nested_fence_fails_in_every_mode(self):
        text = f"## Changes\n\n- {FENCE}md\n  x\n  {FENCE}\n"
        for publish in (False, True):
            with self.subTest(publish=publish):
                code, output, errors = run_main(text, publish)
                self.assertEqual((code, output), (1, None))
                self.assertIn(NESTED.format(3), errors)

    def test_example_section_in_indented_fence_is_not_published(self):
        text = (f"## Examples\n\n  {FENCE}markdown\n{FAKE_SECTION}\n  {FENCE}\n\n"
                "## Changes\n\n- Actual release fix\n")
        self.assertEqual(run_main(text, publish=True)[:2], (0, b"- Actual release fix\n"))

    def test_output_is_section_with_lf(self):
        code, output, _ = run_main("﻿## Changes\r\n\r\n- A\r\n- B\r\n\r\n## Next\r\n", publish=True)
        self.assertEqual((code, output), (0, b"- A\n- B\n"))


if __name__ == "__main__":
    unittest.main()
