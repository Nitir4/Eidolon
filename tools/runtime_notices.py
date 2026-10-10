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
