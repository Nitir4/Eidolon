# Native TORA LP clone

Phase 0 is approved. Foundation 1 supplies the C++17/CMake skeleton, native
Qt 6 Widgets bootstrap, smoke tests and three-OS packaging pipeline.
Foundation 5 adds the native TORA-style main window, menus and LP navigation
preview. Foundation 2 supplies Qt-free matrix/tableau arithmetic, explicit
tolerances and observed decimal/scientific formatting. Problem editing,
algorithms and file compatibility remain behind
their individual gates. The user changed the order to **1 → 5 → 2 → 3 → 4**;
work stops after each foundation for review before starting the next.
One source tree and the `main` branch serve Windows, macOS and Linux.

See [current status](docs/STATUS.json), [Foundation 1 report](docs/FOUNDATION1_REPORT.md)
[Foundation 5 report](docs/FOUNDATION5_REPORT.md) and
[Foundation 2 report](docs/FOUNDATION2_REPORT.md), plus
[build instructions](docs/BUILD.md). With CMake 3.24+, Ninja, a C++17 compiler
and Qt 6.8+ Widgets installed:

```sh
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
build/dev/src/desktop/ToraClone
```

Set `CMAKE_PREFIX_PATH` to your Qt SDK if needed. The `headless` preset builds
without Qt; `ci` builds Release artifacts. The [workflow](.github/workflows/build.yml)
builds, tests, packages and checks deployed startup on all three OSes.

The normal launch starts at Welcome. Choose **Click Here → Linear Programming
→ Go to Input Screen → SOLVE Menu** to review the navigation. Save/edit/solve
commands are disabled in this preview. Review individual layouts with, for
example, `ToraClone --no-settings --screen lp/output-format` (also
`lp/m-value`, `lp/input-mode`, `lp/input`, `lp/solve-menu`, `main`, `welcome`).
`--no-settings` bypasses saved geometry; first launch maximizes the normal
resizable window. Automated screen captures contain only the clone's original
UI, under `build/foundation5/screens/` locally and the CI test-results artifacts.

The approved analysis is in [the Phase 0 report](docs/PHASE0_REPORT.md),
[analysis index](docs/tora-analysis/README.md),
[architecture](docs/ARCHITECTURE.md) and [open questions](docs/OPEN_QUESTIONS.md).

The observation scripts in tools/ are reference tooling only. TORA binaries,
Wine runtime, screenshots and saved reference files are private/gitignored.
Do not publish them or include them in release artifacts without the user's
explicit approval.

Run `python3 tools/audit_phase0.py` to validate the saved local evidence without
launching TORA or repeating any completed cases. It requires the private
reference corpus; its pass is an evidence audit, not Gate 0 approval.
