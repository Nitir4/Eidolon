# Build and package

The desktop build requires CMake 3.24+, Ninja, a C++17 compiler and Qt 6.8+
Widgets and PrintSupport (plus Qt Test and GuiPrivate development headers
when BUILD_TESTING is enabled). GuiPrivate is used only by the synthetic
display-change test; application code uses public Qt APIs.
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
layout. Native LP editing/files and the approved All-slack/M/Two-phase/Bounded/Dual/Graphical methods are functional. Final solution and sensitivity are also functional.

The numerical, LP model and compatibility targets are Qt-free static C++17 libraries.
Tests also require Python 3 for comparison with recorded file SHA-256 values
and pypdf 6.20.0 for independent printer PDF verification. Install the reader
in your development virtual environment and select that interpreter at configure:

```sh
python3 -m venv build/test-python
build/test-python/bin/python -m pip install pypdf==6.20.0
cmake --preset dev -DPython3_EXECUTABLE="$PWD/build/test-python/bin/python"
```

On Windows use the corresponding `Scripts/python.exe` path. The reader is a
test dependency only and is not bundled in the application. Headless tests do
not need pypdf or Qt.
`compat.roundtrip` checks ordered fields and exact bytes for 27 actual TORA
saves reconstructed from original transcriptions. `compat.hashes` compares
every generated file with its actual TORA byte count and SHA-256. `compat.errors`
and `compat.files` cover recovery, malformed inputs, precision and transactional
file operations. No private TORA file is needed by CI or packaged in the app.
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

With the private reference corpus present, check the actual files directly:

```sh
build/headless/tests/codec_roundtrip tests/fixtures/lp-models.txt tests/fixtures/tora-file-settings.tsv build/foundation4/clone-roundtrips reference/tora-captures
```

`codec_probe --roundtrip input output`, `--edit input output`, or
`--new decimal|scientific output` writes files for live reference checks.
It is a test tool and is not installed. The codec is integrated with the native data-entry controller and the implemented LP methods.

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
| Windows x86_64 | Portable ZIP and per-user setup EXE | windeployqt, app-local CRT, installed startup, safe removal |
| Linux x86_64 | AppImage | linuxdeploy Qt plugin, extract-and-run startup without FUSE |
| macOS | DMG containing universal APP | macdeployqt, lipo verifies both architectures, deployed startup |

Each artifact has accompanying SHA-256 and dependency-manifest files.
Qt notices are included; SDK license files are copied when supplied. These are
development LP packages. Windows packaging also builds an NSIS per-user setup
EXE and verifies installation/reinstallation, deployed startup, uninstall and
preservation of unrelated files. macOS signing/notarization and a production
release license review remain later release work. The portable folder remains
available; a single-file portable application is not supplied.

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

## Actual-resolution graphical verification

