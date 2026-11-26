import contextlib
import importlib.util
import io
from pathlib import Path
import sys
import unittest
from unittest import mock


SCRIPT = Path(__file__).resolve().with_name('merge_localization_catalog.py')
spec = importlib.util.spec_from_file_location('merge_localization_catalog', SCRIPT)
merge = importlib.util.module_from_spec(spec)
spec.loader.exec_module(merge)


def catalog(**extra):
    result = {
        'languages': ['zh', 'en', 'vi'],
        'strings': {'连接': {'zh': '连接', 'en': 'Connect', 'vi': 'Kết nối'}},
        'desktop': {'connect': '连接'},
        'slint': ['连接'],
        'platforms': {'android': ['连接']},
    }
    result.update(extra)
    return result


def run_main(catalogs):
    argv = ['merge_localization_catalog.py', '--dry-run', '--base', 'base', '--ours', 'ours', '--theirs', 'theirs']
    output = io.StringIO()
    with mock.patch.object(sys, 'argv', argv), \
            mock.patch.object(merge, 'load_catalog', side_effect=lambda ref: catalogs[ref]), \
            mock.patch.object(merge, 'remap_fork_slint_files', return_value=([], [])), \
            contextlib.redirect_stdout(output):
        return merge.main(), output.getvalue()


class UnknownSectionTest(unittest.TestCase):
    def test_section_added_on_any_side_stops_the_merge(self):
        for side in ('base', 'ours', 'theirs'):
            with self.subTest(side=side):
                catalogs = {name: catalog() for name in ('base', 'ours', 'theirs')}
                catalogs[side] = catalog(themes={'dark': '深色'})
                with self.assertRaises(SystemExit) as raised:
                    run_main(catalogs)
                self.assertEqual(str(raised.exception), f'unknown catalog section in {side}: themes')

    def test_known_sections_merge_normally(self):
        catalogs = {name: catalog() for name in ('base', 'ours', 'theirs')}
        status, output = run_main(catalogs)
        self.assertEqual(status, 0)
        self.assertIn('strings: 1, slint literals: 1', output)


if __name__ == '__main__':
    unittest.main()
