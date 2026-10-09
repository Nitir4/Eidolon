# Unverified observations and module-gate obligations

The original 37-pair repair and later 17-pair repair are finished and empty.
Do not rerun them. Evidence coverage passes 79 full traces plus two expected
warnings, 273 tables, 7,711 numeric cells; 15 graphical and 15 final-summary
studies; 19 byte-identical TORA Save As/reopen round trips. Additional probes
cover finite bounds, manual pivots/alternative optima, notation, M acceptance,
row editing, zoom and specific malformed files.

These unknowns are part of the Gate 0 review package. They are not permission
requests and must not be silently promoted to verified compatibility. Gate 0
approval accepts the analysis/design and its explicit limits; it does not
waive the later requirement to resolve any ambiguity affecting a module's
implementation or comparisons.

| ID | Unverified behavior / observed limit | Must resolve before |
| --- | --- | --- |
| Q-01 | Hidden arithmetic precision, zero/near-tie thresholds, rounding at halfway values, negative-zero creation, cycling safeguard/iteration cap | Foundation 2 decisions and affected numerical module gates |
| Q-02 | N/D allowed extrema, invalid notation-field messages, large-width/exponent overflow, scientific formatting beyond N=5/D=2 | Formatter/input/output module gates |
| Q-03 | Undo enablement/restoration; column insert/delete/copy/paste; all advertised shortcuts, objective-cell editing, invalid relation/name/cell messages | Data-entry module gate |
| Q-04 | Quoted-alias input commitment, other text code pages and arbitrary malformed records; basic read-only flow, suffix, Save target, comma/quoted/non-ASCII titles resolved | Phase 2 input controller and any newly supported encoding/profile |
| Q-05 | Resolved for captured inputs: one/two/three-variable eligibility, explicit bound/free rejection, manual states and sampled range/tick/dot/zoom profile. Hidden rules outside the captured profile remain unverified. | Graphical module evidence / future profile expansion |
| Q-06 | Unrestricted expansion, reverse bound complements, redundant equality/artificial-basic cleanup | Preprocessing and affected solver gates |
| Q-07 | Resolved: direct Dual infeasibility title and message captured in du-infeasible | Dual-simplex module evidence |
| Q-08 | Successful table/graph printing, pagination and print dialog defaults | Printing/output module gate |
| Q-09 | Resolved for35 captured summaries:automatic counts,bounds,free orientation,negative RHS and signed prices. Hidden arithmetic/transformations outside this profile remain unverified. | Future profile expansion |
| Q-10 | Complete independently reviewed golden labels/messages and every transcribed table, beyond automated coverage and reviewed glyph seeds | Tests-first step of each numerical/UI module |

Q-08 has an environment blocker: Write to Printer produced Run-time error
'482': Printer error under Wine, and acknowledging the dialog ended TORA.
A working printer configuration or Windows reference is required to observe
successful printing. The requested fallback is to generate a precise test
packet for the user to run if that reference remains unavailable. No successful
print behavior has been guessed.

Specific file errors are now observed: wrong marker 9999 produces the exact
module-format Retry warning, 15-record truncation leaves a blank grid, an
extra quoted record is ignored, LF-only framing leaves an uninitialized form.
These results do not establish all malformed-file behavior. M 0/-1/0.5/1e2/
bogus each reached an initial tableau; bogus visibly behaves like zero there.
This does not establish arbitrary text parsing or all terminal M behavior.

Revised simplex and Parametric analysis are absent from the complete visible
LP hierarchy; neither is included as an invented module. An additional path
would be studied if direct evidence reveals it. The M=100 infeasible case's
optimal notice with positive Rx5 is recorded legacy behavior, not an unresolved
transcription mismatch. The clone's implementation must preserve observed
compatibility explicitly rather than silently correcting that trace.

