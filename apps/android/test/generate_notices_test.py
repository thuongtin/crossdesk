import importlib.util
import json
import os
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch


SCRIPT = Path(__file__).resolve().parents[1] / 'scripts/generate_notices.py'
spec = importlib.util.spec_from_file_location('android_notices', SCRIPT)
notices = importlib.util.module_from_spec(spec)
spec.loader.exec_module(notices)


class GenerateNoticesTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.android = self.root / 'apps/android'
        self.output = self.root / 'output'
        self.output.mkdir()
        files = [
            'LICENSE', 'apps/android/README.md',
            'apps/android/licenses/OPEN_SOURCE_NOTICE.txt',
            'apps/android/native/xmake.lua', 'apps/android/scripts/build_native.py',
            'deps/submodules/minirtc/xmake.lua',
            'deps/submodules/minirtc/thirdparty/openssl/xmake.lua',
            'deps/submodules/minirtc/thirdparty/svt-av1/xmake.lua',
        ]
        for filename in files:
            path = self.root / filename
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('fixture content\n', encoding='utf-8')
        for name, value in [('ROOT', self.root), ('ANDROID', self.android)]:
            patcher = patch.object(notices, name, value)
            patcher.start()
            self.addCleanup(patcher.stop)
        patcher = patch.object(notices, 'git', return_value='')
        patcher.start()
        self.addCleanup(patcher.stop)
        patcher = patch.dict(os.environ, {'CROSSDESK_SOURCE_TAG': '', 'CROSSDESK_RELEASE_BUILD': 'NO'})
        patcher.start()
        self.addCleanup(patcher.stop)

    def generate(self, components):
        notices.write_pages(self.output, {'components': components}, [], 'revision', 'minirtc')
        return json.loads((self.output / 'SourceMetadata.json').read_text(encoding='utf-8'))

    def test_vietnamese_titles_and_verbatim_documents(self):
        component = {'id': 'crossdesk', 'documents': []}
        metadata = self.generate([component])
        titles = [document['title'] for document in metadata['buildDocuments']]
        self.assertEqual(titles, [
            'Hướng dẫn biên dịch', 'Công thức biên dịch mã gốc',
            'Công thức biên dịch OpenSSL', 'Công thức biên dịch MiniRTC',
            'Script biên dịch mã gốc', 'Công thức biên dịch SVT-AV1',
        ])
        for document in metadata['buildDocuments']:
            self.assertEqual((self.output / document['asset']).read_text(encoding='utf-8'), 'fixture content\n')
        self.assertEqual((self.output / 'about/licenses/crossdesk/0.txt').read_text(), 'fixture content\n')

    def test_empty_component_list(self):
        self.generate([])
        catalog = json.loads((self.output / 'ThirdPartyLicenses.json').read_text())
        self.assertEqual(catalog['components'], [])

    def test_missing_build_document_is_an_error(self):
        (self.android / 'README.md').unlink()
        with self.assertRaises(FileNotFoundError):
            self.generate([])


if __name__ == '__main__':
    unittest.main()
