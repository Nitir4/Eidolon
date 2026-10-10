"""Stage client-side Qt Wayland integrations before ELF dependency deployment."""
from pathlib import Path
import shutil


def stage_wayland_plugins(qt_plugins: Path, appdir: Path):
    platforms = sorted((qt_plugins / 'platforms').glob('libqwayland*.so'))
    if not platforms:
        raise RuntimeError('Qt Wayland platform missing; install the qtwayland SDK archive')
    shell = qt_plugins / 'wayland-shell-integration'
    if not (shell / 'libxdg-shell.so').is_file():
        raise RuntimeError('Qt Wayland xdg-shell integration missing from SDK')
    dependencies = []
    for group in ['wayland-shell-integration', 'wayland-decoration-client',
                  'wayland-graphics-integration-client']:
        for source in sorted((qt_plugins / group).glob('*.so')):
            target = appdir / 'usr/plugins' / group / source.name
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, target)
            dependencies.append(target)
    return ';'.join(['libqoffscreen.so'] + [p.name for p in platforms]), dependencies
