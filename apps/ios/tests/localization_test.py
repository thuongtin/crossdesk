import json
from pathlib import Path
import plistlib
import unittest


ROOT = Path(__file__).resolve().parents[3]
APP = ROOT / 'apps/ios/CrossDeskMobile'
PERMISSION = 'CrossDesk 使用局域网发现并建立原生 P2P 远程控制连接。'


class LocalizationTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog = json.loads((ROOT / 'localization/catalog.json').read_text(encoding='utf-8'))

    def test_base_permission_matches_vietnamese_fallback(self):
        with (APP / 'Info.plist').open('rb') as handle:
            info = plistlib.load(handle)
        self.assertEqual(info['NSLocalNetworkUsageDescription'], self.catalog['strings'][PERMISSION]['vi'])
        self.assertEqual(info['CFBundleDevelopmentRegion'], 'vi')

    def test_generated_permissions_preserve_each_locale(self):
        for language in self.catalog['languages']:
            with self.subTest(language=language):
                text = (APP / f'Resources/{language}.lproj/InfoPlist.strings').read_text(encoding='utf-8')
                value = json.loads(text.split(' = ', 1)[1].removesuffix(';\n'))
                self.assertEqual(value, self.catalog['strings'][PERMISSION][language])


if __name__ == '__main__':
    unittest.main()
