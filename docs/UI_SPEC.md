# Native UI specification for Gate 0

This preserves observed TORA terminology, panel order and teaching workflow.
Legacy screenshot coordinates are reference measurements, not Qt layout sizes.
The proposed clone uses C++17+, CMake and Qt 6 Widgets, with QPainter or
QGraphicsView plots. No browser, HTML UI or QtWebEngine is permitted.

## Screen navigation

`Welcome → MAIN → Linear Programming → input mode → input grid → SOLVE/MODIFY`.
Solve Problem has Graphical and Algebraic; Algebraic has Final solution and
Iterations. Iterations has All-slack starting solution, M-method, Two-phase
method, Bounded simplex, Dual simplex, in that order. M prompts for its finite
value before output formatting. Every output path passes through the shared
format screen. Sensitivity is a section of Final solution. Revised simplex and
Parametric analysis are absent from the observed LP hierarchy.

The MAIN labels/order are recorded in `tora-analysis/APPLICATION_SHELL.md`.
Only LP is implemented in this project's scope; other modules remain future
registrations. Screen specifications and capture references are indexed in
`tora-analysis/README.md`.

## Input screens

Input mode offers Enter New Problem / Select Existing File, Decimal Notation
(NNNNN.DD) / Scientific Notation (N.NNNNeDD), How many N?, How many D?,
Go to Input Screen, MAIN Menu, Exit TORA. Defaults are new/decimal, N=5/D=2.
The grid has Problem Title:, Nbr. of Variables:, No. of Constraints:, input
instructions to its right, and the INPUT GRID - LINEAR PROGRAMMING heading.
Return or Tab initializes the grid. The observed Student limit is 100/100.

Grid columns are Var. Name, x1…xn, Enter <, >, or =, R.H.S. Rows are variable
names, objective sense/coefficients, Constr 1…m, Lower Bound, Upper Bound,
Unrestr'd (y/n)?. Normal initialization uses lower 0.00, upper infinity, n.
The File menu has Save, Save As, Main Menu, Exit TORA. Existing-file opening
is reached through input mode, not this File menu. Leaving asks Do you wish
to save data?. Native Text pickers handle open/save.

EditGrid order: Undo Last Insert, Paste, Or Delete (Ctrl+U), Copy Column or Row
(Ctrl+C), Paste on Column or Row (Ctrl+P), Delete Column or Row (Ctrl+D),
Insert Column or Row (Ctrl+I). Constraint heading selection turns its row red.
Menu-driven row copy/paste, deletion and insertion were executed; Undo remained
disabled in these probes. Column operations and successful shortcuts are unknown.

## Output screens and presentation

Output formatting offers decimal/scientific radios, N/D defaults 5/2 and
Go To Output Screen. It has View/Modify Input Data, MAIN Menu, Exit TORA.
M adds Enter value of M: (100) and Go To Output Format Screen.

Iteration screens display the method heading, title/direction, pivot
instructions, Next Iteration, All Iterations, Write to Printer, and an
accumulated scrollable tableau. Every table has its phase/iteration heading,
Basic, ordered variables, Solution, z (max/min), basis rows, Lower Bound,
Upper Bound, Unrestr'd (y/n)?. Preserve x/sx/Sx/Rx/Ux/Lx/ULx spelling,
column order, blocked artificial objective cells and number strings. Entering
columns turn green; leaving rows turn red. Validation says tied columns must
be leftmost and tied rows topmost. Alternative-optimum selection re-enables
Next and appends another table. Final-summary Next/All are disabled.

Final solution contains title, final iteration count, objective, variable
values/coefficient/contribution, constraint RHS/slack/surplus and sensitivity
coefficient/RHS ranges, reduced costs, dual prices. A trailing slack/surplus
sign is separate from a numeric leading sign. Infeasible/unbounded summaries
show their message without numeric sensitivity sections. The automatic final
count can differ from interactive Two-phase; preserve both observed paths.

Graphical output places model/result panels left and a square native canvas
right, with Zoom In/Out and Print Graph above. Constraint labels are numbered;
feasible dots, objective line and selected optimum follow the captures. One
stroke and each-constraint actions retain TORA wording. The three-variable
Graphical menu item is disabled. See LP_GRAPHICAL.md for termination/zoom.

Observed dominant palette: blue background/numeric cells, cyan headers/grid
panels, white output text/borders, blue header text; green entering selection,
red leaving selection, yellow graph labels. Preserve these roles in scalable
styles. Use original drawing/text; do not extract TORA icons or embed captures.
Exact original font identity is unverified. Adopt readable native fonts and
font metrics, then compare the familiar hierarchy and layout with the oracle.

## Resolution, DPI and accessibility requirements

Use layout-managed QMainWindow/screens, QTableView models, splitters and scroll
areas. Size controls from font metrics; no fixed pixel geometries or a window
locked to 800×600. Qt 6 provides high-DPI coordinates/scaling; handle changes
in screen/DPI/fonts, invalidate graph caches, and repaint natively. Restore
window geometry only within available screens. Keep every control reachable
on a small logical viewport through scrolling; maintain keyboard focus order.

