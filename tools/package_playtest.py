#!/usr/bin/env python3
"""Wrap freshly built native distributions with portable playtest material."""
import argparse
from copy import copy
import hashlib
import json
from pathlib import Path, PurePosixPath
import shutil
import stat
import subprocess
import sys
import tempfile
import zipfile
from datetime import datetime, timezone

from playtest_material import make_material
from portable_zip import write_tree

ROOT = Path(__file__).resolve().parents[1]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def extract_payload(archive, destination):
    with zipfile.ZipFile(archive) as source:
        for entry in source.infolist():
            relative = Path(entry.filename)
            if relative.is_absolute() or '..' in relative.parts or '\\' in entry.filename:
                raise ValueError('Unsafe native payload member: ' + entry.filename)
            if (entry.external_attr >> 16) & 0o170000 == 0o120000:
                raise ValueError('Portable payload must contain materialized files, not symlinks')
        source.extractall(destination)
        for entry in source.infolist():
            if not entry.is_dir():
                mode = (entry.external_attr >> 16) & 0o777
                if mode:
                    (destination/entry.filename).chmod(mode)


def extract_windows(archive, destination):
    extract_payload(archive, destination)
    if not (destination/'Eidolon.exe').is_file():
        raise ValueError('Windows payload is missing Eidolon.exe')


def copy_macos_payload(native, output):
    """Nest the deployed payload without materializing symlinks on the host."""
    files = {}
    with zipfile.ZipFile(native) as source:
        paths = {}
        links = set()
        for entry in source.infolist():
            name = entry.filename.rstrip('/')
            path = PurePosixPath(name)
            if (not path.parts or path.is_absolute() or path.as_posix() != name or
                    '..' in path.parts or '\\' in name or ':' in name or path in paths):
                raise ValueError(f'Unsafe macOS payload member: {entry.filename}')
            paths[path] = entry
            if stat.S_ISLNK(entry.external_attr >> 16):
                target = source.read(entry).decode('utf-8')
                relative = PurePosixPath(target)
                if (path.parts[0] != 'Eidolon.app' or not relative.parts or relative.is_absolute() or
                        '..' in relative.parts or '\\' in target or ':' in target):
                    raise ValueError(f'Unsafe macOS payload link: {path} -> {target}')
                links.add(path)
        executable = PurePosixPath('Eidolon.app/Contents/MacOS/Eidolon')
        if executable not in paths or executable in links or paths[executable].is_dir():
            raise ValueError('Mac portable payload is missing Eidolon.app')
        for path, entry in paths.items():
            if links.intersection(path.parents):
                raise ValueError(f'Mac payload member passes through a symlink: {path}')
            member = copy(entry)
            metadata = path.parts[0] == '__MACOSX'
            if metadata:
                # ditto associates AppleDouble entries with the matching payload path.
                member.filename = '__MACOSX/app/' + entry.filename.removeprefix('__MACOSX/')
            else:
                member.filename = 'app/' + entry.filename
            if entry.is_dir():
                output.writestr(member, b'')
                continue
            checksum = hashlib.sha256()
            with source.open(entry) as payload, output.open(member, 'w') as destination:
                for block in iter(lambda: payload.read(1024 * 1024), b''):
                    destination.write(block)
                    checksum.update(block)
            if path not in links and not metadata:
                files[member.filename] = checksum.hexdigest()
    return files


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
        mac_zip = None
        if target == 'windows':
            native = single(native_dir, 'Eidolon-*-windows-x86_64.zip')
            extract_windows(native, app)
            if list(native_dir.glob('Eidolon-*-windows-x86_64-setup.exe')):
                installer = single(native_dir, 'Eidolon-*-windows-x86_64-setup.exe')
                (app/'setup').mkdir()
                shutil.copy2(installer, app/'setup/Eidolon-Setup.exe')
            platform_files = ['Start-Eidolon.cmd', 'Run-Checks.cmd', 'scripts/run-checks.ps1']
        else:
            images = list(native_dir.glob('Eidolon-*-macos-universal.dmg'))
            if images:
                native = single(native_dir, 'Eidolon-*-macos-universal.dmg')
                shutil.copy2(native, app/'Eidolon.dmg')
            else:
                native = single(native_dir, 'Eidolon-*-macos-universal.zip')
                mac_zip = native
            platform_files = ['Start-Eidolon.command', 'Run-Checks.command', 'scripts/run-checks.sh']
        native_manifest = native.with_suffix(native.suffix+'.manifest.json')
        metadata = json.loads(native_manifest.read_text())
        if metadata['application'] != 'Eidolon':
            raise ValueError('Native distribution has the wrong application identity')
        scripts = platform_files + ['scripts/compare-results.py']
        for relative in scripts + ['testing.md', 'edge-cases.md', 'build-validation.md']:
            destination = stage/relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(ROOT/'playtest'/relative, destination)
        shutil.copytree(ROOT/'examples/practice-bank', stage/'problems')
        coverage = make_material(ROOT/'examples/practice-bank', stage)
        revision = subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', 'HEAD'], text=True).strip()
        release = dict(application='Eidolon', platform=target, version=metadata['version'],
                       source_commit=revision, created_utc=datetime.now(timezone.utc).isoformat(),
                       native_artifact=native.name, native_sha256=digest(native),
                       coverage=coverage, build_validation=metadata.get('build_validation', 'See native CI evidence'),
                       tests='Read build_validation; tester startup scripts do not certify numerical correctness.',
                       signing=metadata.get('signing', 'No release signing recorded.'),
                       notarization=metadata.get('notarization', 'No notarization recorded.'))
        (stage/'release-info.json').write_text(json.dumps(release, indent=2)+'\n')
        shutil.copy2(native_manifest, stage/'native-package-manifest.json')
        files = {p.relative_to(stage).as_posix(): digest(p) for p in sorted(stage.rglob('*')) if p.is_file()}
        archive = output_dir/(target+'.zip')
        executables = {name for name in scripts if name.endswith(('.command', '.sh', '.py'))}
        with tempfile.TemporaryDirectory(prefix='eidolon-archive-', dir=output_dir) as archive_dir:
            temporary_archive = Path(archive_dir)/archive.name
            with zipfile.ZipFile(temporary_archive, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as output:
                if mac_zip is not None:
                    # The deployed archive supplies its own directories under app/.
                    app.rmdir()
                    files.update(copy_macos_payload(mac_zip, output))
                # The manifest excludes itself; SHA256SUMS also covers it.
                (stage/'bundle-manifest.json').write_text(json.dumps({'application': 'Eidolon', 'files': files}, indent=2)+'\n')
                files['bundle-manifest.json'] = digest(stage/'bundle-manifest.json')
                (stage/'SHA256SUMS.txt').write_text(''.join(f'{value}  {name}\n' for name, value in sorted(files.items())))
                write_tree(output, stage, executables)
            temporary_archive.replace(archive)
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
