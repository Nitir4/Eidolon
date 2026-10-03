# Observed pivot and termination policy

## Evidence and limits

This document describes what the captured TORA runs establish. It is not a
claim about a particular textbook implementation. The corpus has 79 complete
numeric traces, including 73 main selections and six targeted finite-bound
runs. Two further selections produce equality precondition warnings.
`methods/lp/corpus/coverage-audit.json` validates 273 tables and 7,711 numeric
cells against their contributing screenshot hashes.

The selection colors were read from original cells: green entering column,
red leaving row. Where an iteration has no selection colors, no selection was
invented. Comparing the visible costs and RHS ratios to the selected colors
found 144/144 most-improving/leftmost primal choices, 129/129 positive-coefficient
minimum-ratio/topmost primal choices, and 5/5 directly applicable Dual choices
including the six finite-bound traces. The reproducible check and per-table
provenance are `tools/audit_tora_pivots.py` and
`methods/lp/corpus/pivot-policy-audit.json`. The earlier 133/119 counts
covered the main traces only.
Arithmetic here checks a policy against captured selections; it never supplies
missing golden numbers. Exact hidden tolerance thresholds remain unknown.

## Primal simplex steps inferred from captured choices

1. Preserve input constraint order and normalize a negative RHS. Case 12's
   `>= -1` becomes an initial slack row with -1.00/-1.00 coefficients and RHS
   1.00. Allocation of surplus/slack/artificial labels is documented in
   `LP_ALGEBRAIC.md`.
2. Read the displayed z row. For `z (max)`, select its most negative eligible
   coefficient. For `z (min)`, select the most positive. Phase 2 artificial
   objective cells are literal `blocked` and are excluded.
3. On an exact displayed tie, choose the leftmost tied column. TORA's entering
   validation dialog explicitly states this rule. Case 14 chooses x2 before
   x1 despite x1's smaller index, separating this rule from Bland's rule.
4. Among positive entries of the entering column, choose the minimum
   nonnegative RHS/entry ratio; ties choose the topmost row. TORA's leaving
   validation dialog explicitly states the topmost rule. Case 11 admits a zero
   step, so zero RHS is not excluded. Finite variable bounds can win the step
   before a row, producing a complemented column as in `LP_BOUNDED_SIMPLEX.md`.
5. Pivoting replaces the leaving row's basis label, normalizes its pivot entry,
   and eliminates the entering coefficient from the other rows. The full
   observed before/after tables, including negative zero, are golden evidence;
   do not regenerate their display strings with a generic solver formatter.
6. An improving column without a leaving candidate reports
   `Last tableau shows solution is unbounded` in the Optimal Tableau dialog.
   Normal stopping reports `Iteration N is optimal`.

## Artificial variables and phases

M is a finite numeric input, observed default 100. Its objective construction
is captured directly, including -203.00/-202.00 for the equality maximization
case and +198.00/+197.00 for the minimization case. Do not replace finite M with
symbolic infinity. The infeasible case 03 at M=100 stops at Iteration 3 and
reports optimal even though Rx5 remains positive (2.00). The last objective
is -194.00. This is repeated observed legacy behavior: M optimality alone does
not establish primal feasibility in this TORA path. The clone needs an explicit
compatibility policy, with this trace as a regression case.

Two-phase starts with `z (min)` and artificial-feasibility costs. In case 06,
Phase 1 values are 6.00, 4.00, 2.00, 0.00 across Iterations 1–4. Phase 2 is
labelled Iteration 5, restores `z (max)` with value 11.00, and shows `blocked`
in the three artificial objective cells. Artificial columns remain visible.
An infeasible Phase 1 reports exactly `Phase 2 nixed. Phase1 shows the problem
has no feasible solution`. The final-summary iteration count can differ from
the interactive path: case 06's interactive terminal is Iteration 5, whereas
its Algebraic final summary says 6. Preserve both observed paths.

## Dual simplex inference

For directly applicable Dual traces, the leaving row has the most negative
RHS, with the usual screen topmost tie validation. Eligible entering entries
are negative in that row. Select the smallest z-coefficient/row-coefficient
ratio, using leftmost tied order. Case 10 starts with RHS -4/-6/-5, selects
Sx4 then x2 (ratio 1.50 rather than x1's 2.00), and finishes at Iteration 4.
All five observed negative-RHS pivots match this rule. Equality is rejected
with `Equations must be converted into inequalities before applying Dual
Simplex`. Other models redirect after a warning; the selected menu choice
must not be confused with the actual algorithm named in the output heading.

## Alternative optima and degeneracy

TORA's terminal notice uses the spelling `Alternative soltuion(s)` and directs
users to choose a nonbasic variable with zj-cj=0 in the last tableau and click
Next Iteration. The final summary uses `Alternative solution(s) detected`.
Case 02 reports alternatives at a degenerate singleton optimum; retain its
notice as observed behavior instead of inferring a different vertex exists.
The manual case-05 probe selected zero-cost x2 after the optimal notice,
then Next Iteration. TORA selected x1 to leave and appended Iteration 3 with
basis x2/sx4/sx5, values x1=0.00/x2=4.00, and the unchanged objective 4.00.
The reviewed table, selection screenshots and hashes are in
`probes/alternative-20261004/observations.json`. No anti-cycling policy or
iteration cap is established by these finite examples.

## Unknowns / things I could not verify

Internal arithmetic precision, tolerance/near-tie thresholds, cycling policy,
iteration limits, symbolic-M support (none observed), and all unrestricted or
inconsistent-bound combinations. A finite set of screenshots cannot prove
these policies universally; targeted extension fixtures must resolve any
implementation ambiguity before the corresponding Phase 2 module gate.
