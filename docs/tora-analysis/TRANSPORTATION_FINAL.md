# Transportation final summary

Status: implemented locally on future-scope. The current gate is recorded in
STATUS.json; the installed release remains the last verified source until this
cycle completes its own packaging and installation checks.

The native route is Transportation → SOLVE Menu → Final solution. It shows
TRANSPORTATION MODEL OUTPUT SUMMARY, the input title, Final Iteration No.,
Objective Value (minimum cost), and a five-column table: From, To, Amt Shipped,
Obj Coeff, Obj Contrib. Next/All are disabled on the final page.

## Independent observed contract

Seventeen retained original summary screens were independently reviewed,
including 355 populated table cells and the following blank rows. See
TRANSPORTATION_FINAL_REVIEW.md for screenshot/hash/REA provenance and the
corrections to earlier pilot-only assumptions.

The final route agrees with the independently captured Least-cost terminal
basis and iteration count in all seventeen cases. Cases07,10,17 report
iteration2; the others report1. Case14 distinguishes this basis from Vogel's
different optimal basis. This supports using Least-cost for the captured
default final route; it does not identify original internal code or prove
every possible input.

Rows follow source-major, then destination-major order and include basic zero
shipments. The pilot reports iteration1/objective150.00:

| From | To | Amt Shipped | Obj Coeff | Obj Contrib |
| --- | --- | --- | --- | --- |
| S1: | D1: | 5 | 2.00 | 10.00 |
| S1: | D3: | 15 | 1.00 | 15.00 |
| S2: | D1: | 5 | 5.00 | 25.00 |
| S2: | D2: | 25 | 4.00 | 100.00 |

The earlier statement that all final screens report iteration1 was too broad.
The pilot's positive amounts also did not justify omitting basic zeros.
Those assumptions are superseded by the complete review and current fixtures.

## Fractional display and alias probes

Case08 shows final amounts0,2,0,3 for actual shipments0.75,1.75,0.75,2.75.
Contributions remain1.69,3.06,4.13,11.69 and objective20.56. Case16 displays
all four amounts as0 with nonzero contributions and objective0.42.

A fresh two-source/nine-destination discriminator isolated the amount rule.
The original displays0 for0.125,0.5,0.75,0.984375;1.125 becomes1; and positive
half ties1.5,2.5,3.5,4.5 become2,3,4,5. A basic zero row is retained as well.
This establishes the bounded policy used by the native presenter: values below
one display0, otherwise round to a whole number with half ties upward.
Contributions always use the actual solved amount, not this displayed integer.

The authentic save is private
reference/tora-captures/transportation/final-probes/actions-fractional-amount-threshold/tora-saved.txt,
SHA256032fabd28c79adab64a34a20790e333bb701e833e0aeed89e2557409f1e16a9e.
REAev_752236634140fc085daaa605bd67cbd497467b8c27aaea70e969987a6262d56e
has child exit0. Its initial.png is the actual final summary. The existing
batch helper returns to input afterward; final-stable.png is that returned
input view and is not used as summary evidence.

An alias probe establishes numbered prefixes with colon plus the user's
source/destination name. Alpha/Beta and Depot/Hub/Port appear with the pilot's
unchanged four rows/objective. Exact separator-space count is not established
by the screenshot. The native view uses one ordinary space before a nonempty
alias, an explicit presentation policy rather than an exact legacy whitespace
claim. Authentic save SHA256:
c81b7572ce1da8394819cdebb17d324a7a1350d22f70300682168396cd610063.
REAev_06b55d2bf25b94a98815e46470fd585c2971e32239b2b4c1693cd1d38c3738eb,
child exit0. Raw images/files remain private; public fixtures contain
independent transcriptions, hashes and provenance.

## Native design and execution flow

The Qt-free FinalSummary API solves the documented default Least-cost route
and formats its terminal basic cells. It returns title, final iteration,
objective and ordered rows. It retains basic zero cells and source/destination
aliases. Costs, objective and true shipment×cost contributions use the existing
two-decimal NumberFormatter. The distinct amount formatter preserves the
observed subunit discrepancy. The input problem is not modified.

TransportationController::computeFinal flushes active input first and builds
a candidate summary before replacing its output state. Failure preserves the
previous summary, model, iteration method and trace. A separate finalChanged
signal refreshes the final page without resetting iteration history. Successful
input edits, structural changes, new/open/reinitialize invalidate stale summary
output. A save with unchanged model does not invalidate it.

The native Final solution action first enters the existing guarded solve-menu
when invoked from input. Save? Cancel therefore blocks computation and
navigation. Only a successfully computed summary opens the final page.
The page uses native layouts, font metrics, scrolling and readable colors.
A direct empty preview gives input guidance without fabricated numbers.

The prior iteration trace remains available after viewing the summary, including
its selected multiplier anchor. Final output does not silently replace whichever
method the learner last viewed. Existing source bounds, balanced input and
exact single-precision-representable quantity profile continue to apply.

## Verification and remaining work

Tests were added before production implementation. Core compilation first failed
on the missing final_summary header, native compilation on missing controller
APIs, and golden verification on the missing --final probe option. Logs are in
build/transportation-final/.

Nineteen independent original cases compare482summary fields and reconstruct
authentic saved-file hashes before reading. Eight alias cells use the explicit
native spacing policy; the remaining observed strings are exact comparisons.
The corpus covers iteration counts, alternative optimal basis, basic zeros,
negative/fractional costs, amount thresholds, aliases and actual contributions.

Core safety checks cover input immutability, zero rows, aliases, dyadic amount
thresholds and invalid/unbalanced/nonfinite arithmetic. A test originally
assumed a single huge unused route would force objective overflow; it was
corrected to force huge costs on every route. Expected original data was not
changed to accommodate that test mistake.

Native checks compare all nineteen tables, title/iteration/objective/header
strings, disabled iteration controls, controller preservation/invalidation,
direct-menu cancellation and twelve renders across three sizes/four scales.
Actual Wayland acceptance adds the summary and return to the prior anchored
iteration trace while preserving the LP model. Current complete results and
delivery identity belong to STATUS.json.

The original offers Write to Printer on its final screen. Transportation
printing/pagination is the following observed cycle and is not claimed
implemented by this summary gate. Scientific/N/D settings, broader arithmetic
profiles, legacy encodings, unbalanced models and original maximum dimensions
remain separate investigations. Physical mixed-DPI monitor handoff and printer
hardware remain unverified.


Local gate:98/98desktopCTests in183.23seconds;38/38Qt-free in4.17seconds;24finalQtchecks at each of four scales; nineteen authentic-file-backed summaries/482comparedfields. ActualWaylandacceptance passed26views with LPpreserved. Nativefinalsummary and small200%render visually reviewed. Three-OS/package/install verification remains pending.
