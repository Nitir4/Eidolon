# Shared LP iteration screen

## Purpose

The iteration screen teaches pivots and displays the complete accumulated
sequence. All-slack, M-method, Two-phase, Bounded simplex, and Dual simplex
use this shared presentation with different titles and rows.

## Interaction directly observed

The heading is `SIMPLEX TABLEAU - (...)`, followed by the problem title and
objective direction. Instructions say to choose an ENTERING NONBASIC variable
column (correct selections turn green), choose a LEAVING BASIC variable row
(correct selections turn red), and use NEXT ITERATION or ALL ITERATIONS.
The latter commands may execute without selecting either or both pivots.
These instructions were exercised on the normal model. Incorrect columns and
rows trigger the exact Entering Variable/Leaving Variable dialogs transcribed
in `LP_ALL_SLACK.md`; accepted x1/sx4 selections produce Iteration 2.

Controls are `Next Iteration`, `All Iterations`, `Write to Printer`,
`View/Modify Input Data`, `MAIN Menu`, and `Exit TORA`. After All Iterations
reaches termination, the Next/All buttons appear disabled in the captured
normal/equality traces. Alternative-optimum dialogs instruct the user to
select a nonbasic zero-reduced-cost variable and use NEXT ITERATION. This was
executed on case 05: x2 enters, x1 leaves, and Iteration 3 displays the other
optimum (0.00,4.00), still with objective 4.00. Evidence:
`probes/alternative-20261004/observations.json`.

## Table layout and display

Rows start with an iteration/phase label and a header containing `Basic`,
ordered variable columns, and `Solution`. The objective row is `z (max)` or
`z (min)`. Basis rows follow, then `Lower Bound`, `Upper Bound`, and
`Unrestr'd (y/n)?`. Unbounded upper bounds display `infinity`. Most current
captures use two decimal places. Phase 2 artificial columns can show `blocked`.

Observed ordinary-slack labels are `sx3`, etc.; surplus labels are `Sx3`,
etc.; artificial labels are `Rx5`, etc. A finite-upper-bound example contains
`Ux1`. These spellings and ordering must come from each actual captured trace,
not from a solver library's default naming.

Accumulated iterations are appended below prior tables. The native grid can
have both horizontal and vertical scrollbars: artificial columns move the
Solution column beyond the initial viewport. The observation tool now records
both horizontal positions with overlapping vertical frames and retains the
last stable view. A single top screenshot does not establish trace coverage.

## Evidence

`reference/tora-captures/methods/lp/corpus/01_normal/all_slack/` establishes the
normal three-table progression. `06_equality/m_method/` establishes the wide
four-table sequence with left/right frames. Method-specific documents describe
phase labels, fallback warnings, and terminal dialogs.

## Unknowns / things I could not verify

The coverage audit passes all 79 full traces. They remain marked
`assembled-transcription-needs-review`, rather than asserting that every
individual cell has independent human approval. Keyboard controls,
column-width rules, and every disabled-state transition remain unverified.
Successful printing is unavailable in this Wine environment; the observed
runtime error is documented in `FILE_BEHAVIOR.md`.
