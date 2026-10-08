# Phase 2 all-slack simplex contract

This method gate implements All-slack starting solution and its native iteration
screen. It reuses the approved LP input/file contracts. No other solver is
implemented in this cycle.

## Evidence and coverage

`tests/fixtures/all-slack.json` contains 27 observed input classifications:
20 direct sessions (19 distinct problems), 57 complete ordered tables, and
seven selections that TORA actually redirects to Two-phase. Each direct record
identifies its private capture or transcription. Existing corpus and finite-bound
captures were reused. The manual alternative pivot is from
`probes/alternative-20261004/observations.json`.

New focused inputs are in `probes/phase2-all-slack/`: slack-minimum,
slack-unbounded, slack-row-tie, slack-near-tie, slack-complement-return,
slack-unrestricted, slack-unrestricted-negative and slack-unrestricted-crossing.
Every successful case includes input, saved file, output format, initial table,
terminal dialog and all scrolled tables. The crossing-precision and
crossing-scientific probes resolve a specific rounded-zero ambiguity; their two
independently read cells are additional goldens, not invented complete traces.
The initial-optimal notice is also backed by Foundation 2's signed-zero capture.
Raw reference images/files remain private and excluded from source/packages.

## Purpose, inputs and method eligibility

Choose Solve Problem → Algebraic → Iterations → All-slack starting solution
for the all-slack initial basis. Input fields, editing, defaults and the observed
100-variable/100-constraint limit are shared with LP_DATA_ENTRY_MODULE.md.
Output format is the observed decimal/scientific N/D panel; its native supported
precision range remains 0..99 with decimal N at least one.

Finite lower bounds shift the model before RHS normalization; a negative RHS
negates its row and reverses <=/>=. A remaining >= or = triggers the exact
observed Warning from LP_ALL_SLACK.md. OK selects the Two-phase setup;
Cancel selects M-value setup. These dependent algorithms are separate method
gates, so Go remains unavailable after this handoff in the current product.
Those seven traces are classification tests, not computed all-slack goldens.

## Observed iteration policy and native numerical implementation

- Maximize selects the most negative nonbasic objective coefficient; Minimize
  selects the most positive. Exact ties select the leftmost column. The new
  3 versus 3.0000000001 probe selects x2 although both show -3.00.
- Positive entering-column coefficients participate in the nonnegative minimum
  ratio; exact ties select the topmost row. The row-tie capture selects sx3.
  Zero-RHS degeneracy retains a zero-length pivot and its new table.
- A finite entering upper range can cause a bound complement with no leaving
  row: x1 becomes Ux1, lower-shifted Lx1 becomes ULx1. Original bound rows remain
  unchanged. The new complement-return capture restores Ux1 to x1.
- A finite basic upper bound participates in the opposite-direction ratio.
  This bounded-variable implementation is used within the all-slack method;
  the separate Bounded simplex menu method remains a later gate.
- Unrestricted variables keep one column. Positive/negative improving
  directions are labeled x1+/x1-. A nonbasic column can reverse its orientation;
  crossing gives bases sx3/sx4/sx5 → x1+/sx4/sx5 → x2/sx4/sx5 → x2/x1-/sx5.
- No improving nonbasic column gives an optimal notice. A zero nonbasic reduced
  cost gives the observed alternative notice, including `soltuion(s)` spelling.
  Selecting a zero-cost nonbasic column and Next appends an alternative table.
  A selected improving direction without a finite ratio/bound gives the captured
  `Last tableau shows solution is unbounded` notice, without a duplicate table.

Manual entering/leaving choices are checked against the same rule and use the
exact observed error titles/bodies in LP_ALL_SLACK.md. Invalid choices leave
both the numerical snapshot and accepted selection intact.

The native library uses binary64, strict comparisons and value-owned snapshots.
It reconstructs numerical tables from original equations and a basis inverse
with ascending basis columns. This is our implementation choice; observation
cannot establish TORA's internal algorithm. It matches every golden, including
the crossing cell -5.55112e-17 (scientific N=5/D=2) and -0.000000000000
(decimal D=12). Forced exact zeros and display-dependent arithmetic were rejected
because they lose that observed residual. Formatting never changes stored values.
A native cap of 1024 automatic pivots retains a resumable partial trace and emits
an engineering diagnostic; this is not asserted to be TORA's cycling policy.

## Screen, labels and states

Native Qt Widgets fill the window with the familiar blue LP page, method heading,
title/direction, three instructions, Next Iteration / All Iterations / Write to
Printer, and View/Modify Input Data / MAIN Menu / Exit TORA. The table accumulates
Iteration headings, Basic, ordered decision/slack columns, Solution, objective
row, ordered basic rows, Lower Bound, Upper Bound and Unrestr'd (y/n)? rows.
Names, all displayed strings and ordering are compared directly with captures.

Headers/labels are cyan with blue text; numeric cells are blue with white bold
text. Nonbasic original-slack columns in constraint rows are white with blue
text. Entering columns turn green and leaving rows red, including the observed
bound/sign-row column shading. Next/All stop when a terminal result is reached;
manual selection enables the next alternative pivot. Initial optimal status
opens the notice on entering the screen. Horizontal/vertical scroll bars retain
access at large dimensions, small viewports or large N/D fields. Font metrics
measure the actual bold numeric font. New tables preserve scrolling.

All Iterations uses one pivot per Qt event-loop tick; leaving the screen pauses
it. Editing or opening input invalidates the prior session. Native printing
renders the current ordered tables through QPrinter, with page breaks. PDF
creation and numerical non-mutation are tested, but successful TORA printing
remains unavailable under Wine (Q-08); no print-layout fidelity is claimed.
No new file format, iteration export or shortcut is invented. Model Open/Save
continues through the approved codec and controller; iterations are not saved
as model data.

## Unknowns / things I could not verify

Hidden numerical thresholds outside the measured near-tie case, cycling/iteration
limits, upper-bound-versus-row exact ties, unrestricted zero-cost initial
orientation in every configuration, ill-conditioned basis behavior, and successful
TORA printer layout/pagination remain unverified. Physical per-monitor DPI moves
and OS-version acceptance require hardware review. Redirected infeasible/equality
computations await M/Two-phase cycles; they are explicitly not passed solver
comparisons in this gate. No currently asserted golden mismatch remains.
