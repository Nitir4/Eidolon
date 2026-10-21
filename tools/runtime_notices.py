"""Copy verified upstream Qt legal documents for the exact pinned SDK version."""
import hashlib
import json
from pathlib import Path, PurePosixPath
import shutil

QT_BUNDLE = Path(__file__).resolve().parents[1] / 'packaging/licenses/Qt-6.10.3'


def validate_qt_bundle(bundle):
    data = json.loads((bundle / 'SOURCES.json').read_text())
    for record in data['files']:
        relative = PurePosixPath(record['path'])
        if relative.is_absolute() or '..' in relative.parts or ':' in str(relative):
            raise ValueError('Unsafe notice path')
        path = bundle / relative
        if not path.resolve().is_relative_to(bundle.resolve()):
            raise ValueError('Notice path escapes bundle')
        if hashlib.sha256(path.read_bytes()).hexdigest() != record['sha256']:
            raise ValueError('Notice hash differs: ' + str(relative))
    return data


def copy_qt_notices(stage, qt_version):
    data = json.loads((QT_BUNDLE / 'SOURCES.json').read_text())
    if qt_version != data['qt_version']:
        return False
    validate_qt_bundle(QT_BUNDLE)
    shutil.copytree(QT_BUNDLE, stage / 'licenses' / ('Qt-' + qt_version), dirs_exist_ok=True)
    return True


def copy_distribution_qt_notices(stage, qt_root, qt_version, prefix=Path('/usr')):
    """Preserve installed distribution notices when packaging its system Qt SDK.

    These are available license texts, not a substitute for an upstream source
    attribution bundle. Never associate the host's notices with a separate SDK.
    """
    if qt_root.resolve() != prefix.resolve():
        return None
    sources = [prefix / 'share/licenses' / name
               for name in ('qt6-base', 'qt6-wayland', 'qt6-svg')]
    sources = [path for path in sources if path.is_dir()]
    if not sources:
        return None
    destination = stage / 'licenses/Qt-distribution'
    records = []
    for source in sources:
        target = destination / source.name
        shutil.copytree(source, target, dirs_exist_ok=True)
        records.append({'installed_path': str(source), 'resolved_path': str(source.resolve())})
    files = {str(path.relative_to(destination)): hashlib.sha256(path.read_bytes()).hexdigest()
             for path in sorted(destination.rglob('*')) if path.is_file()}
    data = {'qt_version': qt_version, 'source': 'installed distribution license directories',
            'complete_upstream_attribution_bundle': False, 'directories': records, 'files': files}
    (destination / 'PROVENANCE.json').write_text(json.dumps(data, indent=2) + '\n')
    return data
