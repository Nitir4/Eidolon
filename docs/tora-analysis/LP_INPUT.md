# TORA LP input screen

## Purpose

Define a linear program before selecting graphical or algebraic solution.

## Observed controls

The input-mode screen has radio buttons for `Enter New Problem` and `Select
Existing File`; radio buttons for decimal/scientific notation; N and D fields;
and `Go to Input Screen`. The observed new-problem defaults were N=5 and D=2. A direct probe with N=10
and D=10 proceeded to the input grid; the accepted maxima for these notation
fields remain unverified. These N/D values control notation, not model size.

The grid header labels are `Problem Title:`, `Nbr. of Variables:`, and `No. of
Constraints:`. The table is headed `INPUT GRID - LINEAR PROGRAMMING`; its
columns are `Var. Name`, one column per variable, `Enter <, >, or =`, and
`R.H.S.`. Rows are the objective sense (`Maximize` or `Minimize`), constraints
`Constr 1` onward, `Lower Bound`, `Upper Bound`, and `Unrestr'd (y/n)?`. The
smoke case showed default lower bounds `0.00`, upper bounds `infinity`,
unrestricted flags `n`, padded `<=` relations, and two decimal places.

The page has `File` and `EditGrid` menus and buttons `SOLVE Menu`, `MAIN Menu`,
and `Exit TORA`. The visible editing instructions say to use Return or Tab to
initialize the grid; change objective sense by editing its cell; and, for
delete/insert/copy/paste, click the target row or column heading and use
`EditGrid`. In insert mode, a single or double click selects where a row or
column is placed. A selected constraint row turns red. Its `EditGrid` menu shows, in order:
`Undo Last Insert, Paste, Or Delete` (Ctrl+U), `Copy Column or Row` (Ctrl+C),
`Paste on Column or Row` (Ctrl+P), `Delete Column or Row` (Ctrl+D), and
`Insert Column or Row` (Ctrl+I). Undo and Paste were disabled in this state;
Copy, Delete, and Insert were enabled. These are observed menu labels and
advertised shortcuts; command execution through the menus is recorded below.
Evidence: `reference/tora-captures/probes/input-menus/01_normal/editgrid-row-selected.png`.

The `File` menu shows `Save`, `Save As`, `Main Menu`, and `Exit TORA`, all
enabled for the loaded normal fixture. There is no Open or Print item in this
input-screen menu. Evidence: the neighboring `file-menu.png`.

## Direct size-limit probe

A 100-variable, 100-constraint blank model initialized successfully. Entering
101 for both counts displayed a modal titled `Problem size` with the exact text
`Student version limits maximum size to 100`. The TORA screenshots are
`reference/tora-captures/probes/dimensions-n100-d100/after-grid-initialize.png`
and `reference/tora-captures/probes/dimensions-n101-d101/after-grid-initialize.png`.
Thus the observed Student-version model limit is 100 variables and 100
constraints. These probes did not attempt to solve the large models.

## File interaction

Selecting an existing file opens a native file picker with `Files of type: Text`
and an `Open as read-only` checkbox. The picker listed `smoke.txt` and reopened
it with identical displayed fields.

## Unknowns / things I could not verify

The accepted maxima for input-format N/D, arbitrary malformed-cell messages,
variable-name limits, inconsistent bounds, unrestricted expansion and every
combination of bounds/signs remain unverified. All three relations, negative
coefficients/RHS, finite and infinite bounds, and saving an unrestricted y
are represented in captured files. Saving y does not verify its solution
transformation. Scientific zero initialization and output are independently
captured; see `LP_INPUT_MODE.md` and `LP_OUTPUT_FORMAT.md`.

## Menu command execution

With explicit window focus and a constraint heading selected, Copy then Paste
replaced Constraint 2 with Constraint 1's 1.00/1.00/<=/4.00 values. Counts
stayed 2/3. Delete removed Constraint 2, reduced its count to 2 and relabeled
the old third row. Insert created a zero row before the selected target,
restoring the count to 3 with 0.00/0.00/<=/0.00. Source model files were not
overwritten. Evidence and hashes: `probes/closing-20261004/grid-focused/`.

Undo attempts did not restore Paste or Delete; the menu item remained gray.
The state needed to enable it remains unknown. An earlier unfocused probe
produced an empty paste and is not evidence of normal copy/paste behavior.
Column operations and successful keyboard shortcuts remain unverified.
