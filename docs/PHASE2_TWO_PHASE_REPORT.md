# Phase 2 Two-phase gate report

Two-phase now runs through the shared Qt-free PrimalSimplexSession and native
iteration view. It constructs the feasibility objective, retains a separate
Phase 1 completion state, appends the Phase 2 transition table, restores the
original objective and blocks artificial entering columns. Zero-valued
artificial basic rows remain, as directly observed. Input/file/formatting and
the earlier All-slack/M controls are shared.

Tests preceded implementation: numerical and GUI builds failed for missing
TwoPhaseSession, TwoPhase enum and requestTwoPhase interfaces. Evidence is in
build/phase2-two-phase/tests-before-implementation.log and
gui-tests-before-implementation.log. The full scientific golden then exposed
nonzero basic reduced costs; the final feasibility reconstruction and canonical
basic costs preserve exact objective zeros and the separately observed
1.11e-16/5.55e-17 constraint residuals. No display rounding patch was added.

The original fifteen selections were reused. Eight new direct problems cover
finite bounds, an unrestricted variable, redundant/zero equalities, a restored
objective pivot, a bound conflict and mixed slack/artificial rows. A full
scientific run adds six tables: 24 numerical sessions (16 direct including
that notation run, eight All-slack fallbacks), 99 complete tables and 5278
exact cell strings. All 6872 side-by-side fields match. Source screenshot
hashes and unresolved/overlap checks pass; evidence is in
build/phase2-two-phase/oracle-verified.json and tests/fixtures/two-phase.json.

Focused manual captures resolve the feasible-phase message and its timing:
Next reports Phase 1 / End of Phase 1: Feasible solution found--initiating
Phase 2. Acknowledging leaves the last Phase 1 table; the next Next appends
Phase 2. All Iterations passes through without that intermediate message.
Cancel on the all-<= warning still uses All-slack. These supplementary
observations are private under probes/phase2-two-phase and do not inflate
the numerical-session count. TORA remains running on the isolated Wine
800×600 display; no reference process is killed between cases.

Local desktop suite: 38/38; explicit C++17 Qt-free suite: 16/16. Each of four
scales runs 29 Two-phase Qt cases, alongside the earlier 29 M, 29 All-slack
and 43 input cases. Native tests cover every golden cell, actual menu/format
controls, phase notices, blocked-column rejection, stale-input cancellation,
100×100 tables, navigation pause, PDF non-mutation and layouts from 1366×768
through 4K. Existing numerical goldens remain unchanged and pass.

Three-OS CI and downloaded package verification are pending. The gate is not
ready until all required checks pass. Remaining unobserved arithmetic,
keyboard, original print-layout and physical mixed-DPI limits are explicit
in LP_TWO_PHASE_MODULE.md and OPEN_QUESTIONS.md.

For review, restart ./build/dev/src/desktop/ToraClone, open an LP with >= or =
constraints and select Solve Problem → Algebraic → Iterations → Two-phase
method. Choose output format, then use Next or All Iterations. A normal all-<=
problem shows the captured fallback warning and uses All-slack.
