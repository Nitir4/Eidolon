"""Check notice provenance and keep version-specific attributions truthful."""
import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from runtime_notices import validate_qt_bundle, copy_qt_notices, QT_BUNDLE


class RuntimeNotices(unittest.TestCase):
    def test_real_upstream_bundle_is_complete_and_integral(self):
        data = validate_qt_bundle(QT_BUNDLE)
        self.assertEqual(data['qt_version'], '6.10.3')
        names = {r['path'] for r in data['files']}
        self.assertIn('qtbase/LICENSES/LGPL-3.0-only.txt', names)
        self.assertIn('qtbase/LICENSES/GPL-3.0-only.txt', names)
        self.assertTrue(any(n.endswith('/qt_attribution.json') for n in names))

    def test_corruption_is_rejected(self):
        with tempfile.TemporaryDirectory() as name:
            root = Path(name)
            (root / 'license.txt').write_text('modified')
            data = {'qt_version': '6.10.3', 'files': [{'path': 'license.txt',
                    'sha256': hashlib.sha256(b'original').hexdigest()}]}
            (root / 'SOURCES.json').write_text(json.dumps(data))
            with self.assertRaisesRegex(ValueError, 'hash'):
                validate_qt_bundle(root)

    def test_version_mismatch_does_not_mislabel_attributions(self):
        with tempfile.TemporaryDirectory() as name:
            stage = Path(name)
            self.assertFalse(copy_qt_notices(stage, '6.11.2'))
            self.assertFalse((stage / 'licenses').exists())
            self.assertTrue(copy_qt_notices(stage, '6.10.3'))
            self.assertTrue((stage / 'licenses/Qt-6.10.3/SOURCES.json').is_file())

    def test_parent_path_is_rejected(self):
        with tempfile.TemporaryDirectory() as name:
            root = Path(name)
            (root / 'SOURCES.json').write_text(json.dumps({'files': [{'path': '../private'}]}))
            with self.assertRaisesRegex(ValueError, 'path'):
                validate_qt_bundle(root)


if __name__ == '__main__':
    unittest.main()
