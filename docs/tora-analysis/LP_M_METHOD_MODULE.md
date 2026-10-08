# Phase 2 M-method implementation contract

## Scope and evidence

This cycle enables M-method setup and its native accumulated iteration screen.
The completed fifteen original selections are reused. Seven run M directly;
eight run All-slack after a warning. Eight additional full sessions close the
finite-penalty/bound/free gaps: M=0, -1, 0.5, 1e2, bogus, upper bound, lower
bound and unrestricted x1. The fixture contains 15 direct M sessions and eight
All-slack fallbacks, 78 complete tables and 4095 exact cell strings. Four
scientific cells bring the compared string count to 4099. Original screenshots,
saved bytes, hashes and exact glyph transcriptions remain private in reference/.

Original evidence: methods/lp/corpus/<case>/m_method/. New evidence:
probes/phase2-m-method/<case>/. tests/fixtures/m-method.json retains the capture
path for each independent transcription and supplementary observation. Numeric
expectations are read from screenshot glyphs, not calculated by the solver.
Full-table assembly checks gaps/overlapping frames; direct visual reviews cover
initial allocations, notices, bound/free labels, inverse cells and residuals.
Clipped scientific cells are not promoted to full scientific-table goldens.

## Selection, input and navigation (LP-MM-01)

Solve Problem → Algebraic → Iterations → M-method opens the M-Method group,
with Enter value of M:, selected default 100 and Go To Output Format Screen.
Return on the field did not advance the observed legacy route. Go opens the
shared Decimal/Scientific N/D panel; Go To Output Screen starts the session.
Input fields, dimension limits (100 variables/constraints), validation,
relations/bounds/sign flags and names retain LP_DATA_ENTRY_MODULE.md's contract.

On an all-normalized-<= model the warning is exactly:
`Problem has all <= constraints. All-Slack option will be used`.
Original OK captures establish the fallback. New m-slack-cancel-button captures
show explicit Cancel also reaches output format; Escape does too. The focused
m-slack-cancel-solve observation verifies the resulting method heading/table.
The native flow preserves this observed legacy behavior. Cancelling All-slack's
artificial-method warning opens the now-functional M-value screen. Its OK path
still hands off to the separately gated Two-phase method.

## Initial equations and finite penalty (LP-MM-02)

Normalize negative RHS and shift finite lower bounds as captured. Decision
columns remain in input order (Lx1 for a lower shift; x1+ for the captured free
case). Allocate all >= surplus columns first, labeled Sx(n+1), etc. Then allocate
one identity/basis column per original constraint in row order: sx for <=,
Rx for >= or =. Equalities have no surplus column. Never remove an Rx column
just because it leaves the basis; the M captures contain numerical coefficients
there, without Two-phase's blocked cells.

Start from -c. For maximization, subtract M times each artificial row; for
minimization add M times it. Eliminate each initial artificial objective cell.
The RHS includes the signed artificial penalty and lower-bound objective shift.
Captured default equality case has z row -203/-202/100/100/0/0/0 and RHS -600.
The minimization equality case has 199/199/-100/-100/0/0/0 and RHS 900.

M is finite binary64, not a symbolic or infinite penalty. The five acceptance
probes are now complete terminal traces: zero and bogus produce the same three
iterations/alternative notice; -1 reaches unbounded after three tables; 0.5
reports iteration 3 optimal with objective 11.50 and positive Rx7; 1e2 reproduces
the default four-table equality trace. Invalid text outside this captured bogus
example is not claimed to implement all legacy parsing quirks. Nonfinite
penalties/overflow receive safe native errors rather than corrupt snapshots.

## Iteration policy and arithmetic (LP-MM-03/04)

Use the captured most-improving eligible objective coefficient (negative for
max, positive for min), leftmost entering tie and topmost minimum nonnegative
ratio tie. Preserve row order. Finite-bound steps can complement a decision
column and replace a basic variable at its upper bound; m-upper-bound has five
tables and Ux1. The lower-bound case has four tables and Lx1. Free x1 retains a
single oriented x1+ column in this capture. Common selection errors and exact
terminal text are in LP_PIVOT_POLICY.md; invalid selections do not mutate data.

The native implementation reconstructs a basis from original equations, using
cross-product elimination for M and forming the dual row before reduced costs.
This is an implementation choice verified against the captures, not a claim
about TORA's hidden code. It matches the scientific iteration-3 Rx6/x1 residual
1.11e-16 and Rx6/x2 residual 5.55e-17, while x1/x2 is exactly displayed 0.00e+00
and the final Sx5/x1 is 0.00e+00. Ordinary incremental tableau elimination or
dividing the inverse's pivot row first did not match all these observations.
There is no per-cell correction, tolerance cleanup or display-driven arithmetic.
All-slack retains its independently verified reconstruction order. The LP
target disables contraction/fast-math (MSVC strict FP), matching the core
policy so compiler optimizations preserve the tested operation order.

The native 1024-pivot cap retains a resumable partial trace. It is engineering
protection, not an observed TORA cycling limit.

## Termination (LP-MM-05)

Preserve exact optimal, alternative (including soltuion(s)) and unbounded
messages and their Optimal Tableau dialog title. The original infeasible
fixture reports iteration 3 optimal despite Rx5=2.00 and penalized objective
-194.00. No extra feasibility cleanup or mathematical correction is inserted.
A nonbasic zero cost can be selected after an alternative notice to pivot to
another displayed optimum through the common teaching controls.

## Output, UI and files (LP-MM-06/07)

Heading: SIMPLEX TABLEAU - (M-Method). Shared columns: Basic, decisions, surplus,
initial basis columns, Solution. Rows: Iteration N, header, z (max/min), ordered
basis, Lower Bound, Upper Bound, Unrestr'd (y/n)?, separator. Default decimal
precision is two; formatting is delegated to the verified NumberFormatter.
Bound rows describe original decision bounds and leave other cells blank.
Constraint cells of the original identity block (sx/Rx) have white backgrounds
and blue values, including columns still basic. Ordinary values use blue/white;
entering columns turn green, leaving rows red. Objective cells stay blue/white.

Next Iteration, All Iterations, Write to Printer and original footer navigation
use the shared native Qt Widgets view/controller. Automatic work yields to Qt,
pauses on navigation and is discarded on input edits. Font metrics determine
sizes; large tables scroll and the blue background fills each native window.
Native PDF printing consumes the same immutable displays. Legacy successful
print layout remains unverified because Wine printing raises runtime error 482.
Open/save/new/export behavior remains the shared captured file contract; M is
session setup, not a new model file format. Raw reference assets are never
included in the product or release packages.

## Unknowns / things I could not verify

Hidden tolerances, cycling, ill-conditioned arithmetic beyond these cases,
upper-bound/row exact ties, arbitrary M text parsing/overflow behavior, successful
legacy printing, every keyboard shortcut and physical mixed-DPI monitor moves
remain explicit. The cross-product/dual order is compatibility evidence for
these inputs, not proof of a universal internal algorithm. Two-phase, Bounded,
Dual, graphical and final/sensitivity views remain separately gated modules.
