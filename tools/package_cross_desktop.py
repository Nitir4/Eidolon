#!/usr/bin/env python3
"""Stage real Linux cross-built executables; never claim native macOS validation."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

from package_desktop import source_provenance
from runtime_notices import copy_qt_notices

ROOT = Path(__file__).resolve().parents[1]
WINDOWS_SYSTEM = set('''advapi32 authz bcrypt cfgmgr32 comctl32 comdlg32 crypt32 d3d9 d3d11
 d3d12 d3dcompiler_47 dcomp dnsapi dwmapi dwrite dxgi gdi32 imm32 iphlpapi kernel32
 mpr msvcrt netapi32 normaliz ntdll ole32 oleaut32 opengl32 powrprof propsys
 oleacc psapi rpcrt4 secur32 setupapi shcore shell32 shlwapi synchronization ucrtbase user32 userenv
 usp10 uxtheme version winhttp wininet winmm winspool wintrust ws2_32 wtsapi32'''.split())


def output(*args):
    return subprocess.check_output(list(map(str, args)), text=True)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def plugins(sdk, stage, extension, platform):
    selected = ['platforms/' + platform, 'platforms/' + ('qoffscreen.dll' if extension == '.dll' else 'libqoffscreen.dylib')]
    for directory in ['styles', 'imageformats']:
        selected += [p.relative_to(sdk/'plugins').as_posix() for p in (sdk/'plugins'/directory).glob('*'+extension)]
    for relative in selected:
        source = sdk/'plugins'/relative
        if not source.is_file():
            raise ValueError('Missing plugin: ' + str(source))
        destination = stage/'plugins'/relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, destination)


def stage_windows(build, sdk, stage, compiler):
    shutil.copy2(build/'src/desktop/Eidolon.exe', stage/'Eidolon.exe')
    plugins(sdk, stage, '.dll', 'qwindows.dll')
    (stage/'qt.conf').write_text('[Paths]\nPlugins=plugins\n')
    queue = list(stage.rglob('*.dll')) + [stage/'Eidolon.exe']
    checked = set()
    report = {}
    unresolved = set()
    while queue:
        path = queue.pop()
        if path in checked:
            continue
        checked.add(path)
        header = output('x86_64-w64-mingw32-objdump', '-f', path)
        if 'pei-x86-64' not in header:
            raise ValueError('Wrong PE architecture: ' + str(path))
        dependencies = re.findall(r'DLL Name:\s*(\S+)', output('x86_64-w64-mingw32-objdump', '-p', path))
        report[path.relative_to(stage).as_posix()] = dependencies
        for name in dependencies:
            destination = stage/name
            if destination.is_file():
                queue.append(destination)
                continue
            candidates = [p for p in (sdk/'bin').glob('*.dll') if p.name.lower() == name.lower()]
            runtime = Path(output(compiler, '-print-file-name='+name).strip())
            if not candidates and runtime.is_file():
                candidates = [runtime]
            if candidates:
                shutil.copy2(candidates[0], destination)
                queue.append(destination)
            elif Path(name).stem.lower() not in WINDOWS_SYSTEM and not name.lower().startswith(('api-ms-', 'ext-ms-')):
                unresolved.add(name)
    if unresolved:
        raise ValueError('Unresolved Windows dependencies: ' + ', '.join(sorted(unresolved)))
    # Runtime notices accompany the matching distro compiler DLLs.
    notices = stage/'licenses/MinGW-runtime'
    notices.mkdir(parents=True)
    for folder in ['/usr/share/doc/gcc-mingw-w64-base', '/usr/share/doc/mingw-w64-common']:
        source = Path(folder)/'copyright'
        if source.is_file():
            shutil.copy2(source, notices/(Path(folder).name+'-copyright'))
    return report


def mach_dependencies(path):
    text = output('llvm-objdump-18', '--macho', '--dylibs-used', '--arch=arm64', path)
    return re.findall(r'^\s+(\S+) \(compatibility version', text, re.M)


def stage_macos(build, sdk, stage):
    bundle = stage/'Eidolon.app'
    shutil.copytree(build/'src/desktop/Eidolon.app', bundle)
    contents = bundle/'Contents'
    plugins(sdk, contents, '.dylib', 'libqcocoa.dylib')
    # Qt uses Contents/PlugIns for app bundles; match qt.conf explicitly.
    (contents/'plugins').rename(contents/'PlugIns')
    (contents/'Resources').mkdir(exist_ok=True)
    (contents/'Resources/qt.conf').write_text('[Paths]\nPlugins=PlugIns\n')
    queue = [contents/'MacOS/Eidolon'] + list((contents/'PlugIns').rglob('*.dylib'))
    report = {}
    while queue:
        path = queue.pop()
        relative = path.relative_to(stage).as_posix()
        if relative in report:
            continue
        output('llvm-lipo-18', path, '-verify_arch', 'arm64', 'x86_64')
        dependencies = mach_dependencies(path)
        report[relative] = dependencies
        for dependency in dependencies:
            if dependency.startswith(('/System/Library/', '/usr/lib/')):
                continue
            if not dependency.startswith('@rpath/Qt') or '.framework/' not in dependency:
                raise ValueError('Unexpected macOS dependency: '+dependency)
            suffix = dependency.removeprefix('@rpath/')
            target = contents/'Frameworks'/suffix
            if not target.is_file():
                framework = suffix.split('/')[0]
                shutil.copytree(sdk/'lib'/framework, contents/'Frameworks'/framework,
                                symlinks=False, ignore=shutil.ignore_patterns('Headers', 'Modules', '*.prl'))
            if target != path:
                queue.append(target)
    # Verify the executable finds the bundled frameworks, never the local SDK.
    loads = output('llvm-objdump-18', '--macho', '--private-headers', contents/'MacOS/Eidolon')
    if '@executable_path/../Frameworks' not in loads:
        raise ValueError('App-relative Frameworks rpath missing')
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--platform', choices=['windows', 'macos'], required=True)
    parser.add_argument('--build-dir', type=Path, required=True)
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--output-dir', type=Path, required=True)
    parser.add_argument('--compiler', default='x86_64-w64-mingw32-g++')
    args = parser.parse_args()
    configuration = (args.sdk/'mkspecs/qconfig.pri').read_text()
    if not re.search(r'^QT_VERSION\s*=\s*6\.10\.3\s*$', configuration, re.M):
        parser.error('This delivery and its verified notices require the Qt6.10.3 SDK')
    destination = args.output_dir.resolve()
    destination.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='eidolon-cross-stage-') as temporary:
        stage = Path(temporary)
        function = stage_windows if args.platform == 'windows' else stage_macos
        arguments = [args.build_dir.resolve(), args.sdk.resolve(), stage]
        if args.platform == 'windows':
            arguments.append(args.compiler)
        dependencies = function(*arguments)
        copy_qt_notices(stage, '6.10.3')
        shutil.copy2(ROOT/'packaging/THIRD_PARTY_NOTICES.md', stage)
        metadata = dict(application='Eidolon', version='0.1.0', qt_version='6.10.3',
                        platform=args.platform, source=source_provenance(),
                        build_validation='Linux cross-build; dependency/architecture checks only here. See delivery evidence for Wine tests. No native Windows/macOS execution.',
                        minimum_os='Windows 10 1809 x64' if args.platform=='windows' else 'macOS 13.3',
                        dependencies=dependencies,
                        files={p.relative_to(stage).as_posix():digest(p) for p in sorted(stage.rglob('*')) if p.is_file()})
        (stage/'package-manifest.json').write_text(json.dumps(metadata, indent=2)+'\n')
        label = 'windows-x86_64' if args.platform == 'windows' else 'macos-universal'
        archive = Path(shutil.make_archive(str(destination/('Eidolon-0.1.0-'+label)), 'zip', root_dir=stage))
        archive.with_suffix('.zip.manifest.json').write_text(json.dumps(metadata, indent=2)+'\n')
        archive.with_suffix('.zip.sha256').write_text(digest(archive)+'  '+archive.name+'\n')
        print('Cross-built payload:', archive)
        print('Audited executable/library dependency records:', len(dependencies))


if __name__ == '__main__':
    main()