Foundation 2 narrows Q-01/Q-02: 80 reviewed display strings verify halfway
rounding, tiny-negative display, exponent padding, D=0/D=10 and scientific
N=0/1/2/5/10. Decimal N=0 triggers runtime error 5; 1e-100/1e100 triggers
runtime error 6. Exact policies, sources and remaining limits are recorded in
`tora-analysis/NUMERIC_FOUNDATION.md`. The Qt-free pivot primitive matches the
normal initial/next two tables. Solver thresholds/tie policies and iteration
caps are still required at solver gates; the primitive never selects a pivot.

Foundation 3 resolves the basic inconsistent-bound validation part of Q-06:
TORA rejects lower > upper, including unrestricted y, with the captured
two-line Input Data Error message. Equal bounds and default-bound y reach
method format selection. Zero counts produce the captured Problem Size
diagnostic. Signed finite input values are preserved. Near-equal bound
thresholds, arbitrary numeric text parsing, unrestricted transformation and
negative-bound algorithm behavior are not inferred from these input checks;
their ownership and capture hashes are in `tora-analysis/LP_MODEL_VALIDATION.md`.

Foundation 4 resolves read-only behavior and the quoted-title storage failure.
Read-only Save opens Save As without changing the original; edits and Save As
copies are allowed. TORA writes doubled quotes but corrupts its own reopened
title/dimensions in the captured combined text case. The user explicitly chose
recovery; that approved exception is tested against the actual saved bytes.
The bounded parser uses explicit engineering diagnostics for unverified corrupt
cases and does not create partial/blank legacy models. Native-picker policies
belong to the later controller. See `tora-analysis/LP_FILE_IO.md` and the F4 gate
report for current live bridge/encoding limits; no unreviewed automation frame
is promoted to compatibility evidence.

Phase 2 input narrows Q-02/Q-03/Q-04 further. N/D=11 and 99 initialize/save;
N=999 raises runtime error 6 and exits after acknowledgment. The native
engineering profile is bounded at 99; successful extrema 100..998 and
arbitrary malformed-field messages remain unverified for future expansion.
Real keyboard captures resolve numeric filtering, objective toggle, alias
Return, relation/sign normalization, column/row Copy/Paste/Insert/Delete,
count Return/Tab reinitialization and gray Undo states. No successful Undo
state or pending insertion mode has been observed or invented.

Quoted-alias commitment is resolved: TORA writes doubled quotes, then corrupts
its own grid on reopen. The previously approved quoted-field data-recovery
policy retains the model/alias. Windows-1252 native conversion and Unicode
filenames are tested; additional code pages are not claimed. See the
Phase 2 input specification/report for exact capture paths and limits.

Phase 2 all-slack resolves the captured unrestricted positive/negative directions,
nonbasic direction reversal and reverse bound complement portions of Q-06.
It keeps one x1+/x1- column. The near entering tie resolves 3 versus
3.0000000001 (x2 is selected despite equal displayed coefficients); the exact
row tie chooses the topmost row. These observations do not establish a universal
hidden tolerance or cycling policy (Q-01).

The apparent crossing -0.00 mismatch is resolved by scientific -5.55112e-17;
D=12 retains the negative rounded zero. The native basis reconstruction matches
both observations. This is not proof of TORA's internal arithmetic. No asserted
all-slack golden mismatch remains. Upper-bound-versus-row exact ties, additional
ill-conditioned/free-zero-cost cases and hardware DPI moves remain unverified.
Successful native PDF generation does not resolve Q-08's unavailable TORA print
output. Seven artificial-method handoffs are classified and their setup routes
are tested; actual M/Two-phase algorithms remain separate cycles with Go disabled
for their pending sessions.

Phase 2 M-method narrows Q-01/Q-06: fifteen direct sessions/eight fallback
sequences and four scientific cells now match. Complete 0/-1/0.5/1e2/bogus
outcomes and upper/lower/free traces replace their earlier initial-only limits
for these exact inputs. Explicit fallback Cancel also advances to output format.
Cross-product basis inversion with a dual-first cost row reproduces the observed
1.11e-16/5.55e-17 residuals without cell-specific patches; this is not proof of
TORA internals. Arbitrary M parsing, ill-conditioned/unseen cases, cycling, exact
bound ties and physical DPI/legacy-print acceptance remain unknown. M Go is
now functional; Two-phase is the next separately gated numerical cycle.

