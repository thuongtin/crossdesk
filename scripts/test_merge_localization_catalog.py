import contextlib
import importlib.util
import io
import json
from pathlib import Path
import sys
import tempfile
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


def strings(**values):
    return {key: {'zh': key, 'en': english} for key, english in values.items()}


def run_main(catalogs):
    argv = ['merge_localization_catalog.py', '--dry-run', '--base', 'base', '--ours', 'ours', '--theirs', 'theirs']
    output = io.StringIO()
    with mock.patch.object(sys, 'argv', argv), \
            mock.patch.object(merge, 'load_catalog', side_effect=lambda ref: catalogs[ref]), \
            mock.patch.object(merge, 'remap_fork_slint_files', return_value=([], [])), \
            contextlib.redirect_stdout(output):
        return merge.main(), output.getvalue()


def run_main_writing(catalogs):
    """Chạy main không dry-run trên một ROOT tạm, trả về mã thoát, stdout và catalog đã ghi."""
    argv = ['merge_localization_catalog.py', '--base', 'base', '--ours', 'ours', '--theirs', 'theirs',
            '--language', 'en']
    output = io.StringIO()
    with tempfile.TemporaryDirectory() as root:
        (Path(root) / 'localization').mkdir()
        with mock.patch.object(sys, 'argv', argv), \
                mock.patch.object(merge, 'ROOT', Path(root)), \
                mock.patch.object(merge, 'load_catalog', side_effect=lambda ref: catalogs[ref]), \
                mock.patch.object(merge, 'remap_fork_slint_files', return_value=([], [])), \
                contextlib.redirect_stdout(output):
            status = merge.main()
        written = json.loads((Path(root) / merge.CATALOG_PATH).read_text(encoding='utf-8'))
    return status, output.getvalue(), written


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


class DeleteEditConflictTest(unittest.TestCase):
    def test_upstream_edit_survives_a_fork_deletion(self):
        base = catalog(strings=strings(连接='Connect', 断开='Disconnect'))
        ours = catalog(strings=strings(连接='Connect'))
        theirs = catalog(strings=strings(连接='Connect', 断开='Disconnect now'))
        status, output, written = run_main_writing({'base': base, 'ours': ours, 'theirs': theirs})
        self.assertEqual(status, 1)
        self.assertEqual(written['strings']['断开'], {'zh': '断开', 'en': 'Disconnect now'})
        self.assertIn('1 keys deleted in the fork but changed upstream', output)
        self.assertIn("[strings] '断开'", output)

    def test_fork_deletion_of_an_untouched_key_still_drops_it(self):
        base = catalog(strings=strings(连接='Connect', 断开='Disconnect'))
        ours = catalog(strings=strings(连接='Connect'))
        theirs = catalog(strings=strings(连接='Connect', 断开='Disconnect'))
        status, output, written = run_main_writing({'base': base, 'ours': ours, 'theirs': theirs})
        self.assertEqual(status, 0)
        self.assertNotIn('断开', written['strings'])
        self.assertNotIn('deleted in the fork', output)

    def test_desktop_mapping_keeps_upstream_edit_of_a_fork_deletion(self):
        names = strings(连接='Connect', 断开='Disconnect', 取消='Cancel')
        base = catalog(strings=names, desktop={'connect': '连接', 'cancel': '取消'})
        ours = catalog(strings=names, desktop={'connect': '连接'})
        theirs = catalog(strings=names, desktop={'connect': '连接', 'cancel': '断开'})
        status, output, written = run_main_writing({'base': base, 'ours': ours, 'theirs': theirs})
        self.assertEqual(status, 1)
        self.assertEqual(written['desktop'], {'connect': '连接', 'cancel': '断开'})
        self.assertIn("[desktop] 'cancel'", output)

    def test_desktop_mapping_reports_edit_edit_conflict(self):
        names = strings(连接='Connect', 断开='Disconnect', 取消='Cancel')
        base = catalog(strings=names, desktop={'connect': '连接'})
        ours = catalog(strings=names, desktop={'connect': '断开'})
        theirs = catalog(strings=names, desktop={'connect': '取消'})
        status, output, written = run_main_writing({'base': base, 'ours': ours, 'theirs': theirs})
        self.assertEqual(status, 1)
        self.assertEqual(written['desktop'], {'connect': '取消'})
        self.assertIn("[desktop] 'connect': fork='断开' upstream='取消'", output)


class SlintRemapTest(unittest.TestCase):
    PATH = 'apps/desktop/src/gui/ui/sample.slint'
    # Literal 'f' là của fork (chỉ số 2 phía fork); upstream chèn 'u' vào chỉ số 2, nên 'f' chuyển sang 3.
    BASE = {'slint': ['a', 'b']}
    OURS = {'slint': ['a', 'b', 'f']}
    THEIRS = {'slint': ['a', 'b', 'u']}
    MERGED = ['a', 'b', 'u', 'f']
    FORK_TEXT = 'title: GalleryStrings.texts[2];\nhint: GalleryStrings.texts[0];\n'

    def remap(self, root):
        refs = {'ours': {self.PATH}, 'theirs': set()}
        stderr = io.StringIO()
        with mock.patch.object(merge, 'ROOT', root), \
                mock.patch.object(merge, 'slint_files_with_new_refs', side_effect=lambda ref, count: refs[ref]), \
                mock.patch.object(merge, 'git', side_effect=lambda *args: self.FORK_TEXT), \
                contextlib.redirect_stderr(stderr):
            changed, manual = merge.remap_fork_slint_files(
                self.BASE, self.OURS, self.THEIRS, self.MERGED, 'ours', 'theirs', dry_run=False)
        return changed, manual, stderr.getvalue()

    def test_second_run_after_an_upstream_edit_is_a_no_op(self):
        with tempfile.TemporaryDirectory() as root:
            file_path = Path(root) / self.PATH
            file_path.parent.mkdir(parents=True)
            # Upstream sửa phần khác của tệp (chỉ số 0 thành 1), nên tệp đã merge khác bản fork gốc.
            file_path.write_text('title: GalleryStrings.texts[2];\nhint: GalleryStrings.texts[1];\n')
            self.assertEqual(self.remap(Path(root))[:2], ([self.PATH], []))
            remapped = 'title: GalleryStrings.texts[3];\nhint: GalleryStrings.texts[1];\n'
            self.assertEqual(file_path.read_text(), remapped)
            self.assertEqual(self.remap(Path(root))[:2], ([], []))
            self.assertEqual(file_path.read_text(), remapped)

    def test_unrecognised_fork_indexes_are_left_for_manual_remap(self):
        with tempfile.TemporaryDirectory() as root:
            file_path = Path(root) / self.PATH
            file_path.parent.mkdir(parents=True)
            file_path.write_text('title: GalleryStrings.texts[5];\n')
            changed, manual, stderr = self.remap(Path(root))
            self.assertEqual((changed, manual), ([], [self.PATH]))
            self.assertIn('remap them by hand', stderr)
            self.assertEqual(file_path.read_text(), 'title: GalleryStrings.texts[5];\n')


if __name__ == '__main__':
    unittest.main()
