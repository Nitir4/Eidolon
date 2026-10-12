# Transportation editable documents and file writing

This cycle continues the delivered Transportation starts on `future-scope`.
It adds new grids, editable titles/aliases/costs/quantities and native Open,
Save and Save As. The earlier read-only delivery remains a historical release;
this source has its own verification record in `docs/STATUS.json`.

## Observed contract and independent evidence

The dedicated TORA reference uses ordinary UI actions and native file pickers.
No executable inspection, resource extraction or raw reference publication is
part of this work. Existing solver captures are reused.

A fresh 2-source/3-destination grid has `0.00` costs and blank aliases,
supplies and demands. Entering `1.234567` into a cost and committing gives
`1.23`. SOLVE displays `Save?` with `Do you wish to save data?` and Yes/No/Cancel;
Yes opens the native save picker. Its authentic saved-file SHA-256 is
`e214b88ad4a59cbb94d8e60144762ac16706e8e41c25a02f2ed77735be5a8338`.
The independently transcribed scenario in `tests/fixtures/transportation-input.json`
checks this exact hash against clone-created bytes. It contains no raw TORA file.

A clone-written balanced file, with its first cost changed to `1.23`, was
opened and saved by TORA. Both byte sequences hash to
`5780c617030858c85a81f31cd55aa8b04bcfaa6a4fc5a467a7a6ba377236cd26`.
Private provenance is under
`reference/tora-captures/transportation/input-editing/`;
the successful bridge is `actions-bridge-cost-corrected`, REA evidence
`ev_449f9a937895b0e5c993a8e702532e6083f20ca54963dda1aaf90e2b9ec68247`.
Failed navigation/editor attempts are retained separately and do not count as
successful compatibility checks.

A separate quantity probe saved integer `20` as `20` and `30.125` as `30`.
A discriminating probe saved `20.5` → `21`, `21.5` → `22`, `30.75` → `31`,
and `30.125` → `30`. Supplies and demands therefore use nearest-integer
commitment with observed positive half ties away from zero, separately from
cost D=2 formatting. Their independently saved hashes are in the same input
fixture. Untouched imported fractional quantities keep their original lexemes.
The last attempted `0.5` stayed in TORA's editor and did not enter its saved
file; it is excluded from the committed scenario. The clone explicitly flushes
live editors before saving, including the last cell.

## Design and execution flow

`TransportationDocument` in `src/compat/transportation_document.cpp` owns the
numerical `Problem` and the original ordered grid strings. `decode` checks the
4000/default header, strict CRLF framing, quoted strings and dimensions, then
parses numerical cells without replacing their spelling. `readTransportation`
remains a convenience wrapper returning its numerical problem.

The serialized grid has `(m+2)*(n+2)` cells. Its destination aliases are on row
zero at columns 1..n. Source rows 1..m have an alias at column zero, costs at
1..n and supply at n+1. Demand is on row m+1 at columns 1..n. Reserved cells
and original count-record spellings survive unchanged saves.

Native coordinates include one additional label row and column, so UI cell
`(2,2)` maps to document `(1,1)`, the first cost. Headers/reserved cells are
immutable. A delegate validates a candidate before committing it, updates only
that item, and advances to the next editable cell on Return/Tab. Full grid
replacement uses `inputReset`; `cellChanged` preserves the next live editor.
A reentrancy guard prevents an error dialog's focus events from committing
again while validation is already running.

`TransportationController` in `src/desktop/transportation_controller.cpp`
owns the optional document, dirty/read-only flags, filenames, selected method
and trace. Successful edits invalidate stale solver output. Invalid edits,
failed opens and failed saves preserve the existing model and file state.
Commit requests flush the live grid editor and title before save/navigation.
New grids initialize through Return on a count field or Initialize Grid.
Reinitializing counts rebuilds defaults; it does not resize while preserving
old cells. Example loading uses the same document representation.

Save and Save As both use a picker with a `.txt` default suffix and overwrite
confirmation. `QSaveFile` writes a temporary file and commits it atomically;
no direct-write fallback is enabled. Dirty/path/read-only state changes only
after successful commit. A read-only source can be saved to another filename,
which becomes writable; saving back to the protected source is rejected.
Paths are Unicode-capable Qt strings even though file contents use the bounded
ASCII profile.

The native dirty guard asks before abandoning edited input or leaving the
module. Cancel, canceled pickers and failed saves keep the route/model.
No permits navigation, retaining the in-memory document until replacement.
Clean documents do not prompt. This dirty-state policy is a native safeguard,
not a claim that every original TORA navigation path prompts identically.

## Model editing versus solving

A document may be unfinished or unbalanced. Blank numeric entries expose zero
without losing their blank spelling. Finite negative costs are allowed;
negative supplies/demands are rejected. The solver retains its existing exact
single-precision-representable quantity and balance requirements. Editing/save
does not expand the verified numerical profile or add dummy sources/routes.

The file profile stays `4000`, N=5, D=2, decimal header 1. Scientific settings,
other code pages, all malformed-file recovery and original dimension extrema
are unverified. Native limits remain 1..100 dimensions, 4 MiB files and 250,000
displayed trace cells. Doubled quotes round-trip within the clone; TORA's
Transportation behavior for those titles/aliases remains unverified.

This cycle does not implement EditGrid insertion/deletion/copy/paste, manual
loops, alternate multiplier initialization, final summary or Transportation
printing. Those require separate observed contracts and feature cycles.

## Verification and practical limits

Tests were added before implementation. The core target first failed on the
missing document header; native tests first failed on missing controller APIs.
Logs are `build/transportation-input/tests-before-implementation.log` and
`native-tests-before-implementation.log`.

The regression suite covers exact original file records, reserved cells,
validation transactions, blank drafts, aliases/quotes, dimensions, read-only
copies, Unicode filenames, save/open failure preservation, navigation
cancellation, trace invalidation, active-editor saving and successive keyboard
commits. Existing method goldens additionally compare writer bytes to their
independently reconstructed and authentic-hash-checked inputs.

Input renders cover 1366x768, 1920x1080 and 3840x2160 at four scales. Small
high-DPI views scroll vertically to expose lower controls. Native Wayland
acceptance also saves/reopens an edited input while preserving the LP model.
Current test counts/results and delivery limits belong to `STATUS.json`;
previous Vogel CI and installed binaries do not validate new uncommitted edits.

Primary implementation references:
[Qt QSaveFile](https://doc.qt.io/qt-6/qsavefile.html) and
[Qt item delegate commit/edit lifecycle](https://doc.qt.io/qt-6/qabstractitemdelegate.html).

Local final product verification: 88/88 desktop CTests in 173.88 seconds; 32/32 Qt-free CTests in 4.12 seconds; three authentic input hashes; 52 existing method-case writer round trips; eight input Qt checks at each of four scales; twelve input resolution renders; seventeen actual Wayland views with LP model preserved. The saved TORA copy of the clone-written cost model was reopened successfully (REA ev_2afa299374889cf0c37ece3ae0869e55b67a6b025127528f7c19aad7eadc7338). Cross-OS CI/package/install are pending for this feature.
