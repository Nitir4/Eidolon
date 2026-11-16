# Native UI specification for Gate 0


## Readability revision requested on 12 October 2026

This section supersedes the historical blue/cyan screen-colour descriptions
below. Following direct user feedback about invisible text and tiny fonts,
keep TORA's workflow, wording and table order while using a light blue canvas,
dark text, white editable cells and muted blue headers. Pair dark-blue
selection with white text, including selected row labels. Entering/leaving
highlights use pale green/red with dark text. Graphs use a white plotting area.
Screen fonts have a 12pt minimum (larger preferences preserved), scale through
Qt's native DPI handling and drive existing layout metrics. The original 9pt
print face is independent of the screen minimum; numerical/file expectations
remain unchanged. Small logical viewports retain scrolling.

In the LP grid, Up/Down commit and open the next editable cell. Left/Right
move the caret within a value, then navigate at its edge or when text is
selected. Ctrl/Shift text navigation stays native. Arrow movement skips
read-only cells and stops at the grid boundary. Return/Tab retain row-major
movement; Shift+Return/Shift+Tab reverse it. Escape cancels an active edit.
Tests exercise active editors, stored values, boundaries, caret movement,
selected/unselected contrast and valid-but-tiny desktop fonts.

This preserves observed TORA terminology, panel order and teaching workflow.
Legacy screenshot coordinates are reference measurements, not Qt layout sizes.
The proposed clone uses C++17+, CMake and Qt 6 Widgets, with QPainter or
QGraphicsView plots. No browser, HTML UI or QtWebEngine is permitted.

## Screen navigation

`MAIN → Linear Programming → input mode → input grid → SOLVE/MODIFY`.
The shared Windows, Linux and macOS shell opens directly at MAIN; the Welcome
screen and its Click Here step have been removed.

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

The original shell supplied seven native screen frameworks: welcome, MAIN,
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

## Phase 2 all-slack screen

All-slack starting solution is enabled for a loaded LP. Its output-format panel
is editable and Go starts the all-slack session. `lp/simplex` is a native lazy
route with an accumulated QTableView, three observed iteration instructions,
Next/All/Write to Printer and the original footer navigation. Nonbasic original
slack columns in constraint rows use white/blue inverse cells; selected columns
are green and selected leaving rows red. Actual bold-font metrics determine
numeric widths; horizontal/vertical scrolling supports 100×100 tables and large
precision. Automatic steps yield to the event loop and pause on navigation.
Input changes discard stale tables. The exact table/notice strings and limits
are in tora-analysis/LP_ALL_SLACK_MODULE.md.

M/Two-phase warning handoffs now enter their functional setup/output routes.
Printing uses native QtPrintSupport; successful TORA print layout remains Q-08.

## Phase 2 M-method screen

M-method is now enabled alongside All-slack for a loaded problem. Its M-Method
setup group has an editable selected default 100 and a working Go button. The
common output panel starts the requested finite-M session and the accumulated
view uses SIMPLEX TABLEAU - (M-Method). All-<= warning OK and explicit Cancel
both continue to the All-slack output-format route, as directly captured.
All-slack's artificial-warning Cancel opens functional M setup; OK selects
Two-phase and enables its output Go.

The white/blue inverse block covers initial identity columns sx/Rx in constraint
rows, including still-basic columns; the earlier nonbasic-only description is
corrected by direct initial-table review. Native font/layout/scroll/printing
boundaries and original footer controls are shared. The Basic/row-label column
remains pinned while numeric columns scroll horizontally; vertical scrolling
and leaving-row clicks stay synchronized. Column widths come from font metrics,
and Solution stays visible when the numeric columns fit the viewport.
No original screenshots
or proprietary visual assets form part of the interface.

## Phase 2 Two-phase screen

