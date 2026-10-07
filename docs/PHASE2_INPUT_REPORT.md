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
- Three-OS CI: pending final verification.

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
The next separate module is simplex with the all-slack starting solution.
