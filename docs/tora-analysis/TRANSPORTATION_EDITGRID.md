# Transportation EditGrid

Status: implemented locally on future-scope; current verification and delivery
identity are tracked in STATUS.json. Input editing/new/save was already
delivered at edc34b0. This separate cycle adds structural editing.

## Observed commands and selection

The original input-page EditGrid menu contains Undo Last Insert, Paste, Or
Delete (Ctrl+U), Copy Column or Row (Ctrl+C), Paste on Column or Row (Ctrl+P),
Delete Column or Row (Ctrl+D), and Insert column or Row (Ctrl+I). Click a source
or destination heading first. Single click followed by Insert inserts after
that line; double click followed by Insert inserts before it. Insert executes
immediately, rather than entering a later insertion mode.

Inserted costs are 0.00; the new alias and quantity are blank. Existing rows,
columns, quantities and aliases move with their line. Copy/paste includes
the alias, costs and supply/demand. Source and destination types cannot cross:
the original displays “Mismatch: Copy of Row(column) cannot be pasted on
column(row)”. The native menu disables incompatible paste proactively; direct
controller invocation also rejects it transactionally.

Original deletion succeeds from three lines to two but silently preserves
the model at two. The native controller keeps at least two lines for Delete
and displays an explicit error. The document API still represents 1..100-line
models; those are native data/safety bounds rather than proven original extrema.
There is one undo entry for the last Insert/Paste/Delete. The original menu
does not expose a general edit history. Save can retain structural Undo.

## Evidence and original defects

All evidence is ordinary UI/screenshots/saved bytes captured through REA.
Raw original material stays private in ignored reference/. Public fixtures
contain independent grid transcriptions, hashes and evidence identifiers.

| Observation | REA evidence | Result |
| --- | --- | --- |
| Repeated destination deletion/save | ev_73f6eccc6c0471d8104bcac652d626a9b9160164ce4cd7ef28a0047e6bc92a1e | Visible2×2 and16grid cells, but saved header1×2 |
| Source insertion | ev_755d7563999638b7b938da64d57fed0c00bea64926451f63c747ee415752e2b2 | Blank source afterS1; original rows preserved |
| Destination insertion beforeD2 | ev_0e8c3a7f2c44250cc3ed72ffb3b4ac495e95bf8a23d34de3a8247611f570878c | Started from three-source model; blank column beforeD2 |
| Fresh source insertion/undo | ev_cf936d7ccf309caf4850c1c3783117b3ab9f62d77a55498dbf19ef3cba3e682d | Original undo loses unrelatedD2, leaving2×2 |
| Cross-axis menu paste | ev_6a57ae880dad47c2ad9bc39dae723610adafabb133ac49cdee5d0c88df1f91f0 | Mismatch warning, unchanged grid |
| Boundary contrast | ev_fd26df1392c34edccfc472522725500974a64c976bea1ac352879f1b073ba961 | Insert2→3 then delete3→2; destination delete3→2 |
| Stable two-axis delete floor | ev_7126320b53b46409d2a8c9c8a87109498255bffb2563b0a9fc3783712cf1fed7 | Both axes unchanged at two |
| Destination copy/paste | ev_7b3a2814f7f43186f7e6213b832d5424daa28d1c7c030d6bb3e06ef1383165e2 | D1 costs3/4 and demand25 copied toD2; supplies preserved |
| Stable source copy/paste | ev_53427e21ffc00390c70dace20fca333d819606c6adc4dabd020ca57f85ac10a9 | S1 costs2/3/1 and supply20 copied toS2; demands preserved |

Each listed completed capture has child exit0. Earlier failed save/helper
attempts remain private evidence and are excluded from successful-save claims.
Some screenshot names describe an attempted later command: the destination
paste fixture uses delete-at-two.png because that image shows the committed
paste. Stable subsequent screenshots confirm its grid.

