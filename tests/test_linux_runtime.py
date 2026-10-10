"""Portable regression coverage for the native Linux runtime payload."""
import sys
import tempfile
from pathlib import Path
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from linux_runtime import stage_wayland_plugins


class WaylandPayload(unittest.TestCase):
    def sdk(self, root, names):
        for name in names:
            path = root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(name.encode())

    def test_modern_platform_and_client_integrations(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            sdk = root / 'sdk'
            self.sdk(sdk, ['platforms/libqwayland.so',
                           'wayland-shell-integration/libxdg-shell.so',
                           'wayland-decoration-client/libbradient.so',
                           'wayland-graphics-integration-client/libqt-plugin.so',
                           'wayland-graphics-integration-server/libserver.so'])
            platforms, dependencies = stage_wayland_plugins(sdk, root / 'AppDir')
            self.assertEqual(platforms, 'libqoffscreen.so;libqwayland.so')
            self.assertEqual(len(dependencies), 3)
            for path in dependencies:
                self.assertEqual(path.read_bytes(), (sdk / path.relative_to(root / 'AppDir/usr/plugins')).read_bytes())
            self.assertFalse((root / 'AppDir/usr/plugins/wayland-graphics-integration-server').exists())

    def test_older_platform_pair_is_discovered(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.sdk(root, ['platforms/libqwayland-egl.so', 'platforms/libqwayland-generic.so',
                            'wayland-shell-integration/libxdg-shell.so'])
            platforms, _ = stage_wayland_plugins(root, root / 'AppDir')
            self.assertEqual(platforms, 'libqoffscreen.so;libqwayland-egl.so;libqwayland-generic.so')

    def test_missing_native_platform_or_shell_fails_before_deployment(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with self.assertRaisesRegex(RuntimeError, 'Wayland platform'):
                stage_wayland_plugins(root, root / 'AppDir')
            self.sdk(root, ['platforms/libqwayland.so'])
            with self.assertRaisesRegex(RuntimeError, 'xdg-shell'):
                stage_wayland_plugins(root, root / 'AppDir')


if __name__ == '__main__':
    unittest.main()
