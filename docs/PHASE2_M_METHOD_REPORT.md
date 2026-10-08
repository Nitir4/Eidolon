# Phase 2 M-method gate report

M-method now has finite-M setup, a Qt-free session and the native iteration
screen. It shares preprocessing, pivot selection, immutable traces, formatting,
manual/automatic controls and printing with All-slack. The common engine is
PrimalSimplexSession; AllSlackSession and MMethodSession choose their observed
initialization and arithmetic policies without duplicating the solver or GUI.

Tests preceded implementation: the numerical and GUI targets failed for missing
M interfaces. Golden tests then exposed signed-zero arithmetic differences.
Focused scientific captures distinguish basis reconstruction from ordinary
incremental pivots. The final cross-product inverse and dual-row construction
match those observed residuals; no formatter or cell-specific patch was added.
An additional visual review corrected initial sx/Rx inverse-cell coloring.

Validation covers 15 direct M sessions and eight actual All-slack fallbacks:
78 complete tables, 4095 strings plus four scientific cells, all captured
selection metadata and exact terminal notices. The original completed corpus
was reused. Only finite-M/bound/free/scientific/Cancel gaps were observed anew,
using a launch-first, resumable observer on the isolated Wine 800×600 screen.
TORA remains running; no reference instance is killed between cases.

The finite-M compatibility behavior is explicit: the infeasible example still
reports optimal with a positive artificial variable; M=0/bogus share alternative
notices; M=-1 becomes unbounded; M=0.5 can stop with positive artificial value.
The fallback warning's explicit Cancel reaches All-slack output too.

GUI tests bind every golden table/label/value at four scales, exercise real
M/output controls, errors/manual pivots, both fallback responses, scientific
residuals, setup reset, input invalidation, PDF printing, 100×100 scrolling/pause
and 1366×768/1920×1080/4K layouts. Existing All-slack and input tests remain in
the full suite. Local full desktop suite: 32/32. Explicit C++17 Qt-free suite: 14/14.
All 5354 side-by-side comparison fields match. Three-OS CI and package
verification remain pending before the gate can be declared ready.

Recorded limits remain in LP_M_METHOD_MODULE.md and OPEN_QUESTIONS.md. No known
golden mismatch may be carried into the gate. Other LP methods remain gated.

For review after completion, restart ./build/dev/src/desktop/ToraClone, open an
LP with >= or = constraints, and choose Solve Problem → Algebraic → Iterations
→ M-method. Keep default M=100 or use a captured alternate value, then select
output format and Next/All Iterations. All-<= inputs show the fallback warning.
