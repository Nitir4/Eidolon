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
import plistlib
import re
import shutil
import subprocess
import sys
import tempfile
import urllib.request
from windows_installer import build_installer, verify_installer
from linux_runtime import stage_wayland_plugins
from macos_bundle import macos_frameworks
from runtime_notices import copy_qt_notices, copy_distribution_qt_notices

ROOT = Path(__file__).resolve().parents[1]


def source_provenance(root=ROOT):
    """Identify local source without presenting uncommitted work as a HEAD release."""
    head = subprocess.run(['git', '-C', str(root), 'rev-parse', 'HEAD'],
                          capture_output=True, text=True)
    if head.returncode:
        return {'base_commit': None, 'dirty': None, 'repository_tree_sha256': None}
    status = subprocess.check_output(['git', '-C', str(root), 'status', '--porcelain',
                                      '--untracked-files=all'], text=True)
    paths = subprocess.check_output(['git', '-C', str(root), 'ls-files', '-z',
                                     '--cached', '--others', '--exclude-standard'])
    tree = hashlib.sha256()
    for relative in sorted(set(paths.split(b'\0')) - {b''}):
        path = root / os.fsdecode(relative)
        if path.is_symlink():
            content = b'symlink\0' + os.fsencode(os.readlink(path))
        elif path.is_file():
            content = path.read_bytes()
        else:
            continue  # Deleted tracked files have no working-tree payload.
        tree.update(relative + b'\0' + hashlib.sha256(content).digest())
    return {'base_commit': head.stdout.strip(), 'dirty': bool(status),
            'repository_tree_sha256': tree.hexdigest(),
            'snapshot_kind': 'working tree observed during packaging, not build input attestation',
            'identity': 'local working tree' if status else 'clean repository checkout'}


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
    version = subprocess.check_output([str(sdk_tool(qt_root, 'qmake')),
                                       '-query', 'QT_VERSION'], text=True).strip()
    copy_qt_notices(stage, version)
    if sys.platform.startswith('linux'):
        copy_distribution_qt_notices(stage, qt_root, version)
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
    data = {'application': 'Eidolon', 'version': version, 'qt_version': qt_version,
            'qt_legal_bundle': 'licenses/Qt-' + qt_version
                               if (stage / 'licenses' / ('Qt-' + qt_version) / 'SOURCES.json').is_file() else None,
            'qt_distribution_notices': 'licenses/Qt-distribution'
                                       if (stage / 'licenses/Qt-distribution/PROVENANCE.json').is_file() else None,
            'platform': target,
            'stage': 'Native full application: Linear Equations and Matrix Inversion; Linear Programming and Cj/RHS/Dual Revised extensions; Transportation; Integer Programming; Networks; Project Planning; Queuing; Zero-Sum Games; Assignment extension. Local validation does not establish exhaustive original parity or cross-platform release readiness.',
            'source': source_provenance(),
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


def smoke_application(executable, env, cwd):
    # Each family builds its input widgets lazily. Check those factories using
    # the deployed runtime, beyond simply constructing the MAIN window.
    for screen in (None, 'lp/input-mode', 'transportation/input-mode',
                   'assignment/input', 'games/input', 'equations/input',
                   'integer/input', 'networks/input', 'planning/input', 'queuing/input'):
        arguments = [executable, '--smoke-test']
        if screen:
            arguments += ['--screen', screen]
        run(arguments, env=env, timeout=45, cwd=cwd)


def windows_package(build, stage, qt_root, config, output, version):
    run(['cmake', '--install', build, '--config', config, '--prefix', stage])
    executable = stage / 'Eidolon.exe'
    (stage / 'bin/Eidolon.exe').replace(executable)
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
    smoke_application(executable, env, stage)
    name = f'Eidolon-{version}-windows-x86_64'
    archive = Path(shutil.make_archive(str(output / name), 'zip', root_dir=stage))
    installer = build_installer(stage, output, version)
    verification = verify_installer(installer, data, env, output / (installer.name + '.verification.json'))
    installer_data = {**data, 'kind': 'per-user-installer', 'verification': verification}
    output.joinpath(installer.name + '.sha256').write_text(f'{digest(installer)}  {installer.name}\n')
    output.joinpath(installer.name + '.manifest.json').write_text(json.dumps(installer_data, indent=2) + '\n')
    return archive, data


def verify_macos_bundle(bundle):
    list(macos_frameworks(bundle))
    run(['codesign', '--verify', '--deep', '--strict', '--all-architectures', bundle])


def notarize(path, profile):
    try:
        result = run(['xcrun', 'notarytool', 'submit', path, '--keychain-profile', profile,
                      '--wait', '--output-format', 'json'], capture_output=True, text=True)
    except subprocess.CalledProcessError as error:
        raise RuntimeError(f'Apple notarization failed for {path.name}: {error.stderr or error.stdout}') from error
    submission = json.loads(result.stdout)
    if submission['status'] != 'Accepted':
        raise RuntimeError(f"Apple notarization returned {submission['status']} for {submission['id']}")


def macos_package(build, stage, qt_root, config, output, version):
    identity = os.environ.get('SIGNING_IDENTITY') or '-'
    notary_profile = os.environ.get('NOTARY_PROFILE')
    if notary_profile and identity == '-':
        raise RuntimeError('Notarization requires a Developer ID Application signing identity')
    run(['cmake', '--install', build, '--config', config, '--prefix', stage])
    bundle = stage / 'Eidolon.app'
    executable = bundle / 'Contents/MacOS/Eidolon'
    plugins = bundle / 'Contents/PlugIns/platforms'
    plugins.mkdir(parents=True, exist_ok=True)
    shutil.copy2(qt_root / 'plugins/platforms/libqoffscreen.dylib', plugins)
    run([sdk_tool(qt_root, 'macdeployqt'), bundle, '-always-overwrite'])
    frameworks = list(macos_frameworks(bundle))
    libraries = sorted(path for path in bundle.rglob('*.dylib') if not path.is_symlink())
    for binary in [executable, *libraries, *(binary for _, binary in frameworks)]:
        run(['lipo', binary, '-verify_arch', 'arm64', 'x86_64'])
    sign = ['codesign', '--force', '--sign', identity]
    sign += ['--timestamp=none'] if identity == '-' else ['--options', 'runtime', '--timestamp']
    for item in [*libraries, *(framework for framework, _ in frameworks), bundle]:
        run([*sign, item])
    if notary_profile:
        with tempfile.TemporaryDirectory(prefix='eidolon-notary-') as temporary:
            submission = Path(temporary) / 'Eidolon.zip'
            run(['ditto', '-c', '-k', '--sequesterRsrc', '--keepParent', bundle, submission])
            notarize(submission, notary_profile)
        run(['xcrun', 'stapler', 'staple', bundle])
        run(['xcrun', 'stapler', 'validate', bundle])
    verify_macos_bundle(bundle)
    notices(stage, qt_root)
    data = manifest(stage, qt_root, version, 'macos-universal')
    data['signing'] = 'Ad hoc' if identity == '-' else 'Signed with a distribution identity'
    data['notarization'] = 'Apple notarized and stapled' if notary_profile else 'Not notarized'
    data['symlinks'] = {path.relative_to(stage).as_posix(): path.readlink().as_posix()
                        for path in sorted(bundle.rglob('*')) if path.is_symlink()}
    for name in data['symlinks']:
        data['files'].pop(name, None)
    (stage / 'package-manifest.json').write_text(json.dumps(data, indent=2) + '\n')

    archive = output / f'Eidolon-{version}-macos-universal.zip'
    run(['ditto', '-c', '-k', '--sequesterRsrc', stage, archive])
    with tempfile.TemporaryDirectory(prefix='eidolon-zip-check-') as temporary:
        extracted = Path(temporary)
        run(['ditto', '-x', '-k', archive, extracted])
        verify_macos_bundle(extracted / bundle.name)
        smoke_application(extracted / executable.relative_to(stage), smoke_env(), extracted)
    (stage / 'Applications').symlink_to('/Applications', target_is_directory=True)
    artifact = output / f'Eidolon-{version}-macos-universal.dmg'
    run(['hdiutil', 'create', '-volname', 'Eidolon', '-srcfolder', stage,
         '-ov', '-format', 'UDZO', artifact])
    if identity != '-':
        with (bundle / 'Contents/Info.plist').open('rb') as info:
            identifier = plistlib.load(info)['CFBundleIdentifier']
        run(['codesign', '--sign', identity, '--timestamp', '--identifier', identifier + '.dmg', artifact])
    if notary_profile:
        notarize(artifact, notary_profile)
        run(['xcrun', 'stapler', 'staple', artifact])
        run(['xcrun', 'stapler', 'validate', artifact])
    run(['hdiutil', 'verify', artifact])
    with tempfile.TemporaryDirectory(prefix='eidolon-dmg-check-') as temporary:
        mount = Path(temporary)
        run(['hdiutil', 'attach', '-readonly', '-nobrowse', '-mountpoint', mount, artifact])
        try:
            verify_macos_bundle(mount / bundle.name)
            if (mount / 'Applications').readlink() != Path('/Applications'):
                raise RuntimeError('The disk image is missing its Applications shortcut')
        finally:
            run(['hdiutil', 'detach', mount])
    return (artifact, archive), data


def linux_package(build, stage, qt_root, config, output, version, tool_dir):
    if platform.machine() not in ('x86_64', 'AMD64'):
        raise RuntimeError('The Linux artifact currently targets x86_64')
    appdir = stage / 'AppDir'
    run(['cmake', '--install', build, '--config', config, '--prefix', appdir / 'usr'])
    executable = appdir / 'usr/bin/Eidolon'
    # A root-level executable symlink can bypass the adjacent Qt configuration
    # with distribution Qt builds. Keep argv[0] at the actual usr/bin entry.
    apprun = appdir / 'AppRun'
    shutil.copy2(ROOT / 'packaging/linux-apprun.sh', apprun)
    apprun.chmod(0o755)
    plugins = Path(subprocess.check_output([str(sdk_tool(qt_root, 'qmake')),
                                           '-query', 'QT_INSTALL_PLUGINS'], text=True).strip())
    platforms, integrations = stage_wayland_plugins(plugins, appdir)
    env = os.environ.copy()
    # The installed executable no longer has its build RPATH. Let deployment
    # inspect the selected SDK even when Qt is absent from the system loader.
    libraries = subprocess.check_output([str(sdk_tool(qt_root, 'qmake')),
                                         '-query', 'QT_INSTALL_LIBS'], text=True).strip()
    env['LD_LIBRARY_PATH'] = libraries + (
        os.pathsep + env['LD_LIBRARY_PATH'] if env.get('LD_LIBRARY_PATH') else '')
    env.update({'APPIMAGE_EXTRACT_AND_RUN': '1', 'QMAKE': str(sdk_tool(qt_root, 'qmake')),
                'OUTPUT': str(output / f'Eidolon-{version}-linux-x86_64.AppImage'),
                'EXTRA_PLATFORM_PLUGINS': platforms,
                # linuxdeploy's bundled strip predates modern RELR sections.
                # Preserve symbols instead of modifying incompatible files.
                'NO_STRIP': '1', 'ARCH': 'x86_64', 'VERSION': version,
                'LDAI_RUNTIME_FILE': str(tool_dir / 'runtime-x86_64')})
    notices(appdir, qt_root)
    dependencies = [argument for plugin in integrations
                    for argument in ['--deploy-deps-only', plugin]]
    run([linux_tools(tool_dir), '--appdir', appdir, '--executable', executable, *dependencies,
         '--desktop-file', ROOT / 'packaging/resources/eidolon.desktop',
         '--icon-file', ROOT / 'packaging/resources/eidolon.svg',
         '--plugin', 'qt', '--output', 'appimage'], env=env, cwd=stage)
    # Read final dependency hashes after deployment has finished patching ELF
    # paths. The manifest is distributed alongside the final AppImage.
    data = manifest(appdir, qt_root, version, 'linux-x86_64')
    artifact = Path(env['OUTPUT'])
    if not artifact.is_file():
        raise RuntimeError(f'AppImage tool did not create {artifact}')
    env = smoke_env()
    env['APPIMAGE_EXTRACT_AND_RUN'] = '1'
    smoke_application(artifact, env, stage)
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
            with tempfile.TemporaryDirectory(prefix='eidolon-macos-', dir=output) as temporary_output:
                packaged, data = macos_package(build, stage, qt_root, args.config, Path(temporary_output), version)
                artifacts = tuple(path.replace(output / path.name) for path in packaged)
        elif sys.platform.startswith('linux'):
            artifact, data = linux_package(build, stage, qt_root, args.config, output, version, args.tool_dir.resolve())
        else:
            raise RuntimeError(f'Unsupported packaging OS: {sys.platform}')
    if sys.platform != 'darwin':
        artifacts = (artifact,)
    for artifact in artifacts:
        output.joinpath(artifact.name + '.sha256').write_text(f'{digest(artifact)}  {artifact.name}\n')
        output.joinpath(artifact.name + '.manifest.json').write_text(json.dumps(data, indent=2) + '\n')
        print(f'Packaged and startup-checked: {artifact}')


if __name__ == '__main__':
    main()
