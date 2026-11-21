import importlib.util
import json
import os
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch


SCRIPT = Path(__file__).resolve().parents[1] / 'scripts/generate_mobile_resources.py'
spec = importlib.util.spec_from_file_location('harmony_resources', SCRIPT)
resources = importlib.util.module_from_spec(spec)
spec.loader.exec_module(resources)


class GenerateMobileResourcesTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.app = self.root / 'apps/harmony'
        self.output = self.root / 'output'
        files = {
            'LICENSE': 'legal fixture\n', 'PRIVACY.md': 'policy fixture\n',
            'apps/android/scripts/generate_notices.py': 'COMPONENTS = set()\n',
            'apps/android/licenses/additional-notices.json': '[]',
            'apps/ios/CrossDeskMobile/Resources/ThirdPartyLicenses.json': '{"components": []}',
            'sdk/default/openharmony/native/llvm/NOTICE': 'LLVM legal fixture\n',
            'apps/harmony/README.md': 'build fixture\n',
            'apps/harmony/MOBILE_PARITY.md': 'parity fixture\n',
            'apps/harmony/native/xmake.lua': 'native fixture\n',
            'apps/harmony/scripts/build.py': 'script fixture\n',
            'deps/submodules/minirtc/xmake.lua': 'MiniRTC fixture\n',
            'deps/submodules/minirtc/thirdparty/openssl/xmake.lua': 'OpenSSL fixture\n',
        }
        for filename, content in files.items():
            path = self.root / filename
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content, encoding='utf-8')
        for name, value in [('ROOT', self.root), ('APP', self.app)]:
            patcher = patch.object(resources, name, value)
            patcher.start()
            self.addCleanup(patcher.stop)
        patcher = patch.object(resources, 'git', side_effect=lambda *args, **kwargs: '' if args[0] == 'status' else 'revision')
        patcher.start()
        self.addCleanup(patcher.stop)
        patcher = patch.dict(os.environ, {'DEVECO_SDK_HOME': str(self.root / 'sdk')})
        patcher.start()
        self.addCleanup(patcher.stop)

    def generate(self):
        resources.main(self.output)
        return json.loads((self.output / 'SourceMetadata.json').read_text(encoding='utf-8'))

    def test_vietnamese_titles_and_verbatim_documents(self):
        metadata = self.generate()
        self.assertEqual([document['title'] for document in metadata['buildDocuments']], [
            'Hướng dẫn biên dịch HarmonyOS', 'Đối chiếu tính năng di động',
            'Công thức biên dịch mã gốc', 'Script biên dịch',
            'Công thức biên dịch MiniRTC', 'Công thức biên dịch OpenSSL',
        ])
        self.assertEqual((self.output / metadata['buildDocuments'][0]['asset']).read_text(), 'build fixture\n')
        self.assertEqual((self.output / 'about/licenses/ohos-llvm/0.txt').read_text(), 'LLVM legal fixture\n')

    def test_missing_optional_build_documents(self):
        for filename in ['README.md', 'MOBILE_PARITY.md', 'native/xmake.lua', 'scripts/build.py']:
            (self.app / filename).unlink()
        for filename in ['xmake.lua', 'thirdparty/openssl/xmake.lua']:
            (self.root / 'deps/submodules/minirtc' / filename).unlink()
        self.assertEqual(self.generate()['buildDocuments'], [])

    def test_missing_required_legal_document_is_an_error(self):
        (self.root / 'sdk/default/openharmony/native/llvm/NOTICE').unlink()
        with self.assertRaises(FileNotFoundError):
            self.generate()


if __name__ == '__main__':
    unittest.main()
