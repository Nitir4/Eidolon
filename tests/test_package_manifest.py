"""Keep package source identities honest for local, uncommitted application builds."""
import hashlib
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from package_desktop import manifest, source_provenance


class PackageIdentity(unittest.TestCase):
    def git(self, root, *arguments):
        return subprocess.run(['git', '-C', str(root), *arguments], check=True,
                              capture_output=True, text=True).stdout.strip()

    def test_dirty_and_untracked_content_changes_identity(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.git(root, 'init')
            self.git(root, 'config', 'user.name', 'Package Test')
            self.git(root, 'config', 'user.email', 'package@example.invalid')
            (root / 'source.cpp').write_text('first')
            self.git(root, 'add', 'source.cpp')
            self.git(root, 'commit', '-m', 'Initial fixture')
            clean = source_provenance(root)
            self.assertFalse(clean['dirty'])
            (root / 'source.cpp').write_text('changed')
            dirty = source_provenance(root)
            self.assertTrue(dirty['dirty'])
            self.assertEqual(clean['base_commit'], dirty['base_commit'])
            self.assertNotEqual(clean['repository_tree_sha256'], dirty['repository_tree_sha256'])
            (root / 'new.cpp').write_text('new module')
            untracked = source_provenance(root)
            self.assertTrue(untracked['dirty'])
            self.assertNotEqual(dirty['repository_tree_sha256'], untracked['repository_tree_sha256'])

    def test_full_module_manifest_hashes_and_rejects_reference_payload(self):
        with tempfile.TemporaryDirectory() as directory:
            stage = Path(directory)
            (stage / 'ToraClone').write_bytes(b'application')
            with patch('package_desktop.sdk_tool', return_value=Path('/sdk/qmake')), \
                    patch('package_desktop.subprocess.check_output', return_value='6.11.2\n'), \
                    patch('package_desktop.source_provenance', return_value={'dirty': True}):
                data = manifest(stage, Path('/sdk'), '0.1.0', 'linux-x86_64')
                for module in ['Equations', 'Integer', 'Networks', 'Planning', 'Queuing', 'Games', 'Assignment']:
                    self.assertIn(module, data['stage'])
                self.assertTrue(data['source']['dirty'])
                self.assertEqual(data['files']['ToraClone'], hashlib.sha256(b'application').hexdigest())
                (stage / 'reference').mkdir()
                (stage / 'reference/private.txt').write_text('private')
                with self.assertRaisesRegex(RuntimeError, 'Forbidden reference'):
                    manifest(stage, Path('/sdk'), '0.1.0', 'linux-x86_64')


if __name__ == '__main__':
    unittest.main()