Two-phase narrows Q-01/Q-06 further: the captured redundant equality and zero
RHS runs retain zero-valued artificial basic rows when entering Phase 2. The
full scientific run retains tiny constraint residuals but exact zero basic
objective costs. Manual feasible-phase acknowledgement leaves the final Phase 1
table; the following Next appends Phase 2, whereas All suppresses the feasible
boundary message. Cancel on the all-<= warning also uses All-slack. Hidden
near-zero feasibility thresholds, ill-conditioned inputs beyond these captures
and redundant-basis behavior outside the reviewed cases remain unverified.


The Bounded cycle resolves an exact entering-bound/constraint-row tie (row wins),
complement return Ux1 to x1, zero-width complement, direct warning-free setup,
and lower-shift artificial dispatch with an initial infeasibility notice.
Its 24-session/85-table corpus retains a full unrestricted scientific trace.
Near-tied steps, unseen bound/sign combinations, hidden thresholds/cycling,
original successful print layout and physical mixed-DPI moves remain unverified.
The excluded attempted Cancel probe supplies no M-routing claim.

Bounded scientific output distinguishes its canonical basic objective costs
(exact zero) from the earlier All-slack residual display (1.67e-16). Constraint
residuals remain unchanged. The implementation retains separate profiles;
decimal trace agreement did not establish identical floating-point output.


## Dual cycle update

Focused direct captures now resolve exact RHS/ratio ties, scientific basic-cost canonicalization and direct no-entering infeasibility. Bounds or unrestricted variables cause a specific Two-phase/M warning rather than direct bounded Dual. Initially feasible objective-compatible starts remain Dual and stop at Iteration 1. Manual row/column, Cancel-to-M and alternative continuation probes are now accepted. Final verification passed: 50/50 desktop tests per OS, 20/20 local C++17 Qt-free Release checks and all package/startup checks. The scientific alternative repeat also resolved three small residuals and the no-repeat-notice behavior. Near-tie thresholds, hidden cycling/precision, outside-corpus arithmetic, physical mixed-DPI moves and original printer layout remain unverified.

## Graphical cycle update

The accepted corpus is 35 graphical result/geometry cases, with manual/eligibility observations and 755 independently identified source pixels. It resolves small/fractional/scientific ranges, sampled large lattice/tick changes, ties-to-even zoom and non-returning small zoom pairs. Source inspection confirms that infeasible/unbounded states keep an origin marker without a finite objective contour. Occluded axes are recorded rather than inferred from nearby text. Local full native 56/56 and C++17 Qt-free Release 22/22 checks pass.

The original internal graphical solver/range thresholds outside sampled inputs remain unknown. Native viewport/lattice safety policies are explicitly engineering limits. Graph printing also raises error 482 in Wine; native PDF success does not establish original printer parity. Physical mixed-DPI handoff and successful original printing remain open. Final/sensitivity are not implemented by this cycle.

Final graphical CI run 37908644559 also passes 56/56 tests on each OS and all package/deployment checks. The initial Windows synthetic-font failure was resolved by reusing the established installed-font bootstrap; no golden/drawing tolerance was weakened. All original-print/physical-DPI/outside-profile limits above remain explicit.


## Final solution and sensitivity closeout

The separate final checkpoint passes62/62 per OS at41c5017. Its sensitivity
extension atf18eaf9 passes68/68 per OS,204 overall, with all deployed-package
startup checks successful. Exact expanded comparisons include1116 final fields
and2337 sensitivity fields. All35 actual saved-file round trips and source hashes
pass;1124 fixture numeric strings match independent private glyph transcriptions.
The observed automatic iteration accounting, interleaved bound rows, signed
minimization costs, negative-RHS/free orientation and negative-lower/upper quirk
are resolved. See LP_SENSITIVITY_PARAMETRIC.md for compatibility exceptions.
There is no known mismatch in the accepted corpus. Original printer output,
physical mixed-DPI handoff and behavior outside captured profiles remain limits.
