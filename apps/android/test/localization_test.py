import hashlib
import json
from pathlib import Path
import re
import unittest
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parents[3]
APP = ROOT / 'apps/android/app/src/main'
JAVA = APP / 'java/cn/crossdesk/mobile'


class LocalizationTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog = json.loads((ROOT / 'localization/catalog.json').read_text(encoding='utf-8'))
        cls.resources = {item.attrib['name']: item.text for item in ET.parse(APP / 'res/values/strings.xml').getroot()}

    def test_browser_error_uses_existing_localized_resource(self):
        key = '未找到可打开链接的浏览器'
        self.assertIn('L10n.tr(' + json.dumps(key, ensure_ascii=False) + ')',
                      (JAVA / 'AnnouncementText.java').read_text(encoding='utf-8'))
        identifier = 'text_' + hashlib.sha256(key.encode()).hexdigest()[:16]
        self.assertEqual(self.resources[identifier], '"' + self.catalog['strings'][key]['vi'] + '"')
        self.assertNotRegex(self.resources[identifier], '[\u3400-\u9fff]')

    def test_service_initializes_localization_before_channel_creation(self):
        source = (JAVA / 'RemoteSessionService.java').read_text(encoding='utf-8')
        self.assertLess(source.index('L10n.initialize(this)'), source.index('.createNotificationChannel('))
        self.assertIn('L10n.tr("远程连接")', source)

    def test_project_ui_keys_are_registered(self):
        for filename in ['AboutPages.java', 'LicenseCatalog.java', 'AnnouncementView.java',
                         'RemoteSessionService.java', 'RemoteShortcutPanel.java', 'RemoteKeyboard.java']:
            source = (JAVA / filename).read_text(encoding='utf-8')
            for key in re.findall(r'L10n\.tr\("([^"\\]+)"\)', source):
                with self.subTest(filename=filename, key=key):
                    self.assertIn(key, self.catalog['platforms']['android'])
                    self.assertTrue(self.catalog['strings'][key]['vi'])

    def test_instrumentation_ui_selectors_are_locale_aware(self):
        tests = Path(__file__).with_name('android') / 'java/cn/crossdesk/mobile'
        privacy = (tests / 'PrivacyConsentTest.java').read_text(encoding='utf-8')
        # Upstream tách nhãn đã dịch ra biến `label`; chỉ cần bộ chọn tìm theo nhãn đã qua L10n.tr.
        self.assertIn('String label=L10n.tr(text);', privacy)
        self.assertIn('findText(dialog.getWindow().getDecorView(),label)', privacy)
        updates = (tests / 'AppUpdateCheckerTest.java').read_text(encoding='utf-8')
        # Nhãn cập nhật được định dạng qua L10n.format một lần rồi dùng lại cho mọi lần tìm.
        self.assertEqual(updates.count('L10n.format('), 1)
        self.assertEqual(updates.count('findText(activity.getWindow().getDecorView(), updateLabel)'), 2)


if __name__ == '__main__':
    unittest.main()
