# LP input-mode screen

## Purpose and navigation

The MAIN menu's Linear Programming entry opens this screen. It decides whether
to initialize a new model or open an existing Text file and selects input
notation. `LP_INPUT.md` separately specifies the initialized grid.

## Controls and observed values

- Select Input Mode: Enter New Problem (default), Select Existing File.
- Select Input Format: Decimal Notation (NNNNN.DD) (default), Scientific Notation
  (N.NNNNeDD).
- How many N? default 5; How many D? default 2.
- Go to Input Screen, MAIN Menu, Exit TORA.

New opens a blank input form with title and variable/constraint counts. The
Solve button remains disabled until a grid is initialized. Existing opens the
native Text picker; the model dimensions come from the file, not N/D controls.
The scientific and decimal radios were used to create and save fresh models,
confirming mode records 2/1 and initial zero strings 0.00000e+00/0.00 with
N=5/D=2. Evidence: `probes/targeted-20261004/new-scientific/` and `new-decimal/`.

## File and keyboard behavior

Existing files open through this screen, rather than an Open command on the
grid's File menu. Saved counts and title populate the next screen. See
`FILE_BEHAVIOR.md` and `FILE_FORMAT.md` for successful and malformed-file probes.
Keyboard defaults/accelerators were not established for this screen.

## Unknowns / things I could not verify

Input notation N/D extrema and validation, settings persistence between unrelated
new problems, and focus/Tab traversal. The observed notation strings do not
establish all rounding or width-overflow behavior.
