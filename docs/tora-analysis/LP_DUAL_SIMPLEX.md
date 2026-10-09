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

Near-tie thresholds, cycling safeguards and internal precision remain unverified.
Direct infeasibility is now observed by the focused du-infeasible case below;
the original infeasible corpus case still redirects to Two-phase.

## Shared input, UI and file contract

Input fields, relations, names, dimension limits and bounds/sign flags are
specified in `LP_INPUT.md`. Setup uses `LP_OUTPUT_FORMAT.md`; M alone adds
`LP_M_VALUE.md`. Ordered rows/columns, labels, number strings, highlighting,
scrolling and manual controls are specified in `LP_ITERATION_SCREEN.md`.
Observed automatic rules and exact validation/termination text are in
`LP_PIVOT_POLICY.md` and `LP_ALL_SLACK.md`. Model save/open behavior is shared
and specified in `FILE_BEHAVIOR.md` / `../FILE_FORMAT.md`. These shared
contracts apply to this method; unsupported edge cases stay explicit.


## Phase 2 focused observations (2026-10-09)

The original fifteen selections are reused: thirteen numerical histories (41
complete tables) and two equality errors. Fifteen additional automatic probes
cover RHS and ratio ties, direct infeasibility, initially optimal and zero-RHS
models, zero costs, redundant rows, decimal/scientific display, negative RHS,
maximization and finite, fixed, lower or unrestricted variable bounds. Their
saved files, every scroll frame and glyph-derived tables are private under
`reference/tora-captures/probes/phase2-dual/`. No solver output supplies the
expected values.

Setup first rejects equalities. A nonzero lower bound, finite upper bound or
unrestricted variable shows `Dual simplex not applicable because a var has
upper/lower bound or is unrestricted. Two-phase method will be used; else
click CANCEL to use M-method`. OK enters Two-phase; the focused Cancel case
enters finite M with the observed default 100. The remaining direct seeds have
nonnegative minimization costs or nonpositive maximization costs. The opposite
signs redirect to All-slack when no artificial start is needed, otherwise to
Two-phase with the already recorded Cancel-to-M choice. This sign predicate is
an implementation inference supported by the captured setup cases; hidden
numerical thresholds remain unknown.

In a direct start, normalized >= constraints are negated to create negative
RHS values and unit basic surplus columns labelled Sx. Normalized <= rows
retain their signs and use sx. Constraints and variables keep their input
order. The objective row displays negative input costs. Inverse Sx cells have
white backgrounds in direct Dual, as observed, while the existing primal
methods retain their captured palette.

The most negative RHS leaves. Equal RHS values select the topmost row. Among
nonbasic columns with negative coefficients in that row, the smallest absolute
objective/row ratio enters; exact ties select the leftmost column. The focused
ratio and RHS fixtures establish these rules for exact ties. The running screen
retains TORA's shared entering-first instruction text. A valid leaving-row click
alone marks only the red row; clicking the entering column subsequently adds
green without clearing red. The manual probe captures both invalid selections
and these two independent highlights.

If a negative RHS row has no eligible entering column, the dialog title is
`Entering Var`, and its exact text is `Problem has no feasible solution--Iterations
stopped`. Nonnegative RHS terminates as optimal. Nonbasic zero objective cells
produce the shared alternative-solution notice, including its observed spelling
`soltuion(s)`. The seed reaches Iteration 4 with objective 10.00; the scientific
repeat retains observed small constraint-row residuals while basic objective
costs display exact zero. The alternative continuation was directly captured and reviewed: x2 enters
Sx3, appends Iteration 3 at objective 6.00, and displays no additional notice.
Next and All Iterations are disabled until another valid manual selection.
The accepted corpus totals 32 sessions, 106 complete tables and 5215 exact
displayed cell strings.

All normal input validation, dimensions, save/open/print behavior, output format
fields, navigation and scroll controls use the shared contracts referenced
above. Bounds are not silently discarded: their observed redirect is part of
the native setup flow.


The scientific alternative repeat resolves decimal negative-zero cells in
Iteration 3: x1/x2 is -4.44e-16, Sx5/x1 is -2.22e-16 and Sx5/x2 is -4.44e-16.
These are measured residuals, not exact signed zeros. Current-basis
partial-pivoted LU reconstruction reproduces the continuation profile while
preserving the separately observed direct Dual cross-product profile. This
is an implementation choice supported by exact output comparisons; TORA's
internal factorization is unknown. No cell-specific values are substituted.