Two-phase method is enabled for a loaded problem. It uses the common format
panel and SIMPLEX TABLEAU - (Two-Phase Method). Ordered headings are literally
Phase 1 (Iter N and Phase 2 (Iter N, as captured, with globally consecutive
iteration numbers. Phase 1 uses z (min); Phase 2 restores the original sense.
Artificial columns remain visible and show blocked only in objective cells.
Their constraint values, including zero-valued artificial basic rows, remain.

Manual Next shows Phase 1 / End of Phase 1: Feasible solution found--initiating
Phase 2 at the feasible boundary. Acknowledgement retains the final Phase 1
table; the following Next appends Phase 2. All Iterations suppresses this
intermediate message. Infeasibility uses the captured Phase 1 dialog and stops
before Phase 2. All-<= warning OK/Cancel both continue to All-slack. Native
table colors, pinned Basic labels, font metrics, printing, timer cancellation
and input invalidation are shared with the earlier methods.


Bounded simplex is enabled with a loaded LP document. It reaches output format
directly without an all-slack/artificial redirect warning or M entry. After
lower-bound/RHS normalization its heading is Starting All-Slack Method or
Two-Phase Method. The shared native bound/sign rows, complements, phase controls,
blocked artificial columns, printing and navigation apply. A captured initial
shifted-start infeasibility shows the Phase 1 modal immediately on output.


## Dual menu and iterations

`Solve Problem → Algebraic → Iterations → Dual simplex` is enabled with an input document. Equality rejection uses `Dual Simplex Error`. Direct eligibility goes to output format and the `SIMPLEX TABLEAU - (Dual Simplex Method)` view; user instructions stay the same as captured primal instructions. All-slack fallback has one OK acknowledgment. Artificial or bound/free fallbacks offer OK for Two-phase and Cancel for M value entry. Finite-bound warning explains that condition explicitly. Actual method determines heading and inverse-block coloring; direct Dual Sx inverse cells are white. Direct infeasibility uses `Entering Var` and `Problem has no feasible solution--Iterations stopped`. Other terminal and alternative notices use the shared captured wording.

For direct Dual, clicking a valid leaving row highlights only that row; a later entering click preserves it. After an alternative Next action, the new table appears without another optimum notice. Next stays disabled until a valid entering column is selected; All Iterations remains disabled at an optimum.

## Implemented graphical screen

Graphical is enabled for exactly two variables; upper/lower/free restrictions use the captured `Graphical LP` warning before format selection. The native screen retains model/results left, a square cyan graph right, zoom/print above and input/main/exit below. Row clicks add constraints, objective requires all constraints, the sign row resets and one stroke completes. Source colors are gray feasibility dots, blue axes/objective/status marker, red constraints and yellow labels. Terminal states omit the finite objective line.

Actual window dimensions are asserted for 1366×768, 1920×1080 and 3840×2160 at four scaling factors. Layouts expand to available space; the shell scrolls at small logical viewports. Number fields use the selected output notation while ticks remain integers. The exact text/zoom profile and native printing limitation are in LP_GRAPHICAL.md. Four native scale runs check all 35 result panels and 755 captured color positions each.


## Implemented final and sensitivity screen

Algebraic → Final solution → common Output Format opens the native output
summary. The grid presents variable values/contributions, original relation
labels and slack/surplus, then the spanned `***Sensitivity Analysis***` heading,
coefficient ranges/reduced costs and RHS ranges/dual prices. Blue/cyan colors,
ordered labels, selected number format and exact terminal messages follow
captures. Font metrics size rows and columns; the layout fills the window and
scrolls when content exceeds the viewport. Next/All Iterations remain disabled;
Write to Printer, View/Modify Input Data, MAIN Menu and Exit TORA are native
controls. Changes to input invalidate the completed numerical session.


## Verified printer controls and display-change acceptance

Printing is specified in `tora-analysis/LP_PRINTING.md`. Successful jobs start
with original data; trace printing repeats Basic in horizontal bands, while
final printing includes sensitivity. Infeasible/unbounded final states hide
Write to Printer. Existing default printers receive jobs directly. With no
default, Qt's native printer/PDF selection is an explicit usability improvement.
Cancellation does not modify input or solver state. Graph prints use monochrome
strokes/yellow labels while the native screen retains the captured TORA colors.

The live display regression covers four populated routes at three secondary
DPIs, a live scale change and display removal. Fonts, raster DPR, reachable
window geometry and model/session state are asserted. Test-only offscreen
monitor changes are distinguished from still-pending physical monitor acceptance.


## Native Transportation North-west and Least-cost preview

MAIN Menu → Transportation model opens existing-file/example choices. Open
uses a native picker and shows the cost/supply/demand grid read only. SOLVE
Menu → North-west corner start or Least-cost start opens stacked automatic tableaus. Each source
uses cost, shipment and reduced-cost rows, with u/v multipliers, supply, demand,
Name, ObjVal = and Iter labels. Basic zero shipments remain visible; nonbasic
cells are blank. Next adds one tableau, All adds the remainder, terminal
controls disable and the observed optimal-iteration message appears.

Shared readable colors and a minimum 12-point screen font follow the user's
accessibility direction. Font metrics/layouts drive sizing; native scrolling
handles smaller viewports. Tests check all 17 captured profiles at four scales
and exact window sizes for 1366×768,1080p and4K. A separate real-desktop harness
can inspect the entry/input/initial/terminal routes with `--transportation`.

The selected heading distinguishes North-West Corner Method from Least Cost
Method. Switching methods recomputes the trace and resets visible iterations;
failed solving leaves the current method and trace intact. Both method routes
have 17 independent golden profiles and three resolution checks at four scales.

The preview exposes only supported automatic behavior. New/edit/save, Vogel's
start, manual loop selection, multiplier changes, final summary and printing
remain outside this slice. Read-only input uses normal arrow-key selection.

## Future-scope Vogel preview

The Transportation Solve Problem menu and iterations chooser add Vogel start. Its output heading is `TRANSPORTATION TABLEAU - (Vogel's Method)`, verified from the reference. The shared Next/All, optimal notices, font-metric column sizing, scrolling and HiDPI behavior apply. Input remains read-only in this slice. Three methods are rendered at1366×768,1920×1080 and3840×2160 at100/125/150/200% scale. Final acceptance is recorded in STATUS.json.

Vogel final gate closes at source 6b01925, CI 38265173986: 82/82 per OS, 625 exact new-method fields, 71 Transportation checks/scale and 36 renders/OS. All-three-OS screenshots, package audits and actual installed Wayland identity pass. Evidence and retained limits are recorded in STATUS.json and TRANSPORTATION_VOGEL.md.


## Transportation editing continuation

The current future-scope source adds editable Transportation documents, native new/input initialization, aliases/costs/quantities/title editing, Open/read-only copies and atomic Save/Save As. Historical read-only descriptions above identify earlier deliveries. See [the editing contract](tora-analysis/TRANSPORTATION_EDITING.md) and STATUS.json for current verification. Unbalanced drafts are savable; solver profile restrictions remain. EditGrid structural operations, manual loops, multiplier choices, final summary and Transportation printing remain separate cycles. New source has not yet passed a three-OS/package/install gate.
