# TORA LP file behavior observed

## Purpose and workflow

LP files hold the input model. The input screen's File menu offers Save,
Save As, Main Menu, and Exit TORA. Existing files are opened through MAIN →
Linear Programming → Select Existing File → Go to Input Screen; the input
File menu has no Open command. Leaving the input grid asks `Do you wish to
save data?` even for a loaded fixture. Choosing No returns to the solve/menu
workflow. Save As opens the native file picker with file type `Text`.

## Direct save/reopen evidence

All fifteen corpus fixtures have now been saved using TORA's own Save As,
read as bytes, and reopened successfully. Each output is byte-identical to
its staged input. Four further direct round trips vary dimensions (1×1 and
3×2), named variables, finite bounds, unrestricted flags, precision, and
scientific numeric strings. Captures and byte comparisons are in
`reference/tora-captures/roundtrips/<case>/`.

The smoke file is 281 bytes; the normal fixture is 271 bytes. Files use CRLF
records, quoted titles/sense/grid cells, and unquoted settings/counts. The
first grid row holds variable names: the 1×1 probe displays `decision` in its
Var. Name row while the column remains x1. The schema and lexical preservation
findings are specified field-for-field in `docs/FILE_FORMAT.md`.

TORA preserves existing numeric strings through these saves; it does not
round the stored `1.234567` to two places merely because D=2. It preserved
scientific strings and an input-format mode record of 2. Two fresh models
created through the notation radios confirm that scientific writes 2 and
decimal writes 1; their default zero strings are 0.00000e+00 and 0.00.
Evidence: `probes/targeted-20261004/new-scientific/` and `new-decimal/`.

## Unknowns / things I could not verify

Foundation 4 resolves the observed suffix, read-only, text and subsequent-Save
paths below. Quoted-alias input commitment, other code pages and arbitrary
invalid counts/cells remain unverified. Four specific malformed-file observations are recorded below;
they do not establish recovery behavior for every malformed input.
Only saved bytes and rendered fields are examined; no executable analysis is
used. Clone-written compatibility checks are recorded at Foundation 4.


## Printer response in this oracle environment

Clicking `Write to Printer` on the normal initial tableau opens a `vbTora98`
runtime error dialog: `Run-time error '482':` followed by `Printer error`.
Acknowledging/cancelling the dialog ended the TORA process under this Wine
configuration. Evidence: `probes/targeted-20261004/printing/write-to-printer.png`.
No functioning printer is available in this oracle environment, so a physical
printout, print pagination and the normal Windows print path remain unverified.
This is an environment limitation; it is not a proposed clone failure behavior.


## Wrong module marker

Replacing only the first record (`3000`) with `9999` produces a dialog titled
`Open FileSave As Warning` with `Target file does not match format of elected
TORA module. Retry?`, and Yes/No buttons. Yes retries the picker; cancelling
it returns to a blank input form with Save/Save As disabled. The existing
file title can remain in the window title even though the model grid is blank.
Evidence: `probes/file-errors-20261004/wrong-marker/after-open.png` and its
attempted-input bytes. The word `elected` is the observed spelling.


## Additional framing probes

A 15-record truncated version of the normal file loaded its title and counts
(2 variables, 3 constraints), but left every grid data cell blank; no error
dialog appeared in the captured open/acknowledge states. An extra final quoted
record was accepted and the normal model populated unchanged. An LF-only
version left title/count fields blank and created no grid, again with no
dialog in the captured states. All three processes remained running.
Evidence: `probes/file-errors-20261004/{truncated,extra-record,lf-only}/`,
including attempted bytes and before/after acknowledgement screenshots.
Do not infer that the malformed files are safe to solve, or that all truncated
lengths behave identically. The successful compatible framing remains CRLF.

## Foundation 4 text and read-only observations

TORA saves interior quotes doubled. The directly saved title `F4, "quoted" cafe`
reopened as `F4,` with zero variables/constraints; no successful legacy round
trip is claimed for that file. The user approved recovery on 2026-10-07. The
clone restores the complete saved data and preserves its original bytes.
The combined text attempt reset the grid after editing an alias, so that
capture does not establish quoted-alias behavior.
Evidence: `probes/foundation4/text/text/{text-entered,reopened}.png` and saved
file SHA-256 `a59456ae6d4fe71e71d18c1e50a4dc1547069c2e7afcec55c182deb6ef220c16`.

Selecting Open read-only still allows input/title edits; File Save and Save As
remain enabled. Save opens a native Save As picker and leaves the original
file unchanged. Save As writes the edited title and original full model to a
new file. Its window caption can still show the old source; the subsequent Save
target is established by the separate follow-up below. Normal editable Save
also opens this picker, so that dialog is not unique to read-only mode.
Evidence: `probes/foundation4/readonly/readonly/readonly-selected.png`,
`file-menu.png`, `after-edit.png`; `readonly-save/readonly-save/after-save-command.png`,
`source-after-save.txt` and `tora-saved.txt`.

The reference helper observed Windows ANSI code page 1252 through the OS API.
Entering `F4 cafe é`, saving and reopening verified byte E9 and the same displayed
title/counts. The codec preserves title/alias bytes opaquely; native presenters
must convert those legacy fields with the selected code page. Unicode filesystem
paths are independent. Capture: `probes/foundation4/unicode-title/unicode-title/`;
saved SHA-256 `bc5592afde34a48fbb4aac3131e025b6302066514c441118ceba1c3a5113fbde`.

The comma-only title `F4, comma` reopens without corruption. The quote-only title
`F4 "quoted"` reopens as `F4` with 0 variables/2 constraints and misplaced grid
values; its saved header still contains 2 variables/3 constraints. These isolate
quotes as the problem. Both captured saved payloads are in the exact-byte
goldens; the user-approved recovery handles the quoted one.
Evidence: `probes/foundation4/{comma-title,quoted-title}/`.

The Text picker appends `.txt` when given `without-suffix`. Save after Save As
opens the same native picker with the previous filename selected. Accepting Save
and Yes changes that Save As target; the original source stays byte-identical.
The window caption continues to show the original source. The warning has an
empty caption and the two-line body `File already exists.` / `Do you want to
replace it?`, with Yes/No. Capture: `probes/foundation4/extension/extension/` and
`save-target/save-target/{after-save-command,subsequent-overwrite,after-confirmed-save}.png`.
Thus picker behavior belongs to the controller; the file store's source path
represents the actual saved destination independently of the legacy caption.

Choosing No in the overwrite warning returns to the Save As picker. Cancelling
that picker leaves the existing file byte-identical to its original contents.
A later Save As to a different destination succeeds with the edited title
`F4 overwrite changed`. Capture: `probes/foundation4/overwrite/overwrite/`,
including `overwrite-prompt.png`, `after-no.png`, the retained `tora-saved.txt`
and the new `accepted-new.txt`. This is dialog/controller behavior; the store
only writes after the controller has accepted a destination.
