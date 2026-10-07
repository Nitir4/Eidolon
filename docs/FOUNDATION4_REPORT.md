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
The final [CI run](https://github.com/Nitir4/TORA-Clone/actions/runs/37611008110)
tested source commit `ab04734bf3628f55ad84fdd9d6b807cb812b6704` with Qt 6.10.3:

| OS | Tests | Packaging and deployed startup |
| --- | --- | --- |
| Windows | 15/15 | Portable EXE folder ZIP passed |
| Linux | 15/15 | AppImage passed |
| macOS | 15/15 | Universal arm64+x86_64 APP in DMG passed |

**45/45 CTest tests passed overall**, with no failures or skipped tests.
Downloaded XML at `build/foundation4/ci-final/Test-results-{os}/test-results.xml`
independently confirms all 27 file expectations and saved-byte hashes on each
OS. Only the clone's original screenshots accompany those reports. Native
packages and dependency/checksum manifests were uploaded in the same run.
All three packages were downloaded to `dist/foundation4/` and their SHA-256
checks passed. Windows ZIP integrity also passed. The Linux transfer timed out
once and succeeded on retry; its CI package/startup checks remained successful.

The final native-picker captures also confirm `.txt` suffix completion,
subsequent Save targeting the latest Save As destination, and overwrite No /
picker Cancel retaining the original bytes. The comma-only and Windows-1252
title reopen correctly; the quote-only title independently isolates TORA's
reader bug. Quoted-alias commitment remains an explicit Phase 2 input-editor
question. These observations are specified in FILE_BEHAVIOR.md; no unverified
alias behavior is claimed.

The native app remains the reviewed navigation preview. The file library will
be connected to enabled Open/Save and grid editing in the Phase 2 data-entry
module; those actions are not claimed complete by this foundation gate.

See [file specification](tora-analysis/LP_FILE_IO.md),
[format](FILE_FORMAT.md), [traceability](TRACEABILITY.md), and
[recorded limits](OPEN_QUESTIONS.md).

**Foundation status: complete.** All five foundations are implemented and
verified in the approved order. Phase 2 has not started. The next approval is
the phase boundary, followed by one module cycle for LP data entry and its
native file/controller bindings. Physical mixed-DPI hardware, additional legacy
code pages, the quoted-alias grid workflow and printing remain explicitly
recorded acceptance/module limits; this gate does not claim they were verified.
