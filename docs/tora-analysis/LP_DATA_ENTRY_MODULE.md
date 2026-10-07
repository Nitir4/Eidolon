# Phase 2 LP data-entry module

Phase 2 authorized 2026-10-07. This first cycle covers input mode, title/counts,
the editable grid, EditGrid, native file pickers and save-on-leave navigation.
Solver, graphical, sensitivity and printing implementations have separate gates.
The completed Phase 0 solver corpus was reused, not rerun.

## Sources, tests first and reference environment

The 15 corpus input models and all 27 Foundation 4 file expectations are reused.
Shared specifications: LP_INPUT.md, LP_INPUT_MODE.md, LP_FILE_IO.md and
FILE_BEHAVIOR.md. Focused captures are private under
reference/tora-captures/probes/phase2-input/<case>/. Independent hashes in
tests/fixtures/tora-input-edits.tsv cover 22 saved outcomes. Original expected
field changes below, together with those hashes, form the goldens.

The observer launches/reuses actual TORA 1.00 under Wine 11.18 on an isolated
800×600 Xwayland display. Captures do not depend on the host workspace/cursor.
No process is deliberately killed. N=999 caused TORA's own runtime error 6
and exit after acknowledgment; the next observer restarted it. That failed
boundary attempt is not a successful capture. Completed outcomes are skipped
on resume. Real keyboard events are required for VB grid handlers; direct
WM_CHAR injection produced invalid attempts, which are not oracle results.

Tests first: desktop_input_test initially failed to compile because its
controller did not exist; compat.grid failed before ordered-cell APIs existed.
Negative-infinite free-variable tests failed in lp.validation/compat.errors
before the correction. Precision-11 fixtures failed in core.formatting before
the supported profile was extended. All were then implemented and rerun.

## Inputs, grid order and entry behavior

Input mode has New/Existing, Decimal/Scientific and N/D defaults 5/2. Existing
first opens blank input, then the Text picker with Open as read-only. Cancel
leaves blank input with the grid hidden and SOLVE disabled. New enables title/count entry;
SOLVE Menu stays disabled until initialization. Return/Tab in the constraint
count initializes/reinitializes the entire grid, including unchanged counts.
Counts 1..100 and exact invalid/Student-limit messages remain those observed
in Foundation 3. Imported counts/title populate the fields.

The grid has n+3 columns: description, x1..xn, Enter <, >, or =, R.H.S.
There are m+5 rows: Var. Name, Maximize/Minimize, Constr 1..m, Lower Bound,
Upper Bound, Unrestr'd (y/n)?. Labels and reserved cells are not editable.
Aliases stay separate from x headers. Fresh numeric cells use the selected
formatter, lower zero, upper infinity, flags n, relations padded <=.

| Capture | Operation and observed stored outcome |
| --- | --- |
| numeric-replace-keyboard | Home/Shift+End replaces objective x1 with 1.2345678901234567; Tab stores/displays 1.23 |
| numeric-invalid-replace | Alphabetic bogus is filtered from the numeric editor; empty commitment writes 0.00 |
| numeric-prefix | Typing 3junk leaves editor text 3 and saves 3.00 |
| objective-toggle | A single objective-label click switches Maximize to Minimize |
| alias-keyboard | Typing decision and Return stores decision; next alias editor opens |
| quoted-alias-keyboard | named, "x" is stored with doubled quotes, preserving comma/space |
| relation-replace-keyboard | > normalizes to >=, stored without untouched relations' padding |
| flag-replace-keyboard | Y becomes y; lower -infinity, upper infinity are stored |
| flag-restore | N restores n, lower 0.00, upper infinity |
| column-copy-paste | Copy x1/Paste x2 copies alias/objective/constraints/bounds/sign; Constraint 3 becomes zero/zero |
| column-delete-second | Delete in a two-variable grid leaves it unchanged |
| column-delete-three | Insert x3/Delete x3 returns exactly to the original two-variable file |
| column-insert-menu | Insert at x2 adds a zero/default column after x2 |
| undo-menu-state | Insert after Constraint 2 adds a zero/default Constraint 3; Undo remains disabled |
| count-return, count-tab | Reinitialize 2×3 model to defaults, preserving Normal title/settings |
| format-n11, format-d11 | Both 11 settings are accepted/saved; D=11 zero is 0.00000000000 |
| format-d11-value | 1.2345678901234567 commits as 1.23456789012 |
| format-scientific-n11 | Scientific zero is 0.00000000000e+00 |
| format-n99, format-d99 | Both 99 settings initialize/save; D=99 writes 99 fractional zero digits |

Imported lexical precision is preserved until that cell is edited. Core/file
storage does not globally round a loaded model. Edited formatted text is parsed
and stored, retaining trailing zeros exactly. Return/Tab commit and move through
editable cells, skipping labels/reserved cells. Native editors filter numeric
letters and normalize relation/sign text; completed values change transactionally.

