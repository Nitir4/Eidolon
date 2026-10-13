# Native TORA clone

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
the published Vogel release passes 82 tests on each OS, including independent printer PDF
comparison and live synthetic display changes. The latest verification and packages
are recorded in [current status](docs/STATUS.json). Recorded physical-printer/driver,
physical mixed-DPI and outside-profile limits remain in [open questions](docs/OPEN_QUESTIONS.md).
One source tree serves Windows, macOS and Linux; the authorized extensions are on `future-scope`.

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
Transportation is an authorized extension with North-west, Least-cost and Vogel starts on `future-scope`. The working source also adds editable input and file writing; the published Vogel binaries retain their earlier read-only input. Other OR modules remain outside this project. Review layouts with, for
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


The earlier release-readiness delivery at source `19463eb` passes 74/74 tests per OS (222 total)
in [CI run 38062462111](https://github.com/Nitir4/TORA-Clone/actions/runs/38062462111). Downloaded packages, including the verified Windows
installer, are under `dist/release-readiness/final/` in this workspace. The
Linux AppImage supports Wayland and X11 and starts on this laptop's Wayland
desktop with SDK paths removed. Matching Qt legal documents are included.
See [the delivery verification](docs/RELEASE_READINESS.md) and
[build and launch instructions](docs/BUILD.md). Physical mixed-monitor/printer
acceptance and trusted production signing remain resource-dependent.


The keyboard/readability repair at source `c4cf3ad` passes 74/74 tests on each
OS in [CI run 38211734612](https://github.com/Nitir4/TORA-Clone/actions/runs/38211734612).
Screen text uses a 12pt minimum and a light, high-contrast palette. Arrow keys
continue editing in adjacent editable grid cells; Left/Right preserve normal
caret movement within text. New packages are in `dist/readability/final/`.
On this laptop, launch the updated installed app with `tora-clone` or the
**TORA Clone** application-menu entry. Existing windows keep their older
version until closed. Algorithms, numeric displays and saved files are unchanged.


The Transportation preview is available from **MAIN Menu → Transportation
model → Load Example → SOLVE Menu → North-west corner start**. It can also
open existing default-format TORA Transportation files, show input read only,
and step automatic iterations. Seventeen captured cases supply 38 independently
transcribed North-west tableaus. New/edit/save, Least-cost/Vogel, manual loops,
final summary and Transportation printing remain future work. Unbalanced or
unsupported fractional/format inputs produce explicit errors. See the
[Transportation scope and evidence](docs/tora-analysis/TRANSPORTATION_OVERVIEW.md).


The earlier North-west preview delivery at source `7e35986` passes **80/80
tests on each OS (240 overall)** in
[CI run 38243044784](https://github.com/Nitir4/TORA-Clone/actions/runs/38243044784).
Independent downloaded-report checks confirm 1,261 Transportation fields across
17 cases/38 tableaus, alongside the existing 31,654 LP fields per OS. All three
native packages and the Windows installer passed their deployment checks;
packages are in `dist/transportation/verified/`. The installed `tora-clone`
launcher opens this release. Linux's headless Wayland check required an
unchanged-source job rerun; final evidence uses the successful rerun, and the
failed report is retained separately. Actual laptop Wayland and X11 packaged
startup checks passed. The still-open previous window has the identical native
application executable and need not be restarted to see this preview.


On `future-scope`, Transportation also offers **Least-cost start**, with the
same read-only input and automatic MODI iteration view. Source `77e78cb` passes
**81/81 tests per OS (243 overall)** in
[CI run 38249608884](https://github.com/Nitir4/TORA-Clone/actions/runs/38249608884).
Its 17 cases/20 tableaus/644 exact fields match TORA; existing LP and North-west
comparisons also pass. Both starts show the observed initial-optimal notice.
Downloaded packages and reports passed independent integrity/comparison checks;
packages are in `dist/transportation/leastcost-verified/`. The installed
`tora-clone` launcher opens this release. Choose **Transportation model → Load
Example → SOLVE Menu → Least-cost start**; this example is already optimal
under Least-cost, so the initial optimal notice and disabled controls are expected.

Vogel, editing/saving, manual loops, alternate multipliers, final summary and
Transportation printing remain future work. See the
[Least-cost specification](docs/tora-analysis/TRANSPORTATION_LEAST_COST.md)
for evidence and profile limits. The main-branch delivery above is historical;
all additional feature commits remain on `future-scope`.

## Vogel method cycle on future-scope

Vogel start is delivered on `future-scope`. Its 18 captured problems contain
19 tableaus and 625 displayed fields, all matching TORA exactly. The final
[three-OS CI](https://github.com/Nitir4/TORA-Clone/actions/runs/38265173986) passes
82 tests per OS (246 overall), with 71 Transportation checks at each scale and
36 resolution renders per OS. Windows, macOS and Linux renders were reviewed;
the Windows offscreen font omission is fixed and covered by a glyph regression.

The verified Linux AppImage from source `6b01925` is installed and open through
`tora-clone`. Local feature tests passed 82/82, the final focused checks passed
8/8, the Qt-free suite passed 30/30, and actual Wayland acceptance passed all
14 views. See [Vogel analysis](docs/tora-analysis/TRANSPORTATION_VOGEL.md) for
evidence and the remaining Transportation scope.


The current working source adds **Transportation model → Enter New Problem**. Enter source/destination counts and press Return or Initialize Grid. Edit aliases, costs, supplies and demands directly; Return/Tab commits and advances. File → Save/Save As writes a compatible default-format text file. Open includes a read-only option; Save As to another file creates an editable copy. A new file matches an authentic TORA save exactly, and a clone-written cost edit survives TORA open/save byte for byte. See [the editing specification](docs/tora-analysis/TRANSPORTATION_EDITING.md) and [current status](docs/STATUS.json) for verification and remaining work. Launch `tora-clone` and choose Transportation model → Enter New Problem to use the installed feature. The corrected source passed its independent delivery gate and is now installed through `tora-clone`; previous releases are preserved.

Typed Transportation supplies/demands commit to whole numbers, matching the captured input workflow; untouched imported fractions retain their original text. Canceling the unsaved-data prompt blocks direct solver menu actions before a trace is computed.

The input-editing [final three-OS gate](https://github.com/Nitir4/TORA-Clone/actions/runs/38286295916) passes 88 tests per OS (264 overall), three authentic input hashes per OS and native input checks at four scales. The source `edc34b0` AppImage is installed and verified on Wayland. Structural EditGrid operations, manual loops, multiplier choices, final summary, Transportation printing and other OR modules remain separate cycles.


Transportation **EditGrid** is now installed: click a source/destination heading, then choose Copy, Paste, Delete or Insert. Single click inserts after; double click inserts before. Ctrl+U restores the last structural edit. [The verified gate](https://github.com/Nitir4/TORA-Clone/actions/runs/38292868212) passes89tests on eachOS. See [EditGrid contract](docs/tora-analysis/TRANSPORTATION_EDITGRID.md) for observed rules, original defects corrected and remaining scope. Multiplier/manual loops, final summary, printing/settings and six other modules remain underway.


Transportation **Initialize u or v** is now installed. Select any source or destination zero anchor before advancing the first tableau. The anchor persists through later iterations; already-optimal starts still permit selection. [The verified gate](https://github.com/Nitir4/TORA-Clone/actions/runs/38348718758) passes 91 tests on each OS. See [the multiplier contract](docs/tora-analysis/TRANSPORTATION_MULTIPLIERS.md) for observed formatting and control behavior. Manual-loop feedback and the remaining application modules are next.
