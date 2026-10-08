# All-slack starting solution

## Purpose and selection

TORA offers this as `All-slack starting solution` under `Solve Problem →
Algebraic → Iterations`. The screen calls it `Starting All-Slack Method`. This
is the observed default tableau path for models whose initial slack basis can
be used. Observed lower-bound/RHS normalization and the resulting method
classification are now specified in LP_ALL_SLACK_MODULE.md.

## Inputs and interaction

The method consumes the active LP input grid and then shows the common output
format dialog: decimal (`NNNNN.DD`) or scientific (`N.NNNNeDD`) notation, with N
and D values. The screen instructs the user to click a nonbasic-variable column
to enter, click a basic-variable row to leave, and select `Next Iteration`.
`All Iterations` may run without manually choosing either or both pivots.

## Output and directly observed trace

The table is headed `SIMPLEX TABLEAU - (Starting All-Slack Method)`. It shows
an iteration heading, `Basic`, variable and slack columns, `Solution`, then
`Lower Bound`, `Upper Bound`, and `Unrestr'd (y/n)?` rows. Values in the smoke
trace use two decimal places; open upper bounds display `infinity`. The screen
also has `Next Iteration`, `All Iterations`, and `Write to Printer` controls.

For the smoke model, Iteration 1 has `z (max)` coefficients -3.00 and -2.00,
slack basics sx3/sx4/sx5, and RHS values 4.00, 2.00, and 3.00. Iteration 2 has
objective value 6.00 and basis sx3/x1/sx5. The final table
displays the `Optimal Tableau` dialog with `Iteration 3 is optimal`. The
selected pivot row and column are highlighted in color.

## Warning and method redirection observed

An initial sweep recorded TORA's `Warning` dialog for seven cases when
All-Slack was selected:
`Problem has >= and/or = constraints, thus requiring M or Two-phase method.
Two-phase method will be used. Else click CANCEL to use M-method`. A fresh
`03_infeasible` capture confirms the default OK path enters the Two-phase
tableau. Every corpus selection now has a validated first-table heading and
complete trace. The Cancel branch is captured separately below. One
negative-RHS fixture contains an input `>=` yet did not show the warning, so the
trigger depends on the processed model. Phase 2 now verifies normalization
and the seven recorded redirects; see LP_ALL_SLACK_MODULE.md.

## Unknowns / things I could not verify

Hidden near-tie thresholds, cycling protection and numerical precision remain
unverified. All fifteen selections have full numeric transcriptions. Green entering
columns and red leaving rows are verified. The shared policy comparison is
in `LP_PIVOT_POLICY.md`; normal printer output remains unavailable under Wine.

## Entering-variable validation directly observed

On the normal initial tableau, clicking a column other than the required
entering column displays a dialog titled `Entering Variable`:

`If there is a tie, TORA always selects left-most TIED col only. Else, your selection is not consistent with Optimality Condition. Re-select or click NEXT/ALL ITERATIONS button.`

Evidence: `reference/tora-captures/probes/targeted-20261004/manual-pivot/basic-column.png`
and the captured recovery dialog. The normal objective coefficients are
-3.00 and -2.00; selecting x2 also triggered the same validation message.
This establishes that manual selection is checked against TORA's entering
rule. The corpus comparison is in `LP_PIVOT_POLICY.md`; the leftmost tie
instruction is direct text, not a textbook assumption.


## Leaving-variable validation directly observed

With x1 selected on the normal initial tableau, clicking sx5 (whose x1
coefficient is zero) displays `Leaving Variable`:

`If there is a tie, TORA always selects top-most TIED row only. Else, your selection is not consistent with Feasibility Condition. Reselect or click NEXT/ALL ITERATIONS button.`

Choosing sx4 instead highlights that row red. Next Iteration then appends
Iteration 2, whose basis is sx3/x1/sx5 and objective value is 6.00.
The accepted x1 column is green. These captures are in
`reference/tora-captures/probes/targeted-20261004/manual-pivot/`:
`zero-pivot-row.png`, `choose-sx4.png`, and `next-iteration.png`.
This directly establishes topmost tie validation and acceptance of the
normal minimum-ratio row. The other captured method selections are compared
in `LP_PIVOT_POLICY.md`.


The Cancel branch of the All-slack redirect warning was executed on case 03.
It opens `M-Method` / `Enter value of M:` with 100 selected, rather than the
Two-phase output setup. Evidence: `probes/validation-20261004/cancel-to-m/after-cancel.png`.
Return alone did not advance this value screen; its Go button requires explicit
activation and focus. Output-format navigation under Wine proved less reliable
in this route and is recorded separately from the algorithm result.

## Shared input, UI and file contract

Input fields, relations, names, dimension limits and bounds/sign flags are
specified in `LP_INPUT.md`. Setup uses `LP_OUTPUT_FORMAT.md`; M alone adds
`LP_M_VALUE.md`. Ordered rows/columns, labels, number strings, highlighting,
scrolling and manual controls are specified in `LP_ITERATION_SCREEN.md`.
Observed automatic rules and exact validation/termination text are in
`LP_PIVOT_POLICY.md` and `LP_ALL_SLACK.md`. Model save/open behavior is shared
and specified in `FILE_BEHAVIOR.md` / `../FILE_FORMAT.md`. These shared
contracts apply to this method; unsupported edge cases stay explicit.

## Phase 2 implementation evidence

The per-method contract in LP_ALL_SLACK_MODULE.md supersedes earlier unknowns
resolved by focused captures: reverse bound complements, x1+/x1- direction
changes, the measured near entering tie, and the crossing scientific residual.
It records 20 direct sessions (19 unique problems) and 57 reviewed ordered
tables, distinct from the seven original artificial-method redirects.
