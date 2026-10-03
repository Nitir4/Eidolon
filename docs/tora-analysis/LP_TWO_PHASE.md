# Two-phase method

## Purpose and selection

TORA presents the Two-phase method as an iterative algebraic choice and labels its tableaux Phase 1 and Phase 2. This two-stage workflow is directly visible in the captured run.

## Direct observation

The method is selected from `Solve Problem → Algebraic → Iterations →
Two-phase method`, followed by TORA's common output-format dialog. The observed
fixture is `08_two_phase` (minimize x1+x2, x1+x2=5, x1≥2, x2≥2). Evidence is
in `reference/tora-captures/methods/lp/two_phase/`.

The screen is titled `SIMPLEX TABLEAU - (Two-Phase Method)`. Its common
instructions identify entering and leaving selections by clicking a
nonbasic-variable column and a basic-variable row, and provide `Next
Iteration` and `All Iterations`. The first tableau heading is `Phase 1 (Iter
1)` and includes artificial variables `Rx5`, `Rx6`, and `Rx7`. The captured
accumulated output shows Phase 1 iterations 1–4 followed by `Phase 2 (Iter 5)`;
the Phase 2 tableau marks the artificial-variable columns `blocked`. Clicking
`All Iterations` displays `Optimal Tableau`, `Iteration 5 is optimal`, and an
alternative-solution message directing the user to select a nonbasic
variable with zero `zj-cj` and click `NEXT ITERATION`. It explicitly says
the Output Summary cannot show the alternative optimum solution.

For the directly captured `03_infeasible` fixture, pressing `All Iterations`
opens a dialog titled `Phase 1` with the message `Phase 2 nixed. Phase1 shows
the problem has no feasible solution` (spelling and spacing as displayed).
Phase 1 iteration tables remain visible behind that dialog.

Selecting Two-phase for an all-`<=` model also displays `Problem has all <=
constraints. All-Slack option will be used`; after acknowledging the warning,
TORA presents the All-Slack tableau. This was directly recaptured for
`01_normal/two_phase`.

## Corpus verification and algorithm

All fifteen main selections have complete, source-hashed numeric table
transcriptions; models that redirect to All-slack are labeled by their warning.
See `LP_PIVOT_POLICY.md` for observed entering/leaving rules, feasibility-phase
stopping, artificial `blocked` cells, and the different interactive/summary
iteration counts. `coverage-audit.json` checks every row/column and the ordered
phase/iteration sequence.

## Unknowns / things I could not verify

Near-zero Phase 1 feasibility thresholds, redundant equality/artificial-basic
cleanup beyond these cases, cycling safeguards, and internal precision remain
unknown. The artificial columns remain visible in the captured Phase 2 tables;
no deletion policy is invented for unseen cases.

## Shared input, UI and file contract

Input fields, relations, names, dimension limits and bounds/sign flags are
specified in `LP_INPUT.md`. Setup uses `LP_OUTPUT_FORMAT.md`; M alone adds
`LP_M_VALUE.md`. Ordered rows/columns, labels, number strings, highlighting,
scrolling and manual controls are specified in `LP_ITERATION_SCREEN.md`.
Observed automatic rules and exact validation/termination text are in
`LP_PIVOT_POLICY.md` and `LP_ALL_SLACK.md`. Model save/open behavior is shared
and specified in `FILE_BEHAVIOR.md` / `../FILE_FORMAT.md`. These shared
contracts apply to this method; unsupported edge cases stay explicit.
