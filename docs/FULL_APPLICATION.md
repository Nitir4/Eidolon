# Full application implementation

The current scope includes every original MAIN family. The earlier LP edition
and four-module completion boundaries are historical. This work remains native
C++/Qt Widgets; numerical backends also build without Qt.

| Family | Implemented calculation | Native input/output and files | Verification status |
| --- | --- | --- | --- |
| Linear Equations | Rank-aware solve, nullspace, inconsistency witness, inverse, determinant, row operations | Original Equations2000 and Matrix1000; row-operation traces, inverse/determinant and printing | Generated models/trace replay pass; nonzero original solve/inverse outputs now verified; associated inverse/determinant added |
| Linear Programming | Existing graphical/simplex/M/two-phase/bounded/dual/final/sensitivity; Cj/RHS and revised-dual extensions | Existing3000 input/files/traces/graphs/printing | Captured-profile checks pass in the160/160 desktop checkpoint |
| Transportation | North-west, least-cost, Vogel, MODI, manual loops and multiplier anchors | Existing4000 editing/files/summary/printing | Captured-profile checks pass in the160/160 desktop checkpoint |
| Integer Programming | Mixed integer branch and bound with independently checked two-phase LP relaxations | Original5000 files, integer flags, automated/guided tree/branch controls, save/print | 300 pure +300 mixed independent cases; original6/7 and guided bounds; native focused checks pass |
| Networks | Minimum spanning forest, shortest routes, maximum flow/minimum cut; Dijkstra/Floyd actual iteration traces | Original7001/0/7004 captures; node editing, graph, traces, files/print | Independent graph oracles pass; Original MST3/shortest5/flow5 and Dijkstra/Floyd observations verified |
| Project Planning | AOA CPM/PERT, event times, floats, critical path, path variance and approximate deadline probability | Original8001/8002 files, editing, graph, traces, CPM bar chart/shift/zoom/Print Graph | 300 DAG +300 schedule cases; original exchange/CPM10 verified; native focused checks pass |
| Queuing | Stationary finite/infinite M/M/c and finite source; probabilities and comparative scenarios | Original6000 finite records, native state-range output, files/print | 300 generated balance/Little checks; pure birth/death unit-time profiles implemented; effective 500 legacy cap observed; small residual arithmetic discrepancies remain |
| Zero-Sum Games | LP-based pure/mixed equilibria and two-row graphical envelope | Original9000 files, strategy editing, graph/zoom/print | Backend, full desktop checkpoint and final native checks pass |
| Assignment extension | Rectangular min/max Hungarian with dual certificate | Clone JSON and explicit unit4000 import, results/files/print | 600 enumerations pass; native keyboard/PDF checks pass |

## Evidence and limitations

Original entry points and retained runtime Evidence are indexed in
[tora-analysis/FULL_APPLICATION_INVENTORY.md](tora-analysis/FULL_APPLICATION_INVENTORY.md).
Individual module contracts distinguish observations, mathematical extensions,
unsupported numerical profiles and tests actually executed. Raw original
binaries, saved inputs, process evidence and screenshots remain private.

A mathematical result is not established by a matching screen alone. Backend
checks use independent enumeration/formulas or optimality certificates. Native
checks exercise visible edits, error recovery, file roundtrips/read-only guards,
result invalidation, navigation and PDF output at four display scales. The
all-module acceptance check additionally verifies the actual MAIN entry labels
and populated workflows.

Observed original pure-birth/pure-death unit-time PMFs are implemented separately
from positive-rate stationary queues. The original N=3 death partial mean is labeled
separately from the correct expectation. New REA boundary captures explain the
original N=9999 display through an effective 500 calculation cap; a separate
legacy comparison preserves the requested model mathematics. Small original
arithmetic differences remain. See [continuation evidence](tora-analysis/REA_COMPLETION_20261018.md). Physical printers, hardware DPI handoff and the newly expanded
three-OS packaged release remain unverified. The installed Linux executable is
still the previous release. Local source additions are not an installed upgrade.


## Preserved earlier local checkpoint

The earlier full desktop gate passed 160/160 in 125.04s before the final chart,
printing and layout additions. Those changes passed 22/22 focused checks in
6.98s, followed by compact chart 8/8 in 4.71s. That checkpoint had headless 58/58
in 5.44s and five actual Wayland QtTest slots per scale, including setup/cleanup.
Its 80 PNGs and 24 PDFs / 34 pages / 3452 words are retained under
`build/full-application/verification.json`. Fresh Release and packages were not
part of that earlier checkpoint; the continuation below includes them.

## Verified REA continuation — 18 October 2026

Final full local Release passes 161/161 in 112.14s; latest headless passes 58/58
in 7.45s. Actual Wayland at 100% and 200% passes Equations 10, Integer 10 and
Queue 12 QtTest slots per run, including setup/cleanup. The six final native PDFs
have eight pages / 910 words with no blank pages or out-of-page word boxes.

Integer pending children, associated equation inverse/determinant and a separate
Queue 500-cap comparison are implemented. Compact Queue controls fit 200% tiles.
New original nonzero Equations/inverse results and Queue boundary evidence are
in [the REA findings](tora-analysis/REA_COMPLETION_20261018.md).

The reviewed report compiles to 68 pages and preserves 38 verbatim evidence blocks.
A pinned offline Linux preview starts MAIN plus nine modules, passes actual
Wayland/X11 startup, and matches all 247 extracted manifest hashes. Its host
libraries require glibc 2.43; older Linux and three-OS release portability remain
unverified. The installed app is unchanged. Exact receipts are in
`build/continuation-rea/verification.json`.

Actions remain disabled. Windows/macOS, physical hardware and broader original
profiles are outstanding acceptance gates.


## Platform and print followup (18 October 2026)

The latest followup is recorded in
[tora-analysis/PLATFORM_FOLLOWUP_20261018.md](tora-analysis/PLATFORM_FOLLOWUP_20261018.md).
Queue service-time underflow, Integer pending printing and selected-SDK Linux
deployment are corrected. The host Release checkpoint passes 161/161; final
portable PDF reader and font-independent structured verification are being checked.
Original software-printer captures now succeed, superseding the prior environment's
error482 limitation. Changed PERT input still faults at its terminal event.

An isolated Ubuntu24.04/glibc2.39 build and offline package preflight succeeded;
the final full gate/package is pending test-source freeze. User approved one
three-OS workflow run with temporary Actions enablement and disabling afterward;
no run has been dispatched yet. No hardware devices are available, so physical
printing and mixed-DPI handoff remain unverified. Historical checkpoint numbers
above should not be mistaken for final results of this followup.


Current corrected-source checkpoint: host163/163, Ubuntu24.04/glibc2.39 163/163
and headless58/58 pass. The first native run passed Ubuntu fully; Windows/macOS
failures exposed a missing test header, platform edit-key assumptions and a real
flow-certificate range bug. All are corrected locally. The six-file native
followup is prepared and awaits the user's answer; Actions are disabled. Use
`docs/STATUS.json` and the platform-followup ledger for current receipts rather
than historical checkpoint limitations above.
