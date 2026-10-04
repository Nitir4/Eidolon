# Foundation 1 gate report

Gate 0 was approved on 2026-10-04. This step implements only the project
skeleton, CMake, CI, smoke tests and native packaging. Implementation and
verification are complete. Foundation 1 is ready for user gate review.

The C++17 executable starts Qt 6 Widgets with a layout-managed bootstrap
window. Qt-free core, LP and compatibility interface targets establish the
architecture boundaries; they contain no solver or codec implementation yet.
Desktop-only Qt setup keeps the core build free of GUI dependencies.

One workflow builds and runs every current test on Windows, Linux and macOS,
then packages and starts the deployed application. Windows supplies a
portable EXE folder in a ZIP; Linux an AppImage; macOS a DMG with a universal
APP and checks both executable/framework architectures. Checksums and
dependency manifests accompany all artifacts. Private TORA binaries and
reference captures are ignored and excluded from packaging.

| Validation | Result |
| --- | --- |
| Local Linux Debug, GCC 16.2.1 / Qt 6.11.2 | 3/3 tests passed |
| Qt-free headless build | 1/1 test passed |
| Linux AppImage deployed startup | Passed |
| Workflow lint (actionlint 1.7.12) | Passed |
| GitHub Windows build, all tests and portable packaging | Passed; 3/3 tests |
| GitHub Linux build, all tests and AppImage packaging | Passed; 3/3 tests |
| GitHub macOS build, all tests and universal DMG packaging | Passed; 3/3 tests |
| Three downloadable CTest XML reports | Uploaded; 9 passing tests |
| Live Wayland window with malformed qt6ct theme font | Readable label verified visually |

CI evidence: [passing final run](https://github.com/Nitir4/TORA-Clone/actions/runs/37176999256),
source commit `86d910c592c068372fbbea5d2bbb3fe8bb327db1` on `main`.
All three jobs passed build, CTest, deployed startup and artifact upload.
The workflow supplies these native payloads plus SHA-256/manifests:

| Artifact | Payload |
| --- | --- |
| TORA-Clone-windows-latest | TORA-Clone-0.1.0-windows-x86_64.zip; portable EXE and DLLs |
| TORA-Clone-ubuntu-latest | TORA-Clone-0.1.0-linux-x86_64.AppImage |
| TORA-Clone-macos-latest | TORA-Clone-0.1.0-macos-universal.dmg; APP bundle |

Actual CI failures were resolved before this gate: the Linux SDK architecture
name, the older SDK's removed Apple AGL dependency, missing bundled ICU, lipo
argument order, and CTest's report path relative to the build directory.
CI uses Qt 6.10.3 on all OSes and preserves Python 3.12; this Qt version works
with the released installer, whereas its Qt 6.11 Windows support is pending a
new installer release. There are no known Foundation 1 build/test/package
mismatches. The next numerical and UI gates remain unimplemented.

The first live preview exposed malformed serialized font settings from
qt6ct: a garbled family name, style hint 255 and pixel size 167828479.
`src/desktop/ui_font.hpp` now preserves valid preferences and recovers invalid
settings using a DPI-aware sans-serif font in points. The recovered window
font resolves every attribute so a theme's later font update cannot merge
corrupted values back into the label. The new `desktop.font` regression test
checks preference preservation, glyph coverage and recovery after a theme
update. A live screenshot confirmed readable text on the user's laptop.

Windows offscreen font tests explicitly use the system Fonts directory:
Qt's [offscreen backend](https://github.com/qt/qtbase/blob/v6.10.3/src/plugins/platforms/offscreen/qoffscreenintegration.cpp)
uses FreeType there, and its [font directory selection](https://github.com/qt/qtbase/blob/v6.10.3/src/gui/text/qplatformfontdatabase.cpp)
otherwise points to fonts that the Qt SDK does not ship. This is test-only
platform setup; the application uses the normal native platform backend.

The app is a bootstrap window. No TORA numerical, file compatibility or
screen/DPI acceptance claim is made in this step. Reference evidence remains
available locally for later tests-first module implementation. Recorded
Phase 0 unknowns remain in `OPEN_QUESTIONS.md`.

The macOS runner exercised Apple Silicon startup. Both arm64 and x86_64
slices in the executable and deployed Qt frameworks passed lipo checks;
startup on physical Intel hardware is not separately verified. Packages are
unsigned development artifacts. Other OS versions and mixed-monitor/DPI
behavior require later acceptance testing. SDK baseline limits are in BUILD.md.

See [BUILD.md](BUILD.md) for commands and package limitations. The next step is
Foundation 2: the Qt-free numerical library. It starts only after Foundation 1
gate approval.
