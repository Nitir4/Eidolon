# Dual simplex

## Purpose and selection

TORA presents Dual simplex as an iterative algebraic choice. The observed precondition is explicit: equality constraints must be converted into inequalities before this method can be applied.

## Direct observation

TORA exposes `Dual simplex` under `Solve Problem → Algebraic → Iterations`.
Selecting it on a model with an equality displays a modal titled `Dual
Simplex Error` with the exact message `Equations must be converted into
inequalities before applying Dual Simplex`. Evidence for this condition is
`reference/tora-captures/methods/lp/dual_simplex/equation-requirement-warning.png`.

On the all-`<=` `01_normal` fixture, TORA warns `Dual Simplex not applicable
directly. All-slack will be used`; acknowledging it presents the All-Slack
tableau. On the `03_infeasible` fixture, it warns `Dual simplex not applicable
directly. Two-phase method will be used; else click CANCEL to use M-method`.
The default OK action enters the Two-phase tableau; Cancel is offered by the
latter warning. Both warning screenshots are preserved beside their redirected
tableaux in the case corpus.

On the captured all-inequality seed (`10-dual.txt`, minimize 2x1+3x2 with
three ≥ constraints), TORA opens `SIMPLEX TABLEAU - (Dual Simplex Method)`.
The initial table has columns `Basic`, x1, x2, Sx3, Sx4, Sx5, `Solution`; its
initial objective row is -2.00, -3.00, 0.00, 0.00, 0.00, 0.00, and the three
slack rows have RHS values -4.00, -6.00, and -5.00. `All Iterations` reports
`Iteration 4 is optimal`. Evidence is in `reference/tora-captures/methods/lp/dual_simplex/`.

## Corpus algorithm observations

All fifteen selections are transcribed, except the two expected equality
warnings which prevent output. Applicable and redirected paths are distinct.
Five observed negative-RHS pivots choose the most negative RHS and the smallest
eligible z/negative-row ratio. Case 10 selects Sx4/x2, then Sx5/x1, then
Sx3/Sx5; it stops at Iteration 4 with objective 10.00. See `LP_PIVOT_POLICY.md`
for the comparison and direct tie-validation language shared by the screen.

## Unknowns / things I could not verify

Near-tie thresholds, an applicable Dual infeasible case with no entering
candidate, cycling safeguards and internal precision remain unverified.
The captured infeasible corpus case redirects to Two-phase rather than
establishing direct Dual infeasibility behavior.

## Shared input, UI and file contract

Input fields, relations, names, dimension limits and bounds/sign flags are
specified in `LP_INPUT.md`. Setup uses `LP_OUTPUT_FORMAT.md`; M alone adds
`LP_M_VALUE.md`. Ordered rows/columns, labels, number strings, highlighting,
scrolling and manual controls are specified in `LP_ITERATION_SCREEN.md`.
Observed automatic rules and exact validation/termination text are in
`LP_PIVOT_POLICY.md` and `LP_ALL_SLACK.md`. Model save/open behavior is shared
and specified in `FILE_BEHAVIOR.md` / `../FILE_FORMAT.md`. These shared
contracts apply to this method; unsupported edge cases stay explicit.
