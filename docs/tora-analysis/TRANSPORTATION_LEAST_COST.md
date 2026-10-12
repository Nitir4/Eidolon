# Transportation Least-cost start

## Purpose and observed workflow

Choose Transportation model → Solve Problem → Iterations → Least-cost start
to construct a minimum-cost-cell starting basis, followed by automatic MODI
improvements. Existing input data, default decimal N=5/D=2 and u1=0 apply.
The observed heading is TRANSPORTATION TABLEAU - (Least Cost Method).
Input editing and file writing remain outside this method slice.

## Captures and tests first

Seventeen independently transcribed cases contain 20 complete tableaus under
private reference/tora-captures/transportation/corpus/<case>/leastcost/.
Every transcription's screenshot SHA256 is verified before creating
fixtures/transportation-leastcost.json; expectations are copied directly from
transcribed screen cells, never computed by the new algorithm. Authentic saved
file hashes are also compared before file-backed trace checks. The existing
North-west implementation fails this new golden corpus before implementation.
New native method-switch/rendering tests initially fail to compile because
the method API is absent; those logs are retained locally.

## Allocation and improvement rules

Among active source/destination cells, choose the least unit cost, breaking
exact cost ties by source then destination order. Allocate the smaller remaining
supply/demand. Remove one exhausted source if more than one source remains;
otherwise remove the exhausted destination. On equal exhaustion this keeps
a destination available for a basic zero. Explicit zeros and blank nonbasic
cells must remain distinct. Negative costs are allowed. This policy matches
all 17 observed initial bases; case 10 distinguishes choosing the largest tied
allocation, and case 17 preserves the degenerate S3D3 basic zero.

Use the shared captured MODI contract: u1=0, basic u+v=cost, reduced cost
u+v−cost, largest positive enters, minimum negative loop shipment leaves,
source/destination order for ties. No positive reduced cost ends the trace.
Case07, case 10 and case 17 each require one improvement. Shared native numerical
limits and exact-single-precision balanced-quantity restrictions apply.

## Native output and navigation

The same native table renders each method's costs, shipments, reduced costs,
u/v, supply/demand, objective and iteration label. Next/All append iterations;
terminal buttons disable, and advancement to the last tableau reports
“Iteration N is optimal”. Selecting an already optimal start shows the
“Optimal Tableau” dialog with “Iteration 1 is optimal”, as captured in case01.
The same shared selection behavior applies to North-west starts. Method selection replaces the trace only after a
successful solve. Failed method selection preserves the previous trace and
method. Loading another model clears the trace. The method heading refreshes
when revisiting cached views. Screen fonts/layout use the approved readable
palette and scale with native Qt layouts; no browser UI is used.

## Unknowns / things I could not verify

Other notation/code pages, nonbinary fractional display, unbalanced models,
manual loop choices, other multiplier initializations, exact numerical
thresholds and printer output remain outside the verified profile. Equal-cost
scan order is supported by the observed bases; this does not claim that every
possible equivalent internal scan order has been distinguished. Vogel is a
separate method cycle, not part of this implementation.


Final screen review found the initially optimal notice missing from the first
feature commit. A tests-first regression failed for both starts; selection now
shows the observed dialog after rendering the initial tableau. This correction
requires a new full-suite and three-OS gate; the earlier successful matrix alone
does not close it.


The next macOS matrix exposed a test assumption: Qt deliberately suppresses
QMessageBox titles on macOS, while Linux/Windows retain the observed title.
[Qt 6.10 documentation](https://doc.qt.io/qt-6.10/qmessagebox.html#setWindowTitle)
confirms this native platform behavior. The regression expects an empty macOS
title and the exact title elsewhere; every OS still requires the exact message,
Information icon, OK button, visible popup and disabled terminal controls.
No product or numerical changes were made for this test correction.


## Completed gate (12 October 2026)

Final source `77e78cb6adca3795b6f83e5cd18b12cebdc4934a` passes
[three-OS CI run 38249608884](https://github.com/Nitir4/TORA-Clone/actions/runs/38249608884):
81/81 CTest per OS, 49 Transportation Qt checks at each of four scales,
24 Transportation resolution renders per OS and 644 exact Least-cost fields.
All existing LP/North-west comparisons and 23 printer PDFs also pass.
Independent downloaded-report/package audits verify all three native artifacts,
Qt legal manifests, Windows' seven installer checks and Linux's 281-file payload.
The final AppImage starts on actual Wayland/xcb with SDK paths removed and is
installed under the stable `tora-clone` launcher. Its native Wayland window was
confirmed by process/AppImage identity; previous windows/releases were preserved.

Evidence is in build/transportation-leastcost-verified/{ci-verified,
packages-verified,linux-payload-verified,installation,installed-window}.json;
packages are in dist/transportation/leastcost-verified/. No known mismatch
remains in the accepted profile. Earlier successful-numerical/failed-screen
and macOS-title-test runs are retained as history, not final acceptance evidence.
The full local suite passed 81/81 after the product correction; final test-only
portability changes passed focused 7/7 and the complete three-OS matrix.
