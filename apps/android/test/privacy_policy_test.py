"""Run the production policy text methods without an Android SDK."""

from pathlib import Path
import json
import shutil
import subprocess
import tempfile
import unittest


@unittest.skipUnless(shutil.which('javac') and shutil.which('java'), 'requires a JDK')
class PrivacyPolicyTest(unittest.TestCase):
    def test_policy_sections(self):
        root = Path(__file__).resolve().parents[1]
        source = root / 'app/src/main/java/cn/crossdesk/mobile/PrivacyPolicyDocument.java'
        methods = source.read_text(encoding='utf-8').split('    static String selectText', 1)[1].split('    private static String load', 1)[0]
        policy = '## Tiếng Việt\nVietnamese\n## English\nEnglish'
        cases = [(policy, locale, 'Vietnamese') for locale in ['vi', 'vi-VN', 'VI_vn.UTF-8', '', 'id-ID', None]]
        cases += [(policy, locale, 'English') for locale in ['en', 'zh-CN', 'zh-Hant', 'ru', 'ja', 'ko', 'de', 'fr', 'es', 'pt-BR']]
        cases += [('## 中文\r\nLegacy\r\n## English\r\nEnglish', 'vi', 'Legacy'),
                  ('## 中文\nLegacy\n## English\nEnglish', 'zh', 'English'), (None, None, '')]
        for locale in ['vi', 'en']:
            for document, expected in [('## English\nEnglish', 'English'),
                                       ('## Tiếng Việt\nVietnamese', 'Vietnamese'),
                                       ('## Tiếng Việt\n## English\nEnglish', 'English'),
                                       ('## English\n## Tiếng Việt\nVietnamese', 'Vietnamese'),
                                       ('Plain text', 'Plain text'), (' \r\n ', '')]:
                cases.append((document, locale, expected))
        assertions = '\n'.join('if (!Objects.equals(selectText(%s, %s), %s)) throw new AssertionError("case %d");' %
                               (*[json.dumps(value, ensure_ascii=False) for value in case], index)
                               for index, case in enumerate(cases))
        with tempfile.TemporaryDirectory(prefix='crossdesk-policy-tests-') as temporary:
            directory = Path(temporary)
            harness = directory / 'PolicyHarness.java'
            harness.write_text('import java.util.*;\nimport java.util.regex.*;\nclass PolicyHarness {\nstatic String selectText' +
                               methods + '\npublic static void main(String[] args) {\n' + assertions +
                               '\nSystem.out.println("PASS: policy sections and locale fallback");\n}\n}', encoding='utf-8')
            subprocess.run(['javac', '-encoding', 'UTF-8', str(harness)], check=True, capture_output=True, text=True)
            result = subprocess.run(['java', '-cp', str(directory), 'PolicyHarness'], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)


if __name__ == '__main__':
    unittest.main()
