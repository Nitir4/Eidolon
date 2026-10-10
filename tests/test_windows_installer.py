"""Portable checks for package-owned installation/removal commands."""
import sys
from pathlib import Path, PurePosixPath, PureWindowsPath
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from windows_installer import payload_commands, nsis_literal, install_command


class InstallerSafety(unittest.TestCase):
    def test_literals_do_not_expand_nsis_variables(self):
        self.assertEqual(nsis_literal('cost$5 "quoted"'), 'cost$$5 $\\"quoted$\\"')
        for value in ['bad\npath', 'bad\rpath', 'bad\0path']:
            with self.assertRaises(ValueError):
                nsis_literal(value)

    def test_owns_only_explicit_files_and_removes_empty_directories(self):
        with tempfile.TemporaryDirectory() as name:
            stage = Path(name)
            (stage / 'platforms').mkdir()
            (stage / 'ToraClone.exe').write_bytes(b'MZ')
            (stage / 'platforms/qwindows.dll').write_bytes(b'plugin')
            install, remove = payload_commands(stage)
            self.assertIn('ToraClone.exe', install)
            self.assertIn('qwindows.dll', remove)
            self.assertNotIn('RMDir /r', remove)
            self.assertNotIn('*', remove)
            self.assertLess(remove.index('Delete '), remove.index('RMDir '))
            self.assertIn('RMDir "$INSTDIR\\platforms"', remove)

    def test_refuses_reference_payloads(self):
        with tempfile.TemporaryDirectory() as name:
            stage = Path(name)
            (stage / 'reference').mkdir()
            (stage / 'reference/capture.txt').write_text('private')
            with self.assertRaises(ValueError):
                payload_commands(stage)

    def test_refuses_symlink_payloads(self):
        with tempfile.TemporaryDirectory() as name:
            stage = Path(name)
            (stage / 'ToraClone.exe').write_bytes(b'MZ')
            with patch.object(Path, 'is_symlink', return_value=True):
                with self.assertRaises(ValueError):
                    payload_commands(stage)

    def test_silent_install_destination_is_last_and_unquoted(self):
        for path_type in [Path, PurePosixPath, PureWindowsPath]:
            with self.subTest(path_type=path_type.__name__):
                target = path_type('C:/Native install café')
                command = install_command(path_type('C:/build/setup.exe'), target)
                self.assertTrue(command.endswith(' /D=' + str(target)))
                self.assertIn('/S', command)
                self.assertNotIn('"/D=', command)
                self.assertNotIn('/D="', command)


if __name__ == '__main__':
    unittest.main()
