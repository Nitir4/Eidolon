# Foundation 1 gate report

Gate 0 was approved on 2026-10-04. This step implements only the project
skeleton, CMake, CI, smoke tests and native packaging. Its gate remains pending
until the actual three-OS workflow passes and the user reviews this report.

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
| Local Linux Debug, GCC 16.2.1 / Qt 6.11.2 | 2/2 tests passed |
| Local Linux Release | 2/2 tests passed |
| Qt-free headless build | 1/1 test passed |
| Linux AppImage deployed startup | Passed |
| Workflow lint (actionlint 1.7.12) | Passed |
| GitHub Windows/Linux/macOS build, tests and packaging | Pending first run |

The app is a bootstrap window. No TORA numerical, file compatibility or
screen/DPI acceptance claim is made in this step. Reference evidence remains
available locally for later tests-first module implementation. Recorded
Phase 0 unknowns remain in `OPEN_QUESTIONS.md`.

See [BUILD.md](BUILD.md) for commands and package limitations. The next step is
Foundation 2: the Qt-free numerical library. It starts only after Foundation 1
gate approval.
