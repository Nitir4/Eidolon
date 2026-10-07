# Foundation 4 gate report

Foundation 4 adds the Qt-free LP document, TORA text codec and binary file store
in the approved order **1 → 5 → 2 → 3 → 4**. Work stops for review before the
first Phase 2 module.

Implemented: all observed header/grid fields, semantic dirty/saved checkpoints,
source paths/read-only mode, preserved numeric lexemes/reserved records,
lossless changed coefficients, typed diagnostics, and transactional native-path
save/replacement. No GUI or solver dependency enters `tora::compat`.

The user approved recovery of doubled quotes from TORA's own saved records,
after a direct capture showed its reader corrupting the title and dimensions.
The clone recovers that saved data instead. This exception is documented and
tested separately from successful compatibility comparisons.

Tests were derived from completed captures before implementation. Twenty-seven
actual TORA saves pass ordered-field, exact-byte and recorded-SHA-256 comparisons.
A local run also compares the actual private files directly. No completed
Phase 0 corpus sweep was repeated. New file-specific captures remain private.

Local Debug: **15/15 passed**, including all previous GUI scale/font tests.
Qt-free explicit C++17: **9/9 passed**. Three clone-written files were opened,
saved and reopened in TORA with identical bytes: named/finite-bound input,
17-digit changed objective and fresh scientific input. Each reopened screen
was reviewed. The coefficient/scientific strings clip in TORA's narrow cells;
the saved bytes independently prove complete precision. Bridge SHA-256 values:

| File | TORA Save As SHA-256 |
| --- | --- |
| named-roundtrip | `8e0f06aed579442dd172969af7cc2f371c3a0bcc307036ea0730703f75cf4395` |
| precision-edited | `f64af249cc085bf05a36db13f165ada8b0ab55e22f3eeaeb97f3b4dbc6beb324` |
| fresh-scientific | `5b0f0b1dbc0b35f82e5c6eed124b71c56a5e1b2a8285a5de295bfdf0715f05f3` |

Host Wine/Wayland input was flaky. The remaining captures moved to a dedicated
800×600 nested Xwayland display and private Wine runtime, using actual TORA UI
and native X screenshots. This removes host desktop/focus dependencies; no
completed corpus was rerun and no TORA instance was deliberately killed.
The three-OS CI run is pending and must be recorded before this gate closes.

The native app remains the reviewed navigation preview. The file library will
be connected to enabled Open/Save and grid editing in the Phase 2 data-entry
module; those actions are not claimed complete by this foundation gate.

See [file specification](tora-analysis/LP_FILE_IO.md),
[format](FILE_FORMAT.md), [traceability](TRACEABILITY.md), and
[recorded limits](OPEN_QUESTIONS.md).

**Gate status: in progress.**
