# Objective and RHS changes: calculation backend

This class extension applies new objective coefficients Cj and/or constraint
right-hand sides B. It is separate from TORA's static sensitivity display.
The Qt-free API is `analyzeChanges` in `src/lp/include/tora/lp/postoptimal.hpp`.
It returns the changed model, original-variable mappings, before/after solutions
and canonical traces, plus the old basis under the changed data.
No new frontend is implemented yet.

Empty replacement vectors preserve the original values; otherwise each vector
must contain all coefficients or all constraint RHS values. Joint changes are
analyzed together. A finite optimal baseline is required. Modified models may
be optimal, alternatively optimal, infeasible or unbounded.

## Mathematics and transformations

For a standard equality system, the old basis gives
`xB(new)=xB(old)+B^-1 deltaB`. With changed costs, recompute `zj-cj`
from changed basic costs and the retained transformed columns. Nonnegative
basic values establish primal feasibility; eligible nonbasic reduced costs
must be nonnegative for maximization, nonpositive for minimization.
A retained basis is optimal only when both tests pass.

Signed RHS normalization is accounted for in the initial identity columns.
Artificial basic variables must remain zero; positive or negative artificial
values cannot establish a feasible solution to the original problem.

Finite lower bounds use x=L+y, finite upper bounds become extra rows, and free
variables split into positive/negative parts. Original variables and objective
constants are restored explicitly. This avoids the captured legacy automatic
final-summary negative-lower-bound asymmetry. All constraint relations and
signed RHS are supported. Canonical size must fit the existing 100-column/
100-row solver profile; an expanded model exceeding it is diagnosed.

The changed model is solved independently from its own starting basis using
the existing verified LP engine. This is a fresh solve, not a claim of warm-start
primal/dual or Revised Dual iterations. Each finite solution is checked against
original bounds/constraints, zero artificial basics and recomputed reduced costs.
A retained optimality certificate must agree with the independently solved value.
Numerical comparisons use the explicit engineering Tolerance; they do not infer
TORA's hidden tolerance policy. Overflow is diagnosed.

## Worked original comparisons

Baseline: maximize3x1+2x2 subject to x1+x2<=4,x1<=2,x2<=3,x>=0.
Original TORA gives(2,2),value10. Its sensitivity display confirms Cj1>=2
and RHS1in[2,5] for the current basis.

| Change | Old basis | Changed optimum | Objective |
|---|---|---|---|
| Cj1:3→2.5 | Primal and dual feasible | (2,2) | 9 |
| Cj1:3→1 | Primal feasible, dual infeasible | (1,3) | 7 |
| RHS1:4→4.5 | Primal and dual feasible | (2,2.5) | 11 |
| RHS1:4→6 | Primal infeasible, dual feasible | (2,3) | 12 |

For the last case the retained basis predicts(2,4),slack-1,value14.
That is an infeasible diagnostic, not the optimum. All four changed models
were actually opened/saved/solved in TORA and their summaries visually reviewed.
The original has no new extension screen; retained-basis flags/values come from
independent algebra and are labeled separately in the fixture.

## Verification

Tests first failed on the missing API, then passed expanded core checks.
Independent vertex enumeration checks100small before/after2D models with bounds,
signed coefficients, min/max, relations and joint changes. Edge cases include
fixed/negative bounds, free variables, equality redundancy, infeasible/unbounded
changes, malformed/nonfinite vectors and nonoptimal baselines.

Four authenticated original-model comparisons check44outcome/basis fields.
Fixtures reconstruct actual saved bytes and require their original SHA256
before decoding and calling the backend. `tests/fixtures/postoptimal.json`
records accepted REA evidence for the baseline and four examples. Raw captures
remain private under `reference/tora-captures/postoptimal/`.
Integrated suite after Games addition:46/46Qt-free in3.93s;
112/112desktop in101.77s. Final focused backend/probe checks6/6Debug in0.55s
and6/6Release in0.30s include both Cj/B checks. Logs:build/games/final-*-focused-*.log.
No new UI, cross-platform CI/package or installed release is claimed.

Primary teaching reference:[MIT1.731Lecture9](https://web.mit.edu/1.731/www/Lectures/Lecture06_9.pdf).
The retained-basis implementation and independent examples were derived for
this class extension, not copied from decompiler output.


## Native integration update — 16 October 2026

Local source now exposes Cj/RHS application through SOLVE Menu -> Class extensions
-> Change Cj / RHS. Draft analysis keeps baseline/input separate; the result
shows fresh solution and retained-basis diagnosis. Use changed model updates only
Cj/RHS in the existing input document, preserving precision/path/read-only/saved
checkpoint; normal Save As persists it. Native trace, formatting and selected-step
printing are verified locally. See LP_CHANGES_NATIVE.md. Assignment/Games integration,
report and cross-platform release/physical printing remain outstanding.
