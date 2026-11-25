"""Execute the production policy selector independently of SwiftUI on macOS."""

from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


@unittest.skipUnless(sys.platform == 'darwin', 'requires the Apple Swift runtime')
class PrivacyPolicyTest(unittest.TestCase):
    def test_policy_sections(self):
        source = Path(__file__).resolve().parents[1] / 'CrossDeskMobile/PrivacyViews.swift'
        selector = source.read_text(encoding='utf-8').split('enum PrivacyPolicyText {', 1)[1].split('\nprivate struct PrivacyPolicyDocument:', 1)[0]
        with tempfile.TemporaryDirectory(prefix='crossdesk-policy-tests-') as temporary:
            harness = Path(temporary) / 'main.swift'
            harness.write_text('import Foundation\nenum PrivacyPolicyText {' + selector + r'''
let policy = "## Tiếng Việt\nVietnamese\n## English\nEnglish"
for language in ["vi", "vi-VN", "VI_vn.UTF-8", "", "id-ID"] {
    precondition(PrivacyPolicyText.select(policy, language: language) == "Vietnamese")
}
precondition(PrivacyPolicyText.select(policy) == "Vietnamese")
for language in ["en", "zh-CN", "zh-Hant", "ru", "ja", "ko", "de", "fr", "es", "pt-BR"] {
    precondition(PrivacyPolicyText.select(policy, language: language) == "English")
}
precondition(PrivacyPolicyText.select("## 中文\r\nLegacy\r\n## English\r\nEnglish") == "Legacy")
precondition(PrivacyPolicyText.select("## 中文\nLegacy\n## English\nEnglish", language: "zh") == "English")
for language in ["vi", "en"] {
    precondition(PrivacyPolicyText.select("## English\nEnglish", language: language) == "English")
    precondition(PrivacyPolicyText.select("## Tiếng Việt\nVietnamese", language: language) == "Vietnamese")
    precondition(PrivacyPolicyText.select("## Tiếng Việt\n## English\nEnglish", language: language) == "English")
    precondition(PrivacyPolicyText.select("## English\n## Tiếng Việt\nVietnamese", language: language) == "Vietnamese")
    precondition(PrivacyPolicyText.select("Plain text", language: language) == "Plain text")
    precondition(PrivacyPolicyText.select(" \r\n ", language: language).isEmpty)
}
print("PASS: Vietnamese/default policy, English locales, legacy, independent/empty sections and plain text")
''', encoding='utf-8')
            result = subprocess.run(['xcrun', 'swift', str(harness)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)


if __name__ == '__main__':
    unittest.main()
