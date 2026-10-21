#!/usr/bin/env python3
"""Wrap freshly built native distributions with portable playtest material."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import zipfile
from datetime import datetime, timezone

from playtest_material import make_material

ROOT = Path(__file__).resolve().parents[1]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def extract_windows(archive, destination):
    with zipfile.ZipFile(archive) as source:
        for entry in source.infolist():
            relative = Path(entry.filename)
            if relative.is_absolute() or '..' in relative.parts or '\\' in entry.filename:
                raise ValueError('Unsafe native payload member: ' + entry.filename)
            if (entry.external_attr >> 16) & 0o170000 == 0o120000:
                raise ValueError('Windows payload contains a symlink')
        source.extractall(destination)
    if not (destination/'Eidolon.exe').is_file():
        raise ValueError('Windows payload is missing Eidolon.exe')


def single(directory, pattern):
    matches = list(directory.glob(pattern))
    if len(matches) != 1:
        raise ValueError(f'Expected one {pattern}; found {len(matches)}')
    return matches[0]


def build(native_dir, output_dir, target):
    output_dir.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='eidolon-playtest-') as temporary:
        stage = Path(temporary)
        app = stage/'app'
        app.mkdir()
        if target == 'windows':
            native = single(native_dir, 'Eidolon-*-windows-x86_64.zip')
            extract_windows(native, app)
            installer = single(native_dir, 'Eidolon-*-windows-x86_64-setup.exe')
            (app/'setup').mkdir()
            shutil.copy2(installer, app/'setup/Eidolon-Setup.exe')
            platform_files = ['Start-Eidolon.cmd', 'Run-Checks.cmd', 'scripts/run-checks.ps1']
        else:
            native = single(native_dir, 'Eidolon-*-macos-universal.dmg')
            shutil.copy2(native, app/'Eidolon.dmg')
            platform_files = ['Start-Eidolon.command', 'Run-Checks.command', 'scripts/run-checks.sh']
        native_manifest = native.with_suffix(native.suffix+'.manifest.json')
        metadata = json.loads(native_manifest.read_text())
        if metadata['application'] != 'Eidolon':
            raise ValueError('Native distribution has the wrong application identity')
        for relative in platform_files + ['testing.md', 'edge-cases.md', 'scripts/compare-results.py']:
            destination = stage/relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(ROOT/'playtest'/relative, destination)
            if relative.endswith(('.command', '.sh', '.py')):
                destination.chmod(0o755)
        shutil.copytree(ROOT/'examples/practice-bank', stage/'problems')
        coverage = make_material(ROOT/'examples/practice-bank', stage)
        revision = subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', 'HEAD'], text=True).strip()
        release = dict(application='Eidolon', platform=target, version=metadata['version'],
                       source_commit=revision, created_utc=datetime.now(timezone.utc).isoformat(),
                       native_artifact=native.name, native_sha256=digest(native),
                       coverage=coverage, tests='See native CI evidence; tester startup scripts do not certify numerical correctness.',
                       signing='Unsigned/unnotarized playtest distribution; no signing identity is configured.')
        (stage/'release-info.json').write_text(json.dumps(release, indent=2)+'\n')
        shutil.copy2(native_manifest, stage/'native-package-manifest.json')
        files = {p.relative_to(stage).as_posix(): digest(p) for p in sorted(stage.rglob('*')) if p.is_file()}
        # The manifest excludes itself; SHA256SUMS additionally authenticates it.
        (stage/'bundle-manifest.json').write_text(json.dumps({'application': 'Eidolon', 'files': files}, indent=2)+'\n')
        files['bundle-manifest.json'] = digest(stage/'bundle-manifest.json')
        (stage/'SHA256SUMS.txt').write_text(''.join(f'{value}  {name}\n' for name, value in sorted(files.items())))
        archive = output_dir/(target+'.zip')
        with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as output:
            for path in sorted(stage.rglob('*')):
                if path.is_file():
                    output.write(path, path.relative_to(stage).as_posix())
        archive.with_suffix('.zip.sha256').write_text(f'{digest(archive)}  {archive.name}\n')
        print(f'Created {archive} ({coverage})')
        return archive


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-dir', type=Path, required=True)
    parser.add_argument('--output-dir', type=Path, required=True)
    parser.add_argument('--platform', choices=['auto', 'windows', 'macos'], default='auto')
    args = parser.parse_args()
    target = args.platform
    if target == 'auto':
        target = {'win32': 'windows', 'darwin': 'macos'}.get(sys.platform)
        if target is None:
            parser.error('Playtest delivery is supported for Windows/macOS native jobs only')
    build(args.native_dir.resolve(), args.output_dir.resolve(), target)


if __name__ == '__main__':
    main()
