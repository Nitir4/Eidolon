"""Build and verify the per-user installer for the deployed Windows payload."""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
UNINSTALL_KEY = r'Software\Microsoft\Windows\CurrentVersion\Uninstall\TORAClone'


def nsis_literal(value):
    if any(ch in value for ch in '\r\n\0'):
        raise ValueError('Control characters cannot occur in installer literals')
    return value.replace('$', '$$').replace('"', '$\\"')


def payload_commands(stage):
    install, remove, directories = [], [], set()
    for path in sorted(stage.rglob('*')):
        relative = path.relative_to(stage)
        if path.is_symlink():
            raise ValueError(f'Symlinks cannot be installed: {relative}')
        if any(part.lower() in {'reference', 'tora.exe'} or part.startswith('.tora-')
               for part in relative.parts):
            raise ValueError(f'Reference material cannot be installed: {relative}')
        if path.is_dir():
            directories.add(relative)
            continue
        if any(ch in str(relative) for ch in '*?'):
            raise ValueError(f'Wildcards cannot occur in owned paths: {relative}')
        parent = '' if relative.parent == Path('.') else '\\' + relative.parent.as_posix().replace('/', '\\')
        filename = relative.as_posix().replace('/', '\\')
        install += [f'SetOutPath "$INSTDIR{nsis_literal(parent)}"',
                    f'File "/oname={nsis_literal(path.name)}" "{nsis_literal(str(path.resolve()))}"']
        remove.append(f'Delete "$INSTDIR\\{nsis_literal(filename)}"')
    for directory in sorted(directories, key=lambda p: (-len(p.parts), str(p))):
        remove.append('RMDir "$INSTDIR\\' + nsis_literal(directory.as_posix().replace('/', '\\')) + '"')
    return '\n'.join(install), '\n'.join(remove)


def install_command(installer, target):
    # NSIS treats the unquoted final /D= value, including spaces, as a path.
    # This is a CreateProcess command line, never a shell command.
    return subprocess.list2cmdline([str(installer), '/S']) + ' /D=' + str(target)


def compiler_path():
    compiler = shutil.which('makensis')
    if compiler:
        return Path(compiler)
    for root in [os.environ.get('ProgramFiles(x86)'), os.environ.get('ProgramFiles')]:
        if root:
            candidate = Path(root) / 'NSIS/makensis.exe'
            if candidate.is_file():
                return candidate
    raise RuntimeError('NSIS compiler missing; install the pinned packaging tool first')


def build_installer(stage, output, version):
    compiler = compiler_path()
    artifact = output / f'TORA-Clone-{version}-windows-x86_64-setup.exe'
    install, remove = payload_commands(stage)
    template = (ROOT / 'packaging/windows/installer.nsi.in').read_text()
    replacements = {'@OUTPUT@': nsis_literal(str(artifact)), '@VERSION@': nsis_literal(version),
                    '@INSTALL_FILES@': install, '@REMOVE_FILES@': remove}
    for key, value in replacements.items():
        template = template.replace(key, value)
    script = stage.parent / 'tora-installer.nsi'
    script.write_text(template, encoding='utf-8')
    subprocess.run([str(compiler), '/V2', str(script)], check=True, timeout=180)
    if artifact.read_bytes()[:2] != b'MZ':
        raise RuntimeError('Compiler did not produce a Windows executable')
    return artifact


def verify_installer(installer, manifest, environment, report):
    import winreg  # Windows packaging only; portable tests never import this.
    def registered():
        try:
            return winreg.OpenKey(winreg.HKEY_CURRENT_USER, UNINSTALL_KEY, 0,
                                  winreg.KEY_READ | winreg.KEY_WOW64_64KEY)
        except FileNotFoundError:
            return None
    key = registered()
    if key is not None:
        key.Close()
        raise RuntimeError('Refusing to change an existing TORA Clone installation during packaging verification')
    shortcuts = Path(os.environ['APPDATA']) / 'Microsoft/Windows/Start Menu/Programs/TORA Clone'
    shortcut_names = ['TORA Clone.lnk', 'Uninstall TORA Clone.lnk']
    if any((shortcuts / name).exists() for name in shortcut_names):
        raise RuntimeError('Refusing to replace existing shortcuts during packaging verification')
    with tempfile.TemporaryDirectory(prefix='tora-installer-check-') as temporary:
        target = Path(temporary) / 'Native install café'
        target.mkdir()
        retained = [target / 'student-work.txt', target / 'platforms/student-notes.txt']
        retained[1].parent.mkdir()
        for p in retained:
            p.write_bytes(b'Student work must survive installation and removal.\n')
        for _ in range(2):  # Initial install and reinstall preserve unrelated files.
            subprocess.run(install_command(installer, target), check=True, timeout=90)
            for name, expected in manifest['files'].items():
                if hashlib.sha256((target / name).read_bytes()).hexdigest() != expected:
                    raise RuntimeError(f'Installed payload differs: {name}')
        with registered() as key:
            if Path(winreg.QueryValueEx(key, 'InstallLocation')[0]) != target:
                raise RuntimeError('Uninstall registration has the wrong install path')
            if winreg.QueryValueEx(key, 'DisplayVersion')[0] != manifest['version']:
                raise RuntimeError('Uninstall registration has the wrong version')
        for name in shortcut_names:
            if not (shortcuts / name).is_file():
                raise RuntimeError(f'Missing Start menu shortcut: {name}')
        env = environment.copy()
        env['PATH'] = os.pathsep.join([str(target), str(Path(os.environ['SystemRoot']) / 'System32')])
        subprocess.run([str(target / 'ToraClone.exe'), '--smoke-test'], env=env,
                       check=True, cwd=target, timeout=30)
        subprocess.run([str(target / 'Uninstall.exe'), '/S'], check=True, timeout=60)
        deadline = time.monotonic() + 30
        while time.monotonic() < deadline:
            key = registered()
            removed = key is None
            if key is not None:
                key.Close()
            if removed and not (target / 'Uninstall.exe').exists() and not any((shortcuts / name).exists() for name in shortcut_names):
                break
            time.sleep(.1)
        key = registered()
        if key is not None:
            key.Close()
            raise RuntimeError('Uninstall registration remains after removal')
        for name in manifest['files']:
            if (target / name).exists():
                raise RuntimeError(f'Owned file remains after removal: {name}')
        if (target / 'Uninstall.exe').exists():
            raise RuntimeError('Uninstaller did not finish removing itself')
        if set(p for p in target.rglob('*') if p.is_file()) != set(retained):
            raise RuntimeError('Unexpected payload files remain after removal')
        if any((shortcuts / name).exists() for name in shortcut_names):
            raise RuntimeError('Package shortcuts remain after removal')
        for p in retained:
            if p.read_bytes() != b'Student work must survive installation and removal.\n':
                raise RuntimeError('Uninstaller removed unrelated user work')
    result = {'installer': installer.name, 'installed_payload_files': len(manifest['files']),
              'install_and_reinstall': 'passed', 'installed_startup_without_sdk': 'passed',
              'unicode_and_spaces_path': 'passed', 'start_menu_and_registry': 'passed',
              'uninstall_owned_files': 'passed', 'unrelated_student_files_preserved': 'passed'}
    report.write_text(json.dumps(result, indent=2) + '\n')
    return result