## Editing commands and state

Select a row description or variable heading, then EditGrid. The menu order and
shortcuts are Undo Ctrl+U, Copy Ctrl+C, Paste Ctrl+P, Delete Ctrl+D, Insert Ctrl+I.
Copy/Paste use matching axes and include all ordered cells. Selection is red.
Constraint Delete shifts/relabels rows; Insert adds defaults after the selected
row. Real Ctrl+C/P/D/I and menu commands were exercised. The earlier Phase 0
statement that insertion was before the selected row is corrected: its
coordinates selected Constraint 1, and the new row followed that selection.
Focused cases disambiguate the operation; tests assert resulting numeric order.

Undo was gray and Ctrl+U did not restore captured Paste/Delete/Insert states.
It stays disabled for these states; no successful Undo is invented. File Save
and Save As are enabled on an unselected loaded grid, gray during active grid
editing/heading selection. Structural commands open the next relevant editor.
Clipboard/document/grid are module-owned, with no solver state.

## Validation and navigation

SOLVE Menu first rejects an all-zero constraint LHS. The retained
column-copy-paste/save-unavailable.png shows Input Data Error, body:

    All Left-hand side coefficients of Constraint 3 are
    zero.  Reenter or delete constraint

MAIN and File Save can preserve that model. Bound consistency remains a method
selection check, not an unconditional Save rejection: reversed bounds are savable.

Leaving initialized input, even an unchanged loaded file, shows Save?,
Do you wish to save data?, Yes/No/Cancel. No retains the model for View/Modify;
Yes invokes the save picker; Cancel, picker cancellation or failure keeps input.
The same guard applies to window close. Pending table edits commit before
validation/save; constraint-count Return/Tab reinitializes the grid.

## Files and native text

Save and Save As both offer the captured picker workflow, prefilling the current
actual destination. The save picker supplies .txt suffix and overwrite checks;
cancellation does not write/checkpoint. Loaded source caption remains after
Save As, while future Save targets the new destination. A fresh input context
retains the previous caption and last save destination. Direct failed Open
preserves the previous document transactionally; choosing Existing explicitly
starts a blank context before the picker. A wrong module marker shows the
captured `Open FileSave As Warning` / `Target file does not match format of
elected TORA module. Retry?` with Yes/No. Yes repeats the picker; No or picker
Cancel leaves the blank context. Source: file-errors-20261004/wrong-marker/after-open.png.
Read-only permits editing and saving a selected copy.

Qt dialogs/widgets and OS save dialogs are native desktop UI. The Open picker
uses Qt Widgets to include its read-only checkbox consistently across OSes.
Dialogs receive the same malformed-theme font recovery as the screens.
No browser or HTML is used. The observed text profile is Windows-1252. Opaque
legacy bytes convert to/from QString; unsupported characters receive an
engineering diagnostic without replacing data. Unicode filenames are independent.
The ordered-cell codec API retains unrelated lexemes, reserved records,
source/read-only metadata and the saved semantic checkpoint.

TORA corrupts its own quoted-alias file on reopen: names become named, and x,
then numeric fields shift into relation/RHS columns. Direct capture:
reopen-quoted-alias/reopened.png. The approved doubled-quote data-recovery
policy applies to this same quoted-field grammar. The clone retains the full
alias/model. This exception is not claimed as a successful legacy reopen.

## Native layout and extension boundary

LpInputController/LpGridModel persist across cached LP routes. QTableView uses
cyan labels, blue numeric cells/white bold values, native white editors and
separate x headings. Panel/row/column sizes derive from fonts and layouts.
The blue canvas fills the resizable window. Large grids scroll internally;
the outer screen scroll area keeps controls reachable on small logical viewports.
Metrics refresh after DPI/font events. No 800×600 product geometry or raster
UI is used. Physical mixed-monitor handoff remains hardware acceptance work.

ModuleDescriptor supplies a generic beforeLeave callback. The shell invokes
its owning module's guard without acquiring LP/model/codec dependencies.
Future modules supply their own controllers/guards through the same interface.

## Unknowns / things I could not verify

Exact successful N/D extrema between 100 and captured 999 overflow are unknown.
The bounded native profile is Decimal N=1..99, Scientific N=0..99, D=0..99;
safe diagnostics replace captured process crashes. This engineering envelope
is not asserted to be TORA's maximum. Unprobed malformed/clipboard combinations
use transactional engineering rejection. Only Windows-1252 is claimed.
No successful Undo state or pending insertion mode was observed. Additional
accelerators, arbitrary alias limits, printing and physical mixed-monitor
moves remain unverified. Solver/output overflow and free-variable preprocessing
retain their separate later gates. See OPEN_QUESTIONS.md.