Two original defects are intentionally corrected. Structural saves sometimes
write a source-count record one below the actual visible grid. The native
writer always uses consistent actual dimensions; the strict reader continues
to reject malformed files. Source-insertion Undo can remove an unrelated
destination in the original. Native Undo restores the entire pre-operation
document, including every grid record and count spelling. These corrections
mean structural output is not claimed byte-identical to the corrupt original.

## Design and execution flow

TransportationDocument owns numeric data and lexical grid strings. line()
extracts a full source row or destination column. insert()/erase() shift
complete records, including reserved edges; paste() replaces the selected
line. Every operation builds and decodes a full candidate before replacing
the document. Rejected positions, sizes, text or numeric values preserve it.

TransportationController commits any active cell editor and title first.
It applies the selected operation to a copied document; success stores a
one-level undo snapshot, swaps the document, marks it dirty and clears stale
solver results. Failed operations preserve document, undo and solver trace.
Undo restores the snapshot and also clears stale results. New/open/reinitialize
reset selection, clipboard and undo. Save clears the selection but retains
history. Ordinary later cell/title edits clear structural undo so it cannot
silently discard unrelated input; this is an explicit native history policy.

The native grid translates label headings to zero-based source/destination
selection and highlights the complete line. Structural shortcuts are scoped
to the input-page EditGrid menu. Read-only documents permit Copy and reject
mutations. The clipboard is typed and checked for current line length; reopening
a model clears it. Broader original cross-document clipboard persistence and
all formatting/settings combinations remain unverified.

## Verification

Core and native tests preceded implementation and failed on the missing APIs.
The independent structural probe gate also first failed on missing options.
Private logs are in build/transportation-editgrid/.

The five-scenario fixture tests full records for source insertion, destination
insertion/deletion and both paste axes. Original corrupt dimensions are
provenance only: expected native dimensions are separately required to be
consistent. Additional asymmetric core tests cover all insertion/erase
positions, exact imported lexemes, aliases/reserved cells, numeric validation,
transactional failures and native1/100bounds.

Qt tests cover heading events, menu availability, insertion placement, dirty
state, read-only protection, trace invalidation, one-level undo, save/history,
minimum deletion, and invalid active editor preservation. Twelve structural
renders cover1366×768,1920×1080,3840×2160 at100/125/150/200%.
Actual-desktop acceptance adds insertion, paste, undo and structural save/reopen
while preserving the existing LP model. Final results belong to STATUS.json.

Remaining Transportation cycles: multiplier choices/manual loops, final
summary, printing and observed settings. Six other OR modules remain on the
whole-app plan. Physical mixed-DPI monitor handoff and printer hardware
acceptance remain outside this automated gate.

Local gate:89/89desktop CTests in158.43s;33/33Qt-free in3.71s. Final five-scenario fixture addition passed focused verification in both builds. Actual Wayland acceptance passed20views with LP preserved; structural save/reopen and undo passed. Native actual views and1366×768 at200% structural render inspected. Three-OS/package/install verification is still pending. Exact source-insertion saved-file evidence is ev_ee28efaa777db1a23c0638ed12aba5cf32f008c40e1d97bb29c4346836fc8f2e (child0); earlier table entry establishes the preceding insertion action.


EditGrid final delivery: sourcea4dd5314d99200d8ef773b816c07a0488642aafa passed [three-OS CI](https://github.com/Nitir4/TORA-Clone/actions/runs/38292868212) with89CTesteach/267total,34326comparisonrecords/OS,11inputchecks/scale and12structuralrenders/OS. All36renders and small200%eachOS reviewed. Sixartifactdigests/archives, package/legal/installer/PDF checks,281Linuxpayloadhashes and actualWayland/xcbstartup passed. Exact AppImage installed/verified in nativeWayland; SHA256 6944bbed0de16be6854afbf420dfad9495a4f57f7ff4e7fce28e9494c09d2722. Previous releases/windows preserved. Evidence build/transportation-editgrid/, packages dist/transportation/editgrid-verified/. Native consistent headers and full snapshot undo intentionally fix documented original defects.
