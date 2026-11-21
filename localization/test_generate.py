import copy
import json
from pathlib import Path
import plistlib
import shutil
import subprocess
import tempfile
import unittest
import xml.etree.ElementTree as ET

import generate


class LocalizationTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data = json.loads(generate.CATALOG.read_text(encoding='utf-8'))
        cls.outputs = generate.render(cls.data)

    def android_strings(self, qualifier):
        text = self.outputs[f'apps/android/app/src/main/res/{qualifier}/strings.xml']
        return {item.attrib['name']: item.text for item in ET.fromstring(text)}

    def test_android_defaults_and_explicit_languages(self):
        default = self.android_strings('values')
        self.assertEqual(default, self.android_strings('values-vi'))
        key = generate.resource_id('连接')
        self.assertEqual(default[key], '"Kết nối"')
        self.assertEqual(self.android_strings('values-en')[key], '"Connect"')
        config = ET.fromstring(self.outputs['apps/android/app/src/main/res/xml/locales_config.xml'])
        self.assertEqual({item.attrib['{http://schemas.android.com/apk/res/android}name'] for item in config},
                         set(self.data['languages']))

    def test_all_generated_languages(self):
        header = self.outputs['apps/desktop/src/gui/assets/localization/localization_data.h']
        self.assertIn(f'const char* text[{len(self.data["languages"])}]', header)
        for language in self.data['languages']:
            prefix = f'apps/ios/CrossDeskMobile/Resources/{language}.lproj/'
            self.assertIn(prefix + 'Localizable.strings', self.outputs)
            self.assertIn(prefix + 'InfoPlist.strings', self.outputs)
        harmony = self.outputs['apps/harmony/entry/src/main/ets/mobile/LocalizationData.ets']
        for line in harmony.splitlines()[2:-1]:
            values = json.loads('[' + line.split(': [', 1)[1].removesuffix(','))
            self.assertEqual(len(values), len(self.data['languages']))

    def test_vietnamese_fallback_for_missing_language(self):
        data = copy.deepcopy(self.data)
        del data['strings']['连接']['en']
        output = generate.render(data)['apps/android/app/src/main/res/values-en/strings.xml']
        self.assertIn('"Kết nối"', output)

    def test_vietnamese_feature_terminology(self):
        cases = {
            '桌面墙': 'DeskHub',
            '剪贴板': 'Clipboard',
            'TURN 中继': 'TURN relay',
            '强制中继连接': 'Buộc kết nối qua relay',
            '信令连接失败': 'Kết nối signaling thất bại',
            '视频编码': 'Codec video',
            '相对鼠标': 'Bàn di chuột',
            '绝对鼠标': 'Chạm trực tiếp',
            '查看指纹': 'Xem fingerprint',
        }
        for key, vietnamese in cases.items():
            with self.subTest(key=key):
                self.assertEqual(self.data['strings'][key]['vi'], vietnamese)
                if key in self.data['platforms']['android']:
                    for qualifier in ['values', 'values-vi']:
                        self.assertEqual(self.android_strings(qualifier)[generate.resource_id(key)],
                                         '"' + vietnamese + '"')
                if key in self.data['platforms']['desktop']:
                    header = self.outputs['apps/desktop/src/gui/assets/localization/localization_data.h']
                    self.assertIn(json.dumps(vietnamese, ensure_ascii=False), header)

    def test_empty_translation_is_preserved(self):
        data = copy.deepcopy(self.data)
        data['strings']['连接']['vi'] = ''
        output = generate.render(data)['apps/android/app/src/main/res/values/strings.xml']
        self.assertIn(f'name="{generate.resource_id("连接")}" formatted="false">""</string>', output)

    def test_file_browser_catalog_and_generated_literals(self):
        cases = {
            '文件传输': 'Truyền tệp',
            '此目录为空': 'Thư mục này trống',
            '目录列表未能完整加载，请刷新重试。':
                'Không thể tải đầy đủ danh sách thư mục, hãy làm mới để thử lại.',
            '已传输 ': 'Đã truyền ',
        }
        header = self.outputs['apps/desktop/src/gui/assets/localization/localization_data.h']
        for key, vietnamese in cases.items():
            with self.subTest(key=key):
                self.assertIn(key, self.data['platforms']['desktop'])
                self.assertIn(key, self.data['slint'])
                self.assertEqual(set(self.data['strings'][key]), set(self.data['languages']))
                self.assertEqual(self.data['strings'][key]['vi'], vietnamese)
                self.assertIn(json.dumps(vietnamese, ensure_ascii=False), header)

    def test_ios_fallback_and_bundle_registration(self):
        app = generate.ROOT / 'apps/ios'
        with (app / 'CrossDeskMobile/Info.plist').open('rb') as handle:
            self.assertEqual(plistlib.load(handle)['CFBundleDevelopmentRegion'], 'vi')
        project = (app / 'CrossDeskMobile.xcodeproj/project.pbxproj').read_text(encoding='utf-8')
        self.assertIn('developmentRegion = vi;', project)
        self.assertRegex(project, r'knownRegions = \(\s*vi,')
        self.assertIn('Resources/vi.lproj/Localizable.strings', project)
        self.assertIn('Resources/vi.lproj/InfoPlist.strings', project)
        self.assertEqual(project.count('A4B0CD5FA58B47E68246F901 /* vi */'), 2)
        self.assertEqual(project.count('A4B0CD5FA58B47E68246F902 /* vi */'), 2)

    @unittest.skipUnless(shutil.which('g++') or shutil.which('clang++'), 'no configured C++ compiler')
    def test_desktop_language_mapping(self):
        source = r'''
#include <cassert>
#include "localization.h"
int main() {
  using namespace crossdesk::localization;
  assert(kSystemLanguage == 10);
  assert(kVietnameseLanguage == 11);
  assert(MatchLanguage("vi-VN") == kVietnameseLanguage);
  assert(MatchLanguage("VI_vn.UTF-8") == kVietnameseLanguage);
  assert(MatchLanguage("zh-CN") == 0);
  assert(MatchLanguage("en-GB") == 1);
  assert(MatchLanguage("ru") == 2);
  assert(MatchLanguage("zh-Hant") == 3);
  assert(MatchLanguage("pt-PT") == 9);
  assert(MatchLanguage("unsupported") == -1);
  assert(MatchPreferredLanguages({}) == kVietnameseLanguage);
  assert(MatchPreferredLanguages({"unsupported"}) == kVietnameseLanguage);
  assert(MatchPreferredLanguages({"unsupported", "en-US"}) == 1);
  assert(detail::ClampLanguageIndex(10) == SystemLanguage());
  assert(detail::ClampLanguageIndex(11) == 11);
  assert(detail::ClampLanguageIndex(-1) == 11);
  assert(detail::ClampLanguageIndex(100) == 11);
  assert(Literal("连接", 11) == "Kết nối");
  assert(Literal("连接", detail::ClampLanguageIndex(11)) == "Kết nối");
  assert(Literal("连接", 1) == "Connect");
  assert(Literal("unknown", 11).empty());
  assert(GetSupportedLanguages().back().display_name == "Tiếng Việt");
}
'''
        with tempfile.TemporaryDirectory() as directory:
            directory = Path(directory)
            harness = directory / 'localization_test.cpp'
            harness.write_text(source, encoding='utf-8')
            executable = directory / 'localization_test.exe'
            compiler = shutil.which('g++') or shutil.which('clang++')
            subprocess.run([compiler, '-std=c++17', '-I' + str(generate.ROOT / 'apps/desktop/src/gui/assets/localization'),
                            str(harness), '-o', str(executable)], check=True)
            subprocess.run([str(executable)], check=True)


if __name__ == '__main__':
    unittest.main()