The native scale tests use `tests/fixtures/offscreen-display.json`, copied to the build test directory. It configures a test monitor large enough for 4K; product sizing remains layout/font driven. This prevents Qt's default small offscreen display from silently clamping windows. Relative configfile paths avoid Windows drive-letter parsing. Graphical tests assert real requested window sizes and save native-only previews with `TORA_UI_CAPTURES`. Qt's supported configuration is documented by its [official offscreen implementation](https://github.com/qt/qtbase/blob/v6.10.3/src/plugins/platforms/offscreen/qoffscreenintegration.cpp).

```sh
TORA_UI_CAPTURES="$PWD/build/phase2-graphical/ui-captures" ctest --preset dev
```

Graphical expected values are independent capture transcriptions in `tests/fixtures/graphical.json`. The private-source audit can be run with `python3 tools/audit_tora_graphical_geometry.py`; `--transcribe` changes only the source-derived pixel annotations and must accompany direct review, never native-output golden generation.

Graphical native tests use the same pre-QApplication Windows FreeType font-directory bootstrap as the established shell/input/simplex tests. An explicit Latin/digit glyph check rejects unreadable offscreen evidence. Normal product rendering still uses the native OS font backend; no font file is bundled or added to the application.

Final graphical packages are under `dist/phase2-graphical/final/`, with downloaded reports/previews under `build/phase2-graphical/ci-final/`. The Linux AppImage can be launched directly after setting its executable bit; the development command remains `build/dev/src/desktop/ToraClone`. At fractional scale, PNG raster dimensions may round by one physical pixel; requested logical dimensions are asserted exactly.


## Printer and live display tests

```sh
ctest --preset dev -R 'desktop\.(printing|display-changes)'
```

The exporter writes PDFs under the build test directory. An independent reader
compares them against actual TORA print transcriptions, with exact numbers and
labels. CI uploads PDFs and test XML for inspection. Live display testing uses
Qt offscreen monitor add/remove/configuration hooks through test-only private
headers. It verifies current SDK behavior without altering the user's display.
Neither test requires a host printer or second monitor. Physical acceptance is
separate and remains recorded in OPEN_QUESTIONS.md.


## Windows installer verification

The Windows job installs pinned NSIS 3.13.0 and wraps the existing deployed
payload. Setup defaults to `%LOCALAPPDATA%\Programs\TORA Clone`, requires no
administrator access, and creates current-user Start menu/uninstall entries.
It does not take over `.txt` file associations. This is an unsigned development
installer. User preferences and unrelated files are preserved on removal.

`tools/windows_installer.py` generates exact owned-file commands from the
staged payload. Its verifier refuses existing installation/shortcut entries,
uses a temporary path with spaces and Unicode, installs twice, checks payload
hashes, launches the installed app without SDK paths, and uninstalls. Student
files placed in both the root and a runtime subdirectory must remain.
`tests/test_windows_installer.py` checks escaping, reference/symlink rejection
and nonrecursive removal on all OSes. Installer hash/manifest/verification JSON
accompany the setup EXE in the Windows artifact.

Primary behavior reference: [NSIS scripting manual](https://nsis.sourceforge.io/Docs/Chapter4.html).


Final installer source `9ac8204` passed 72/72 tests on each OS (216 total) in
[run 38059265577](https://github.com/Nitir4/TORA-Clone/actions/runs/38059265577).
The Windows setup installed and reinstalled into a Unicode path with spaces,
verified 32 manifested payload files, started without SDK paths, and removed
its files/entries while preserving student files. Downloaded package hashes
and manifests were independently checked. Final packages are under
`dist/phase2-installer/final/`; reports/audits are under
`build/installer-delivery/`. Earlier accepted packages remain preserved.

The earlier accepted installer delivery above remains preserved. For the latest
Wayland-enabled delivery, see `docs/RELEASE_READINESS.md`. Launch it on this
Linux laptop with:

```sh
chmod +x dist/release-readiness/final/TORA-Clone-ubuntu-latest/TORA-Clone-0.1.0-linux-x86_64.AppImage
dist/release-readiness/final/TORA-Clone-ubuntu-latest/TORA-Clone-0.1.0-linux-x86_64.AppImage
```

If FUSE is unavailable, set `APPIMAGE_EXTRACT_AND_RUN=1` for the launch.
The same final AppImage passed local startup with Qt SDK paths removed.


## Linux desktop protocols and runtime notices

The Linux Qt SDK must include the `qtwayland` and `qtsvg` archives alongside `qtbase`
and `icu`. Packaging fails if the Wayland platform or xdg-shell client integration
is missing. CI verifies the deployed AppImage using both Weston and Xvfb:

```sh
python tools/verify_linux_desktop.py dist/TORA-Clone-0.1.0-linux-x86_64.AppImage \
  --output build/desktop-runtime
```

On a running Wayland desktop, add `--live-wayland` to check the existing
compositor without changing display settings. SDK search paths are removed.
Headless protocol servers do not establish physical mixed-monitor acceptance.

The pinned Qt6.10.3 legal documents in `packaging/licenses/Qt-6.10.3/` are copied
only for the matching SDK version. `SOURCES.json` records130 upstream file URLs
and SHA-256 values; integrity/path tests run on all three OSes. Distribution
notices already produced by deployment are retained. The upstream index is
not a build SBOM or final legal approval. See `THIRD_PARTY_NOTICES.md` in each
package and `docs/RELEASE_READINESS.md` for remaining release resources.


Latest source `19463eb` passed 74/74 tests on each OS (222 total) in
[run 38062462111](https://github.com/Nitir4/TORA-Clone/actions/runs/38062462111). Six downloaded archives, all LP/PDF/display reports,
package hashes, 130 matching Qt legal documents and Windows installer checks
were independently verified. The final Linux AppImage passed both CI desktop
protocol backends and actual laptop Wayland startup; all 281 extracted payload
hashes match. Windows setup verifies 163 installed files. Latest packages are
under `dist/release-readiness/final/`, with private audits in
`build/release-readiness/`. These unsigned builds retain the hardware/signing
and owner production-release decisions documented in `RELEASE_READINESS.md`.
