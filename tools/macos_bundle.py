"""Validate the versioned framework layout used by the macOS app."""

import os
from pathlib import Path


def macos_frameworks(bundle):
    bundle = bundle.resolve(strict=True)
    frameworks = sorted((bundle / 'Contents/Frameworks').glob('*.framework'))
    if not frameworks:
        raise ValueError(f'No Qt frameworks found in {bundle}')
    for path in bundle.rglob('*'):
        if path.is_symlink():
            try:
                target = path.resolve(strict=True)
            except (OSError, RuntimeError) as error:
                raise ValueError(f'Broken app symlink: {path}') from error
            if not target.is_relative_to(bundle):
                raise ValueError(f'App symlink points outside the bundle: {path}')
    for framework in frameworks:
        current = framework / 'Versions/Current'
        if not current.is_symlink():
            raise ValueError(f'Missing framework version symlink: {current}')
        version = Path(os.readlink(current))
        if len(version.parts) != 1 or version.name == '..' or not current.is_dir():
            raise ValueError(f'Invalid framework version symlink: {current}')
        for entry in framework.iterdir():
            if entry.name == 'Versions':
                continue
            if not entry.is_symlink() or os.readlink(entry) != f'Versions/Current/{entry.name}':
                raise ValueError(f'Invalid top-level framework entry: {entry}')
        binary = framework / framework.stem
        if not binary.is_symlink() or not binary.is_file():
            raise ValueError(f'Missing framework executable symlink: {binary}')
        if not (framework / 'Resources/Info.plist').is_file():
            raise ValueError(f'Missing framework property list: {framework}')
        yield framework, binary.resolve(strict=True)
