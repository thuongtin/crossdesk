"""Kiểm thử installed_release_notes.py: ranh giới mục Changes, code fence và kiểm tra --publish.

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

    def test_output_is_section_with_lf(self):
        code, output, _ = run_main("﻿## Changes\r\n\r\n- A\r\n- B\r\n\r\n## Next\r\n", publish=True)
        self.assertEqual((code, output), (0, b"- A\n- B\n"))


if __name__ == "__main__":
    unittest.main()
