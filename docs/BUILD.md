# Build and package

The desktop build requires CMake 3.24+, Ninja, a C++17 compiler and Qt 6.8+
Widgets (plus Qt Test when BUILD_TESTING is enabled).
Use MSVC on Windows, Apple Clang on macOS, and GCC or Clang on Linux.
Qt discovery uses `CMAKE_PREFIX_PATH`; Qt deployment uses `QT_ROOT_DIR`.
No browser engine or HTML UI is used.

```sh
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

The desktop executable is `build/dev/src/desktop/ToraClone` on Linux,
`ToraClone.exe` on Windows, and a `ToraClone.app` bundle on macOS.
`--smoke-test` starts the native Widgets event loop and exits.
The Foundation 5 shell launches maximized on its first interactive run;
normal window geometry is then remembered. `--no-settings` bypasses preferences
and `--screen lp/output-format` (or another registered route) previews one
layout. Editing, files and solving are disabled at this gate.

The numerical and LP model targets are Qt-free static C++17 libraries.
The compatibility target remains an interface boundary until Foundation 4.
`lp.model` checks 22 original model transcriptions from the private reference;
`lp.validation` checks observed dimension/bound messages and explicitly
identified engineering safety diagnostics. `core.numerics` checks
observed pivot tables and arithmetic failures; `core.formatting` checks reviewed
TORA display strings under both classic and comma locales.
This Qt-free configuration proves the core can build independently:

```sh
cmake --preset headless -DCMAKE_CXX_STANDARD=17
cmake --build --preset headless
ctest --preset headless
```

Release build and packaging:

```sh
cmake --preset ci
cmake --build --preset ci --parallel
ctest --preset ci
python3 tools/package_desktop.py --build-dir build/ci --qt-root /path/to/Qt
```

Packaging uses Python 3.11+ (CI uses 3.12). Windows requires an MSVC developer
environment, including `VCToolsRedistDir`. On macOS, configure with
`cmake --preset ci '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64'` and a universal Qt SDK.
Linux packaging needs network access on its first run to download the
checksum-pinned linuxdeploy tools into `.packaging-tools/`.

The single [workflow](../.github/workflows/build.yml) uses `windows-latest`,
`ubuntu-latest` and `macos-latest`, Qt 6.10.3, and these artifacts:

| OS | Payload | Deployment check |
| --- | --- | --- |
| Windows x86_64 | ZIP containing portable EXE folder | windeployqt, app-local MSVC CRT, startup with SDK paths removed |
| Linux x86_64 | AppImage | linuxdeploy Qt plugin, extract-and-run startup without FUSE |
| macOS | DMG containing universal APP | macdeployqt, lipo verifies both architectures, deployed startup |

Each artifact has accompanying SHA-256 and dependency-manifest files.
Qt notices are included; SDK license files are copied when supplied. These are
development navigation-preview packages. Installers, macOS signing/notarization and a
production release license review are later release work. Windows uses a
portable folder; a single-file executable is not supplied.

The CI SDK's supported floors are Windows 10 1809, macOS 13 and Linux with
glibc 2.39. The AppImage does not bundle glibc; older Linux distributions need
a build against an older baseline. These are SDK/package compatibility limits,
not proof of testing on every OS version. See Qt's
[supported platforms](https://doc.qt.io/qt-6.10/supported-platforms.html).

Linux symbol stripping is disabled because linuxdeploy's bundled strip can
reject modern RELR sections. This increases artifact size and preserves
startup correctness. See the maintainer's [NO_STRIP documentation](https://github.com/linuxdeploy/linuxdeploy/issues/72).
Changed upstream tool assets require a reviewed checksum pin update.

Qt's [deployment documentation](https://doc.qt.io/qt-6.10/cmake-deployment.html)
and [macOS universal build guidance](https://doc.qt.io/qt-6.10/macos.html) support
the packaging configuration. Foundation 5 tests simulate three resolutions
and four scale factors on each OS; CI uploads original clone screenshots with
the test XML. To generate the same optional images locally:

```sh
TORA_UI_CAPTURES="$PWD/build/foundation5/screens" ctest --preset dev
```

Physical mixed-monitor handoff and OS-version acceptance still need hardware
review. Offscreen startup does not establish numerical TORA compatibility.
