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

Automatic filename suffixes, Save overwrite/cancel behavior, read-only editing,
invalid counts/cells, quote/comma escaping, and non-ASCII encoding remain
unverified. Four specific malformed-file observations are recorded below;
they do not establish recovery behavior for every malformed input.
Only saved bytes and rendered fields are examined; no executable analysis is
used. Clone-written compatibility will be tested at Foundation 4.


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
