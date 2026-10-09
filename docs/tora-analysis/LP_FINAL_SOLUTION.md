# Algebraic final solution

## Purpose, inputs and navigation

`Solve Problem → Algebraic → Final solution` solves the active model and
shows its values and sensitivity ranges through the common output-format
screen. It consumes the same objective, ordered constraints, bounds and sign
flags as the iteration methods; input fields/limits are in `LP_INPUT.md`.
The final notice directs users to ITERATIONS mode for alternative solutions.

## Layout and exact labels

The heading is `LINEAR PROGRAMMING OUTPUT SUMMARY`. It reports `Title:`,
`Final Iteration No.:`, and `Objective Value (Max)` or `(Min)`, then its value.
The scrollable grid contains these sections in order:

| Section | Columns |
| --- | --- |
| Variable | Variable, Value, Obj Coeff, Obj Val Contrib |
| Constraint | Constraint, RHS, Slack-/Surplus+ |
| ***Sensitivity Analysis***, variable | Variable, Current Obj Coeff, Min Obj Coeff, Max Obj Coeff, Reduced Cost |
| Sensitivity, constraint | Constraint, Current RHS, Min RHS, Max RHS, Dual Price |

Variable labels are x1:, x2:, etc. Constraint labels include their index and
original relation marker (<, >, =); zero slack does not change that marker. Numeric slack/
surplus suffixes are presentation characters (case 13 has `4.00-`), separate
from leading negative signs. Open ranges use `infinity` or `-infinity`.
`Next Iteration` and `All Iterations` are disabled in the completed normal
summary; `Write to Printer` is enabled. Lower controls are `View/Modify Input
Data`, `MAIN Menu`, `Exit TORA`. The grid has both scrollbars.

## Fifteen independent final-summary runs

All fifteen captures live in `output-studies/<case>/final/`. Thirteen have
complete numeric/sensitivity transcriptions, with exact strings, cell boxes
and screenshot hashes in `transcribed-summary.json`. Cases 03/04 are reviewed
terminal-only summaries. Their exact messages are respectively `Problem has
no feasible solution` and `Optimum solution is unbounded`. Cases 02/05/08
show `Alternative solution(s) detected (enter ITERATIONS mode to determine
such solutions)`.

Normal reports Iteration 3 and objective 10.00, values 2.00/2.00, contributions
6.00/4.00. Variable sensitivity ranges are 2.00…infinity and 0.00…3.00,
reduced costs 0.00/0.00. Constraint RHS/slack pairs are (4.00,0.00),
(2.00,0.00), (3.00,1.00-); RHS ranges are 2.00…5.00, 1.00…4.00,
2.00…infinity; dual prices are 2.00/1.00/0.00. Use the fresh Normal capture
for its title; the older smoke capture has a different title.

The final-summary count can differ from interactive Two-phase: case 06 says
6 here and 5 interactively. Preserve both. Cases 13/15 have positive objectives
5.50/9.00; do not mistake the heading's equals sign for a negative sign.
Scientific N=5 and decimal D=3 are independently captured in
`probes/closing-20261004/`; see `LP_OUTPUT_FORMAT.md`.

## Unknowns / things I could not verify

The automatic final path matches the verified Bounded/Two-phase terminal basis
on the accepted corpus. Its count equals stored interactive tables plus one
after a Phase 2 transition; no extra count is added for infeasible Phase 1.
This is an observed display rule, not proof of TORA's internal implementation.
Ten focused cases independently verify upper/lower/fixed/free variables,
zero objective, aliases, redundant equality, scientific N=5 and decimal D=3.
Explicit bound rows are appended per variable (LB then UB). The original fifteen
plus ten focused results form tests/fixtures/final-solution.json.
Hidden precision outside the captured profile, successful original printing
and all large-grid scrolling states remain unverified. The Wine printer error is recorded in `FILE_BEHAVIOR.md`.


## Native implementation and verification

`FinalSolutionSession` automatically solves a validated LP through the shared
bounded engine, reconstructs original variables, and formats final sections.
`LpFinalController` routes the enabled solve action through Output Format into
a native summary grid. A model change invalidates the prior output. Next/All
Iteration controls are disabled on this screen. Printing uses native QPrinter.
Tests were written before each implementation; failures are retained under
`build/phase2-final/`. Twenty-five exact golden summaries and native cells are
checked at 100%, 125%, 150% and 200%, with actual 1366x768, 1920x1080 and 4K
window geometry. The full local suite passed 62/62 before Release validation.
Sensitivity implementation remains its separate next cycle.


## Sensitivity-cycle extensions to the final checkpoint

Ten further targeted summaries extend the original 25-case final checkpoint to
35 cases. Negative-lower automatic transforms, interleaved bound rows and signed
minimization coefficients now have independent screen regressions. See
`LP_SENSITIVITY_PARAMETRIC.md` for their exact legacy conventions. Sensitivity is
implemented in the same native final view; the earlier 62-test/645-field gate
remains a historical checkpoint, not the final expanded validation total.