Validate Windows 10/11, macOS arm64/x86_64 and Linux x86_64; 1366×768 through
4K; 100/125/150/200% scaling; mixed-DPI monitor moves and disconnects. These
are clone acceptance requirements, not performed Phase 0 GUI tests.

## Unknown observations

Successful printing is unavailable: Wine returned runtime error 482, Printer
error, then exited. Exact print layout, graph eligibility outside the probes,
Undo enablement, general zoom/dot rules, full formatting-field limits and all
keyboard/mnemonic states remain in OPEN_QUESTIONS.md. Product GUI tests and
visual comparisons begin only after Gate 0 and the relevant module gates.

## Foundation 5 implementation and review boundary

This subsection records the original Foundation 5 preview. The Phase 2 input
implementation below supersedes its disabled editor/file controls.

The shell now supplies seven native screen frameworks: welcome, MAIN,
`lp/input-mode`, `lp/input`, `lp/solve-menu`, `lp/output-format`, `lp/m-value`.
The first launch maximizes normally, filling the available window with the blue
canvas. Resizing changes the canvas and placement; centered panels retain
font-derived readable sizes. On a narrow logical viewport their scroll area
keeps controls reachable. There is no 800×600 restriction or raster scaling.

MAIN is a persistent native panel with the observed order and a `MainMenu`
menubar. Non-LP modules are disabled. SOLVE/MODIFY retains its native hierarchy
and separators. The popup uses click/keyboard activation in this foundation;
matching the legacy automatic hover behavior of its centered panel is deferred
to the solving-screen module. Menubar submenus use normal Qt menu behavior.
No Revised or Parametric entry has been invented.

This is explicitly a navigation preview. New/existing and decimal/scientific
radio states work; formatting counts show read-only defaults. Existing-file
loading, title/dimension editing, Save/Save As, EditGrid and solver leaf actions
are disabled until the domain/codec/module foundations are approved. The empty
input page's SOLVE Menu navigates to the menu for review; this intentional
preview exception does not initialize a grid or claim a solution. There is no
save prompt because no document has been created. Output-format and M pages
can be reviewed with `--screen`; the solver menu cannot advance to an output.
Default N=5/D=2 and M=100 are displayed without inventing their numerical limits.

Original clone welcome text replaces proprietary copyright/version artwork.
The classic Qt Fusion widget style and scoped colors keep the display readable
under the laptop's dark theme. Labels on blue are white; cyan entries use blue
text; white menus use black text and dark-blue focus/selection. Border strokes
are logical pixels and padding uses font units; controls have no fixed pixel
geometry. This is a familiar scalable recreation, not an 800×600 screenshot.

Automated checks cover 1366×768, 1920×1080 and 3840×2160 at 1/1.25/1.5/2
scale, seven routes, control/font/palette access and native image DPR. Synthetic
multi-screen rectangles cover negative origins, mixed-DPI gaps and unplugged
monitors; display/font events exercise invalidation. Physical mixed-monitor
handoff and manual Windows/macOS display checks still need those environments.
Qt documents its [high-DPI scaling and test factors](https://doc.qt.io/qt-6/highdpi.html),
[screen-change notification](https://doc.qt.io/qt-6/qwindow.html#screenChanged)
and [available-geometry notification](https://doc.qt.io/qt-6/qscreen.html#availableGeometryChanged).

## Foundation 2 library boundary

The numerical library now supplies independently tested decimal/scientific
formatting. [NUMERIC_FOUNDATION.md](tora-analysis/NUMERIC_FOUNDATION.md) records
halfway/sign, exponent and precision observations. The format screen retains
its reviewed read-only preview until model/output modules supply real values
and confirm full field validation. Foundation 2 changes no screen layout.

## Phase 2 native LP input

New/Existing and N/D fields now drive the real LP controller. Title/count entry
initializes the native editable table with Return/Tab. Save/Save As, read-only
copies, the Save? Yes/No/Cancel flow and applicable EditGrid commands work.
The empty input SOLVE button is disabled; after initialization it validates
all-zero constraint rows before leaving. Solver leaf commands remain disabled.

The table keeps aliases separate from x headers and shows observed row/column
labels and exact stored numeric spellings. It uses cyan labels, blue numeric
cells, bold white values, red heading selection and white native editors.
Fonts determine panel/row/column sizes; large inputs scroll within the table,
and the blue background fills the resizable window at native DPI. The tests
cover populated normal/100×100 grids at three resolutions/four scale factors.

The Qt Widgets Open picker includes Open as read-only; the save picker handles
suffix/overwrite cancellation. Save prefills the actual last destination, even
when the loaded source caption is retained, including a new input context.
Existing enters blank input before opening the picker; cancellation keeps that
blank context. Wrong-module files offer the observed Yes/No retry warning.
Direct failed Open and cancelled Save preserve data/checkpoints. Dialogs use
the same theme-font repair as pages. Module guards also protect window close.

See LP_DATA_ENTRY_MODULE.md for the 22 independently observed stored outcomes,
verified keyboard states and explicit precision/code-page/hardware limits.
