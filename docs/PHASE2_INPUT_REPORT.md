# Phase 2 — LP data entry gate report

The first Phase 2 module implements native LP input and file/controller bindings.
New/Existing, title/count entry, decimal/scientific settings, the editable table,
objective toggling, aliases/relations/bounds/sign flags, row/column commands,
native Open/Save, read-only copies and Save? cancellation now work. Solver
commands remain disabled for their separate module cycles.

## Validation

- Tests preceded the controller/ordered-cell APIs. Free-variable and extended
  precision tests failed against the previous code before their corrections.
- Local desktop suite: 20/20; explicit Qt-free C++17 suite: 10/10.
- All 27 imported saved-file grids are compared field by field, reusing the
  existing 15 normal/abnormal LP models. Completed solver cases were not rerun.
- 22 input-operation outputs match actual TORA saved-file SHA-256 exactly,
  including formatted spellings, aliases/quotes, row/column order, sign/bounds,
  reinitialization and precision 11/99.
- Keyboard Return/Tab, reserved-cell traversal, real Qt read-only picker,
  save cancellation/failure and navigation cancellation are exercised. Wrong
  module marker Yes/No/retry-picker Cancel matches the captured warning;
  Existing Cancel leaves blank input. Pending title Save, retained last Save
  destination and dialog recovery from malformed theme fonts are covered.
- Normal/100×100 grids are exercised at 1366×768, 1920×1080 and 4K, at
  100/125/150/200% scaling. Original clone screenshots are produced under
  build/phase2-input/ui-captures/. Reference images/files remain private.
- Three new clone-written edited files (alias, unrestricted bounds and relation)
  opened, saved and reopened in TORA with identical bytes. No completed bridge
  was repeated. Private frames are under probes/foundation4/clone/.
- Final [three-OS CI run](https://github.com/Nitir4/TORA-Clone/actions/runs/37658262696)
  verifies source `aaf0450032d30ee3670c5ba75e43ac5054aec84a`: Windows,
  Linux and macOS each pass 20/20 CTest checks (60/60 overall), with no
  failures or skipped checks. Downloaded complete input logs confirm 43/43
  cases at each of four scales per OS, including all 22 oracle outcomes.
- Windows initially failed the new offscreen glyph check; the input test
  now configures the installed font directory like the existing shell/font
  tests. Full per-scale logs are retained instead of relying on clipped
  CTest console output. The final Windows run passes the font check.
- Packaging, deployed startup and upload pass on all three OSes: portable
  Windows EXE ZIP, Linux x86_64 AppImage, and universal arm64+x86_64 macOS
  app-in-DMG. Final reports are under build/phase2-input/ci-final-verified/;
  packages are under dist/phase2-input/final/.
- All three downloaded payloads match their SHA-256 sidecars; the Windows
  ZIP passes its integrity check. The downloaded Linux AppImage passes a
  local offscreen extract-and-run startup check. Checksums are recorded in
  build/phase2-input/packages-verified.json.

## Compatibility findings and limits

Numeric UI entry rounds at the selected precision; loaded records retain their
precision. Entering y sets lower -infinity/upper infinity. Delete leaves a
two-variable grid unchanged but deletes three to two; Insert adds after the
selected heading. Undo is disabled in captured states. SOLVE rejects all-zero
LHS rows, while MAIN/Save can retain them.

The approved quote-recovery policy protects aliases too: TORA saves them but
corrupts its own reopened grid. The native profile supports N/D up to 99; 999
caused legacy overflow/exit. Exact larger precision extrema, other code pages,
successful Undo/printing and physical mixed-monitor moves remain unverified.
No known comparison mismatch is moved into a solver cycle; engineering safety
policies are identified separately from observed TORA wording.

See [input specification](tora-analysis/LP_DATA_ENTRY_MODULE.md),
[traceability](TRACEABILITY.md) and [open questions](OPEN_QUESTIONS.md).
The input module is ready for its review gate. Phase 2 as a whole remains
in progress; the next separate module is simplex with the all-slack starting
solution, after approval.

## Review on the laptop

From the project folder, run `build/dev/src/desktop/ToraClone`. Choose Click
Here → Linear Programming → Go to Input Screen, initialize counts with
Return/Tab, then edit and save. Select Existing File to open a TORA LP file.
The original native 1920×1080 input screenshot is
build/phase2-input/ui-captures/input/normal-1.png. Numeric/file comparisons
are summarized in build/phase2-input/comparison.tsv; no raw TORA capture is
included in a package or the repository.
