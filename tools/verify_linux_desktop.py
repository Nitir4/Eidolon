#!/usr/bin/env python3
"""Check the deployed AppImage against real Wayland/X11 protocol servers."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
import time

from package_desktop import smoke_env


def smoke(app, env, output, backend, command_prefix=()):
    env['QT_QPA_PLATFORM'] = backend
    env['APPIMAGE_EXTRACT_AND_RUN'] = '1'
    with (output / (backend + '.log')).open('w') as log:
        subprocess.run([*command_prefix, str(app), '--smoke-test'], env=env,
                       stdout=log, stderr=subprocess.STDOUT, check=True, timeout=45)
    return {'backend': backend, 'startup': 'passed'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('appimage', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--live-wayland', action='store_true',
                        help='Use the existing compositor without changing display settings.')
    args = parser.parse_args()
    app = args.appimage.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    app.chmod(app.stat().st_mode | 0o111)
    env = smoke_env()
    checks = []
    if args.live_wayland:
        if not env.get('WAYLAND_DISPLAY'):
            raise RuntimeError('No existing Wayland display')
        checks.append(smoke(app, env, args.output, 'wayland'))
    else:
        with tempfile.TemporaryDirectory(prefix='tora-wayland-check-') as name:
            runtime = Path(name)
            runtime.chmod(0o700)
            env['XDG_RUNTIME_DIR'] = str(runtime)
            env['WAYLAND_DISPLAY'] = 'tora-check'
            env.pop('DISPLAY', None)
            with (args.output / 'weston.log').open('w') as log:
                server = subprocess.Popen(['weston', '--backend=headless-backend.so',
                                           '--socket=tora-check', '--idle-time=0', '--use-pixman'],
                                          env=env, stdout=log, stderr=subprocess.STDOUT)
                try:
                    deadline = time.monotonic() + 15
                    while not (runtime / 'tora-check').exists():
                        if server.poll() is not None or time.monotonic() > deadline:
                            raise RuntimeError('Wayland check compositor did not start; inspect weston.log')
                        time.sleep(.1)
                    checks.append(smoke(app, env, args.output, 'wayland'))
                finally:
                    if server.poll() is None:
                        server.terminate()
                        try:
                            server.wait(timeout=5)
                        except subprocess.TimeoutExpired:
                            server.kill()
                            server.wait(timeout=5)
        env = smoke_env()
        env.pop('WAYLAND_DISPLAY', None)
        checks.append(smoke(app, env, args.output, 'xcb',
                            ['xvfb-run', '-a', '-s', '-screen 0 1920x1080x24']))
    result = {'package': app.name, 'sha256': hashlib.sha256(app.read_bytes()).hexdigest(),
              'scope': 'existing Wayland compositor' if args.live_wayland else 'headless protocol servers',
              'qt_sdk_paths_removed': True, 'checks': checks}
    (args.output / 'verification.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result))


if __name__ == '__main__':
    main()
