# Transportation input and files

Current working source: editable documents, new/default grid initialization and Open/Save/Save As are locally verified. See TRANSPORTATION_EDITING.md and ../STATUS.json for the current gate. Earlier read-only observations below describe the initial preview; unsupported formats, structural editing and broader original validation remain unverified.

## Input selection and grid

Observed choices: Enter New Problem, Select Existing File, Decimal Notation,
Scientific Notation, N=5, D=2, Go To Input Screen, MAIN Menu, Exit TORA.
The blank screen contains Problem Title, No. of Sources, No. of Dest'ns,
File and EditGrid menus. SOLVE Menu is disabled before grid initialization.
Entering two sources and three destinations and pressing Return initializes
INPUT GRID - TRANSPORTATION.

Columns are D1, D2, D3, Supply, with a separate S/D Name row and source-name
column. Rows are S1, S2, Demand. New costs display 0.00; quantities and aliases
are initially blank. The observed pilot accepts costs, supplies and demands
through the grid editor. Native Open reopens the saved model correctly.
No maximum dimension or complete validation rule is established yet.

## Observed file representation

The reference writes `.txt` files with CRLF line endings. The balanced 2×3
pilot begins with these seven records:

```text
4000
5
2
1
"REA transportation pilot"
2
3
```

These are followed by twenty quoted strings, a 4×5 row-major grid:

```text
S/D Name | destination-name 1 | destination-name 2 | destination-name 3 | blank
source-name 1 | cost 11 | cost 12 | cost 13 | supply 1
source-name 2 | cost 21 | cost 22 | cost 23 | supply 2
blank | demand 1 | demand 2 | demand 3 | blank
```

4000 is the observed Transportation discriminator. N/D settings are plausible
interpretations of records 2/3, but require targeted setting changes to confirm.
Record 4's meaning is unknown. Records 6/7 match source/destination counts.
The preview accepts this exact default header rather than generalizing its
settings mapping. Quotes/commas/encoding behavior in TORA is not established.

A model entered through the UI saved costs as "2.00" etc. A generated
candidate reopened in TORA and saved through its native Save As retained "2"
etc. Thus the source file's lexemes can survive a save and are not necessarily
equal to display formatting. Generated candidate files are kept separately
from authentic `tora-saved.txt` outputs.

## Save/navigation

SOLVE Menu can display “Do you wish to save data?” with Yes, No and Cancel.
Yes opens the native text-file save picker. Successful save then opens
SOLVE/MODIFY. File → Save As was verified after reopening an existing model.
View/Modify Input Data returns from output to the input grid.

## Unknowns / things I could not verify

Full File/EditGrid menu states and shortcuts; insert/delete/copy/paste behavior;
save enablement during active editing; unsaved New/Open/Exit behavior; read-only
files; aliases and quoting; Unicode/code page; source lexeme preservation after
editing/resizing; trailing/reserved records; malformed/truncated files;
scientific notation and dimension/quantity limits; print/export behavior.

## Native read-only reader

`transportation_compat` parses the observed 4000/default-header CRLF records,
exact grid size, finite numbers and nonnegative quantities. Independent test
files are generated from model data and required to hash exactly to the
seventeen authentic TORA saves before being opened by the reader. Raw saved
files are not copied into the public tree. No writer is implemented.

Native engineering safeguards are 4 MiB and 1–100 sources/destinations; these
are not claimed original limits. Malformed/truncated/extra records and unknown
notation headers produce explicit errors without replacing the current model.
Doubled quote/alias handling has native safety coverage, but its TORA semantics
are still unverified. ASCII is the verified corpus encoding; other legacy code
pages are not accepted as verified compatibility. New/edit/save remain future
work; the native input grid is read only and retains arrow-key selection.

The native preview explicitly rejects non-ASCII legacy text until its code page
is verified, and caps a displayed trace at250,000 cells to bound widget allocation.
These are preview safeguards, not original TORA limits.


## Transportation editing continuation

The current future-scope source adds editable Transportation documents, native new/input initialization, aliases/costs/quantities/title editing, Open/read-only copies and atomic Save/Save As. Historical read-only descriptions above identify earlier deliveries. See [the editing contract](TRANSPORTATION_EDITING.md) and STATUS.json for current verification. Unbalanced drafts are savable; solver profile restrictions remain. EditGrid structural operations, manual loops, multiplier choices, final summary and Transportation printing remain separate cycles. New source has not yet passed a three-OS/package/install gate.
