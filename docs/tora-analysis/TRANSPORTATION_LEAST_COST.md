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
