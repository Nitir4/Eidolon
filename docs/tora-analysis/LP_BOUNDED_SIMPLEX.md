# Bounded simplex

## Purpose, inputs and navigation

`Solve Problem → Algebraic → Iterations → Bounded simplex` consumes the same
LP model and lower/upper/unrestricted rows as the other choices. It uses the
common decimal/scientific output-format screen and the shared iteration view.
The visible title for the observed inequality examples is `SIMPLEX TABLEAU -
(Starting All-Slack Method)`.

## Direct finite-bound comparisons

Three additional problems were executed under both Bounded simplex and
All-slack. Their complete transcribed tables, headers, labels, pivot selections
and number strings are identical between the two choices for each problem.
Evidence: `reference/tora-captures/methods/lp/corpus/finite-*/`.
All maximize 4x1+3x2 subject to x1+x2≤10, x1≤6, x2≤7.

| Case | Variable bounds | Final tableau objective | Observed transformation |
| --- | --- | --- | --- |
| finite-upper | x1≤5, x2 open above | 35.00 | x1 column changes to Ux1 when its upper bound wins before a constraint pivot |
| finite-two-upper | x1≤2, x2≤3 | 17.00 | Both variable columns change to their upper-bound representation |
| finite-lower | 1≤x1≤3, 1≤x2≤4 | 24.00 | Initial columns Lx1/Lx2; later ULx1/ULx2 |

The original finite-upper-bound smoke evidence is also retained in
`methods/lp/bounded_simplex/`. `Ux1` identifies a transformed variable column,
not an extra basic-variable row. The original draft called it a basis label;
the full-table comparison corrects that terminology.

For finite-upper, Iteration 1 has objective coefficients -4.00/-3.00 and value
0.00. Iteration 2 has Ux1/x2 columns, objective 4.00/-3.00 and value 20.00;
constraint RHS values are 5.00, 1.00, 7.00. Iteration 3 has x2 as a basic row
and objective 35.00. For finite-lower, the starting objective is already 7.00
and RHS values are 8.00, 5.00, 6.00, directly confirming the lower-bound shift.
The stored bound rows still show original lower/upper values, not residual widths.

## Algorithm inference from these captures

The observations support x=L+y for a finite nonzero lower bound, and an
upper-bound complement when the step reaches U before a constraint ratio.
The column label changes without adding an upper-bound constraint row.
A later constraint pivot uses the transformed column. Keep this mapping in
the preprocessing/trace state, so the final presenter recovers original x.
These are inferences grounded in the six identical observed traces; they do
not establish every combination of signed/infinite/unrestricted bounds.

## Output and termination

The standard Basic/variable/Solution layout and bound/flag rows apply.
The tested bounds terminate with ordinary Optimal Tableau notices. No
mathematical recomputation was used to fill these transcriptions.

## Unknowns / things I could not verify

Inconsistent bound validation, unrestricted-variable expansion, a return from
an upper complement to the original representation, and exact numerical
thresholds for near-tied bound/constraint steps remain unverified. The three
comparisons do not prove the two menu selections are identical for every LP.

## Shared input, UI and file contract

Input fields, relations, names, dimension limits and bounds/sign flags are
specified in `LP_INPUT.md`. Setup uses `LP_OUTPUT_FORMAT.md`; M alone adds
`LP_M_VALUE.md`. Ordered rows/columns, labels, number strings, highlighting,
scrolling and manual controls are specified in `LP_ITERATION_SCREEN.md`.
Observed automatic rules and exact validation/termination text are in
`LP_PIVOT_POLICY.md` and `LP_ALL_SLACK.md`. Model save/open behavior is shared
and specified in `FILE_BEHAVIOR.md` / `../FILE_FORMAT.md`. These shared
contracts apply to this method; unsupported edge cases stay explicit.
