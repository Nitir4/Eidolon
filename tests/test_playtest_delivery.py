"""Exercise fresh playtest material, honest comparisons and ZIP path safety."""
import csv
import importlib.util
import json
from pathlib import Path
import stat
import sys
import tempfile
import unittest
import zipfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from package_playtest import build, copy_macos_payload, extract_windows
from playtest_material import make_material, original_death_mean

spec = importlib.util.spec_from_file_location('compare_results', ROOT/'playtest/scripts/compare-results.py')
review = importlib.util.module_from_spec(spec)
spec.loader.exec_module(review)


class PlaytestDelivery(unittest.TestCase):
    def test_blank_observations_never_pass(self):
        row = {'eidolon': '', 'expected_eidolon': '10', 'absolute_tolerance': '.000051'}
        self.assertEqual(review.check(row, 'eidolon'), 'UNTESTED')
        row['eidolon'] = 'unsupported'
        self.assertEqual(review.check(row, 'eidolon'), 'SKIPPED')
        row['eidolon'] = 'NaN'
        self.assertEqual(review.check(row, 'eidolon'), 'FAIL')
        row['eidolon'] = '10.00004'
        self.assertEqual(review.check(row, 'eidolon'), 'PASS')
        row['eidolon'] = '10.001'
        self.assertEqual(review.check(row, 'eidolon'), 'FAIL')

    def test_original_death_recovered_values(self):
        self.assertAlmostEqual(original_death_mean(2, 1), 1/2.718281828459045)
        self.assertAlmostEqual(original_death_mean(500, 2), 430.33169269056265)
        self.assertEqual(original_death_mean(501, 2), original_death_mean(500, 2))

    def test_complete_bank_and_distinct_queue_keys(self):
        with tempfile.TemporaryDirectory() as temporary:
            destination = Path(temporary)
            coverage = make_material(ROOT/'examples/practice-bank', destination)
            self.assertEqual(coverage['cases'], 126)
            self.assertEqual(coverage['workflows'], 42)
            self.assertEqual(coverage['workflow_cases'], 210)
            with (destination/'comparison-template.csv').open(newline='') as handle:
                rows = list(csv.DictReader(handle))
            self.assertEqual(len(rows), coverage['scalar_rows'])
            self.assertTrue(all(not r['eidolon'] and not r['tora'] for r in rows))
            queue = next(r for r in rows if r['case']=='queue-finite-kn-01' and r['metric']=='W')
            self.assertNotEqual(queue['expected_eidolon'], queue['expected_tora'])
            maximum = next(r for r in rows if r['case']=='assignment-max-01')
            self.assertEqual(float(maximum['expected_eidolon']), -float(maximum['expected_tora']))
            self.assertEqual(len(json.loads((destination/'tora-equivalents/index.json').read_text())), 21)

    def test_zip_cannot_escape_delivery(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            archive = root/'native.zip'
            with zipfile.ZipFile(archive, 'w') as handle:
                handle.writestr('../outside.txt', 'unexpected')
            with self.assertRaises(ValueError):
                extract_windows(archive, root/'payload')
            self.assertFalse((root/'outside.txt').exists())

    def test_direct_mac_app_preserves_executable_permission(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            native = root/'native'
            native.mkdir()
            artifact = native/'Eidolon-test-macos-universal.zip'
            with zipfile.ZipFile(artifact, 'w') as handle:
                executable = zipfile.ZipInfo('Eidolon.app/Contents/MacOS/Eidolon')
                executable.create_system = 3
                executable.external_attr = 0o100755 << 16
                handle.writestr(executable, b'temporary unit fixture')
                framework = 'Eidolon.app/Contents/Frameworks/QtCore.framework'
                current = zipfile.ZipInfo(framework + '/Versions/Current')
                current.create_system = 3
                current.external_attr = (stat.S_IFLNK | 0o777) << 16
                handle.writestr(current, b'A')
                handle.writestr(framework + '/Versions/A/QtCore', b'unit framework file')
                handle.writestr('__MACOSX/Eidolon.app/Contents/MacOS/._Eidolon', b'unit metadata')
            artifact.with_suffix('.zip.manifest.json').write_text(json.dumps({
                'application':'Eidolon', 'version':'test',
                'build_validation':'cross-built; no native execution'}))
            archive = build(native, root/'output', 'macos')
            with zipfile.ZipFile(archive) as handle:
                member = handle.getinfo('app/Eidolon.app/Contents/MacOS/Eidolon')
                self.assertEqual(member.create_system, 3)
                self.assertTrue(member.external_attr >> 16 & 0o111)
                link = handle.getinfo('app/' + current.filename)
                self.assertTrue(stat.S_ISLNK(link.external_attr >> 16))
                self.assertEqual(handle.read(link), b'A')
                files = json.loads(handle.read('bundle-manifest.json'))['files']
                self.assertNotIn(link.filename, files)
                self.assertIn('app/' + framework + '/Versions/A/QtCore', files)
                metadata = '__MACOSX/app/Eidolon.app/Contents/MacOS/._Eidolon'
                self.assertEqual(handle.read(metadata), b'unit metadata')
                self.assertNotIn(metadata, files)
                release = json.loads(handle.read('release-info.json'))
                self.assertIn('no native execution', release['build_validation'])

    def test_macos_payload_cannot_redirect_extraction(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            native = root/'native.zip'
            for target in ('../outside', 'Versions/A'):
                with zipfile.ZipFile(native, 'w') as handle:
                    handle.writestr('Eidolon.app/Contents/MacOS/Eidolon', b'unit fixture')
                    link = zipfile.ZipInfo('Eidolon.app/Contents/Frameworks/Current')
                    link.external_attr = (stat.S_IFLNK | 0o777) << 16
                    handle.writestr(link, target)
                    handle.writestr(link.filename + '/external.txt', b'unit file')
                with zipfile.ZipFile(root/'playtest.zip', 'w') as output:
                    with self.assertRaisesRegex(ValueError, 'Unsafe macOS payload link|through a symlink'):
                        copy_macos_payload(native, output)

    def test_archive_contents_and_mac_script_permissions(self):
        # These are tiny fake native payloads used only inside a temporary unit
        # fixture, never delivery artifacts or a claim of native-platform testing.
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            for target in ['windows', 'macos']:
                native = root/target
                native.mkdir()
                if target == 'windows':
                    artifact = native/'Eidolon-test-windows-x86_64.zip'
                    with zipfile.ZipFile(artifact, 'w') as handle:
                        handle.writestr('Eidolon.exe', b'MZ unit-test fixture')
                    (native/'Eidolon-test-windows-x86_64-setup.exe').write_bytes(b'MZ fixture')
                else:
                    artifact = native/'Eidolon-test-macos-universal.dmg'
                    artifact.write_bytes(b'unit-test image fixture')
                artifact.with_suffix(artifact.suffix+'.manifest.json').write_text(json.dumps({'application':'Eidolon','version':'test'}))
                archive = build(native, root/'output', target)
                self.assertEqual(archive.name, target+'.zip')
                with zipfile.ZipFile(archive) as handle:
                    names = handle.namelist()
                    self.assertIn('testing.md', names)
                    self.assertIn('problems/bank.json', names)
                    self.assertIn('comparison-template.csv', names)
                    self.assertFalse(any(n.endswith('tora.exe') for n in names))
                    if target == 'macos':
                        launcher = handle.getinfo('Run-Checks.command')
                        self.assertEqual(launcher.create_system, 3)
                        self.assertEqual((launcher.external_attr >> 16) & 0o777, 0o755)


if __name__ == '__main__':
    unittest.main()
