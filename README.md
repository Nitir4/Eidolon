# Native TORA LP clone

Phase 0 is approved. Foundation 1 supplies the C++17/CMake skeleton, native
Qt 6 Widgets bootstrap, smoke tests and three-OS packaging pipeline.
Foundation 5 adds the native TORA-style main window, menus and LP navigation
preview. Foundation 2 supplies Qt-free matrix/tableau arithmetic, explicit
tolerances and observed decimal/scientific formatting. Foundation 3 adds the
Qt-free LP model and validation against observed input fields. Foundation 4 adds
the Qt-free TORA LP document, codec and binary file store. All foundations are
complete in the approved order **1 → 5 → 2 → 3 → 4**. Phase 2 is implemented
and verified for the captured LP profile;
its input module supplies native LP data entry, row/column editing and Open/Save.
All-slack, finite M-method, Two-phase, Bounded and Dual simplex supply manual/automatic iteration tables;
Graphical supplies native two-variable plots, constraint-by-constraint drawing, zoom and PDF printing.
Final solution and sensitivity are implemented in the native output summary;
all three OS builds pass 71 tests each, including independent printer PDF
comparison and live synthetic display changes. The latest verification and packages
are recorded in [current status](docs/STATUS.json). Recorded physical-printer/driver,
physical mixed-DPI and outside-profile limits remain in [open questions](docs/OPEN_QUESTIONS.md).
One source tree and the `main` branch serve Windows, macOS and Linux.

See [current status](docs/STATUS.json), [Foundation 1 report](docs/FOUNDATION1_REPORT.md)
[Foundation 5 report](docs/FOUNDATION5_REPORT.md) and
[Foundation 2 report](docs/FOUNDATION2_REPORT.md) and
[Foundation 3 report](docs/FOUNDATION3_REPORT.md) and
[Foundation 4 report](docs/FOUNDATION4_REPORT.md) and
[Phase 2 input report](docs/PHASE2_INPUT_REPORT.md),
[All-slack report](docs/PHASE2_ALL_SLACK_REPORT.md),
[M-method report](docs/PHASE2_M_METHOD_REPORT.md),
[Two-phase report](docs/PHASE2_TWO_PHASE_REPORT.md), plus
[build instructions](docs/BUILD.md). With CMake 3.24+, Ninja, a C++17 compiler
and Qt 6.8+ Widgets/PrintSupport installed (plus Qt Test/GuiPrivate and the
Python PDF reader when running tests; see the build instructions):

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
→ Go to Input Screen**, enter the title/counts and press Return or Tab in the
constraint count to initialize the editable grid. Choose Select Existing File
to open a TORA .txt file, including a read-only copy. Save/Save As and EditGrid
are enabled when applicable; SOLVE Menu validates the input and offers Save?.
Choose Solve Problem → Algebraic → Iterations → All-slack starting solution or
M-method, Two-phase method, Bounded simplex or Dual simplex. M-method asks for a finite penalty (default 100),
then output format. Two-phase uses the common output format and shows its
feasibility and original-objective phases. Bounded goes directly to output format
and uses All-slack or Two-phase after lower-bound/RHS normalization.
Dual rejects equations, uses a direct negative-RHS iteration path when applicable,
and preserves the observed warnings and All-slack/Two-phase/M fallbacks.
These iteration methods support Next/All Iterations and native printing.
For two variables with default nonnegative bounds, choose **Solve Problem → Graphical**,
select the output format, then click each constraint and the objective or
**Click here to graph LP in one stroke**. The graph and result panel expand with the native window.
Other OR modules remain outside this LP project. Review layouts with, for
example, `ToraClone --no-settings --screen lp/output-format` (also
`lp/m-value`, `lp/input-mode`, `lp/input`, `lp/solve-menu`, `main`, `welcome`).
`--no-settings` bypasses saved geometry; first launch maximizes the normal
resizable window. Automated screen captures contain only the clone's original
UI, under `build/phase2-input/ui-captures/` for the input module, the earlier
`build/foundation5/screens/` for navigation, and the CI test-results artifacts.

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


Printing now follows 25 observed original print states: input pages, final
summary/sensitivity, method-specific trace bands and graphical output. Public
independent text fixtures are checked against native PDFs. See the
[printer specification](docs/tora-analysis/LP_PRINTING.md) for coverage and
font/driver limits. Live synthetic display-change tests cover four populated
views at 125%, 150% and 200%, including scale changes and display removal;
physical mixed-monitor handoff still needs hardware acceptance.

Windows packaging now also generates a per-user setup EXE alongside the
portable ZIP. Install/reinstall, installed startup and safe removal are checked
in the Windows CI packaging step. Physical acceptance steps are in
[the hardware protocol](docs/HARDWARE_ACCEPTANCE.md); unavailable devices are
recorded as untested.


The final delivery at source `9ac8204` passes 72/72 tests per OS (216 total)
in [CI run 38059265577](https://github.com/Nitir4/TORA-Clone/actions/runs/38059265577).
Downloaded packages, including the verified Windows installer, are
under `dist/phase2-installer/final/` in this workspace. See
[build and launch instructions](docs/BUILD.md#windows-installer-verification).
