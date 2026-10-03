#!/usr/bin/env python3
"""Package only CMake-installed application files and native runtime dependencies.

OS-specific deployment stays here. No reference capture or TORA binary is
read or copied. Requires an already-built desktop target and Qt SDK.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys
import tempfile
import urllib.request

ROOT = Path(__file__).resolve().parents[1]


def run(args, **kwargs):
    print('+', ' '.join(map(str, args)), flush=True)
    return subprocess.run([str(arg) for arg in args], check=True, **kwargs)


def digest(path):
    with path.open('rb') as handle:
        return hashlib.file_digest(handle, 'sha256').hexdigest()


def sdk_tool(qt_root, name):
    suffix = '.exe' if sys.platform == 'win32' else ''
    if name == 'qmake':
        for candidate in ('qmake6', 'qmake'):
            path = qt_root / 'bin' / (candidate + suffix)
            if path.is_file():
                version = subprocess.check_output([str(path), '-query', 'QT_VERSION'], text=True).strip()
                if tuple(map(int, version.split('.')[:2])) >= (6, 8):
                    return path
        raise RuntimeError(f'No Qt 6.8+ qmake found under {qt_root / "bin"}')
    path = qt_root / 'bin' / (name + suffix)
    if not path.is_file():
        raise RuntimeError(f'Missing Qt SDK tool: {path}')
    return path


def linux_tools(directory):
    directory.mkdir(parents=True, exist_ok=True)
    pins = json.loads((ROOT / 'packaging/linux-tools.json').read_text())
    for name, data in pins.items():
        path = directory / name
        if not path.is_file() or digest(path) != data['sha256']:
            temporary = path.with_suffix('.download')
            with urllib.request.urlopen(data['url'], timeout=120) as response, temporary.open('wb') as output:
                shutil.copyfileobj(response, output)
            if digest(temporary) != data['sha256']:
                raise RuntimeError(f'{name} checksum differs; review/update packaging/linux-tools.json')
            temporary.replace(path)
        path.chmod(path.stat().st_mode | 0o111)
    return directory / 'linuxdeploy-x86_64.AppImage'


def notices(stage, qt_root):
    shutil.copy2(ROOT / 'packaging/THIRD_PARTY_NOTICES.md', stage)
    for name in ('Licenses', 'LICENSES'):
        source = qt_root / name
        if source.is_dir():
            shutil.copytree(source, stage / 'licenses' / 'Qt', dirs_exist_ok=True)


def manifest(stage, qt_root, version, target):
    for path in stage.rglob('*'):
        relative = path.relative_to(stage)
        if any(part.lower() in {'reference', 'tora.exe'} or part.startswith('.tora-')
               for part in relative.parts):
            raise RuntimeError(f'Forbidden reference material in package: {relative}')
    qmake = sdk_tool(qt_root, 'qmake')
    qt_version = subprocess.check_output([str(qmake), '-query', 'QT_VERSION'], text=True).strip()
    data = {'application': 'ToraClone', 'version': version, 'qt_version': qt_version,
            'platform': target, 'stage': 'Foundation 1 bootstrap',
            'files': {str(p.relative_to(stage)).replace('\\', '/'): digest(p)
                      for p in sorted(stage.rglob('*')) if p.is_file()}}
    (stage / 'package-manifest.json').write_text(json.dumps(data, indent=2) + '\n')
    return data


def smoke_env():
    env = os.environ.copy()
    # A deployed executable must find its own SDK/runtime, not the build SDK.
    for key in ('QT_PLUGIN_PATH', 'QT_QPA_PLATFORM_PLUGIN_PATH', 'QML2_IMPORT_PATH',
                'QML_IMPORT_PATH', 'LD_LIBRARY_PATH', 'DYLD_LIBRARY_PATH',
                'DYLD_FRAMEWORK_PATH'):
        env.pop(key, None)
    env['QT_QPA_PLATFORM'] = 'offscreen'
    return env


def windows_package(build, stage, qt_root, config, output, version):
    run(['cmake', '--install', build, '--config', config, '--prefix', stage])
    executable = stage / 'ToraClone.exe'
    (stage / 'bin/ToraClone.exe').replace(executable)
    (stage / 'bin').rmdir()
    run([sdk_tool(qt_root, 'windeployqt'), '--release', '--no-translations', executable])
    # The offscreen backend is used solely by the deployed startup check.
    plugin = qt_root / 'plugins/platforms/qoffscreen.dll'
    (stage / 'platforms').mkdir(exist_ok=True)
    shutil.copy2(plugin, stage / 'platforms/qoffscreen.dll')
    # windeployqt's MSVC redistributable installer alone is insufficient for a
    # portable folder. Include the matching app-local CRT DLLs from the SDK.
    redist = os.environ.get('VCToolsRedistDir')
    if not redist:
        raise RuntimeError('Run inside the MSVC developer environment (VCToolsRedistDir is missing)')
    crt = list((Path(redist) / 'x64').glob('Microsoft.VC*.CRT/*.dll'))
    if not crt:
        raise RuntimeError('MSVC x64 app-local CRT DLLs were not found')
    for path in crt:
        shutil.copy2(path, stage / path.name)
    notices(stage, qt_root)
    data = manifest(stage, qt_root, version, 'windows-x86_64')
    env = smoke_env()
    env['PATH'] = os.pathsep.join([str(stage), str(Path(os.environ['SystemRoot']) / 'System32')])
    run([executable, '--smoke-test'], env=env, timeout=30, cwd=stage)
    name = f'TORA-Clone-{version}-windows-x86_64'
    archive = Path(shutil.make_archive(str(output / name), 'zip', root_dir=stage))
    return archive, data


def macos_package(build, stage, qt_root, config, output, version):
    run(['cmake', '--install', build, '--config', config, '--prefix', stage])
    bundle = stage / 'ToraClone.app'
    executable = bundle / 'Contents/MacOS/ToraClone'
    plugins = bundle / 'Contents/PlugIns/platforms'
    plugins.mkdir(parents=True, exist_ok=True)
    shutil.copy2(qt_root / 'plugins/platforms/libqoffscreen.dylib', plugins)
    run([sdk_tool(qt_root, 'macdeployqt'), bundle, '-always-overwrite'])
    run(['lipo', '-verify_arch', 'arm64', 'x86_64', executable])
    frameworks = bundle / 'Contents/Frameworks'
    for framework in frameworks.glob('*.framework'):
        binary = framework / 'Versions/A' / framework.stem
        if binary.is_file():
            run(['lipo', '-verify_arch', 'arm64', 'x86_64', binary])
    notices(stage, qt_root)
    data = manifest(stage, qt_root, version, 'macos-universal')
    run([executable, '--smoke-test'], env=smoke_env(), timeout=30)
    (stage / 'Applications').symlink_to('/Applications', target_is_directory=True)
    artifact = output / f'TORA-Clone-{version}-macos-universal.dmg'
    run(['hdiutil', 'create', '-volname', 'TORA Clone', '-srcfolder', stage,
         '-ov', '-format', 'UDZO', artifact])
    return artifact, data


def linux_package(build, stage, qt_root, config, output, version, tool_dir):
    if platform.machine() not in ('x86_64', 'AMD64'):
        raise RuntimeError('The Linux artifact currently targets x86_64')
    appdir = stage / 'AppDir'
    run(['cmake', '--install', build, '--config', config, '--prefix', appdir / 'usr'])
    executable = appdir / 'usr/bin/ToraClone'
    env = os.environ.copy()
    env.update({'APPIMAGE_EXTRACT_AND_RUN': '1', 'QMAKE': str(sdk_tool(qt_root, 'qmake')),
                'OUTPUT': str(output / f'TORA-Clone-{version}-linux-x86_64.AppImage'),
                'EXTRA_PLATFORM_PLUGINS': 'libqoffscreen.so',
                # linuxdeploy's bundled strip predates modern RELR sections.
                # Preserve symbols instead of modifying incompatible files.
                'NO_STRIP': '1', 'ARCH': 'x86_64', 'VERSION': version})
    notices(appdir, qt_root)
    run([linux_tools(tool_dir), '--appdir', appdir, '--executable', executable,
         '--desktop-file', ROOT / 'packaging/resources/tora-clone.desktop',
         '--icon-file', ROOT / 'packaging/resources/tora-clone.svg',
         '--plugin', 'qt', '--output', 'appimage'], env=env, cwd=stage)
    # Read final dependency hashes after deployment has finished patching ELF
    # paths. The manifest is distributed alongside the final AppImage.
    data = manifest(appdir, qt_root, version, 'linux-x86_64')
    artifact = Path(env['OUTPUT'])
    if not artifact.is_file():
        raise RuntimeError(f'AppImage tool did not create {artifact}')
    env = smoke_env()
    env['APPIMAGE_EXTRACT_AND_RUN'] = '1'
    run([artifact, '--smoke-test'], env=env, timeout=45, cwd=stage)
    return artifact, data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir', type=Path, default=ROOT / 'build/ci')
    parser.add_argument('--output-dir', type=Path, default=ROOT / 'dist')
    parser.add_argument('--config', default='Release')
    parser.add_argument('--qt-root', type=Path, default=os.environ.get('QT_ROOT_DIR'))
    parser.add_argument('--tool-dir', type=Path, default=ROOT / '.packaging-tools')
    args = parser.parse_args()
    if args.qt_root is None:
        parser.error('--qt-root or QT_ROOT_DIR is required')
    build, output, qt_root = args.build_dir.resolve(), args.output_dir.resolve(), args.qt_root.resolve()
    metadata = (build / 'generated/include/tora/build_info.hpp').read_text()
    match = re.search(r'version\s*=\s*"([^"]+)"', metadata)
    if not match:
        raise RuntimeError('Missing configured build version')
    version = match.group(1)
    output.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='tora-package-', dir=build) as temporary:
        stage = Path(temporary)
        if sys.platform == 'win32':
            artifact, data = windows_package(build, stage, qt_root, args.config, output, version)
        elif sys.platform == 'darwin':
            artifact, data = macos_package(build, stage, qt_root, args.config, output, version)
        elif sys.platform.startswith('linux'):
            artifact, data = linux_package(build, stage, qt_root, args.config, output, version, args.tool_dir.resolve())
        else:
            raise RuntimeError(f'Unsupported packaging OS: {sys.platform}')
    output.joinpath(artifact.name + '.sha256').write_text(f'{digest(artifact)}  {artifact.name}\n')
    output.joinpath(artifact.name + '.manifest.json').write_text(json.dumps(data, indent=2) + '\n')
    print(f'Packaged and startup-checked: {artifact}')


if __name__ == '__main__':
    main()
