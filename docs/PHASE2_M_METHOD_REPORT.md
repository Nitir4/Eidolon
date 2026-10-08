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
An additional visual review corrected initial sx/Rx inverse-cell coloring and
kept the Basic labels pinned during horizontal scrolling. Regression checks
exercise vertical synchronization, a real pinned-label leaving-row click and
an unclipped Solution column.

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
All 5354 side-by-side comparison fields match. Final source
`d334285a85cca0aa46b01ff709a92e0b2f4643e9` passed
[three-OS CI run 37785270579](https://github.com/Nitir4/TORA-Clone/actions/runs/37785270579):
32/32 CTest checks on Windows, Linux and macOS, zero failures or skips.
Downloaded reports confirm 29 M, 29 All-slack and 43 input Qt cases at each of
four scales per OS, plus all 5354 M and 3241 All-slack comparison fields.
Evidence is in `build/phase2-m-method/ci-reviewed-final/` and
`build/phase2-m-method/ci-reviewed-verified.json`. All three packaging,
deployed-startup and artifact-upload steps passed. All three downloaded package
SHA-256 checks pass; the Windows ZIP integrity, EXE and Qt PrintSupport checks
pass. The final Linux AppImage passes local offscreen extract-and-run startup.
The macOS package contains the universal arm64/x86_64 bundle verified by CI.
Packages are in `dist/phase2-m-method/final/`; evidence is in
`build/phase2-m-method/packages-reviewed-verified.json` and
`build/phase2-m-method/linux-reviewed-startup.json`. No original TORA files,
screenshots or executable are included in these artifacts.

The M-method gate is ready for review, with no known golden mismatch. This
completes this method's implementation cycle; Two-phase waits for approval.

Recorded limits remain in LP_M_METHOD_MODULE.md and OPEN_QUESTIONS.md, including
successful legacy print layout and physical mixed-DPI handoff. Other LP methods
remain gated.

For review, restart ./build/dev/src/desktop/ToraClone, open an
LP with >= or = constraints, and choose Solve Problem → Algebraic → Iterations
→ M-method. Keep default M=100 or use a captured alternate value, then select
output format and Next/All Iterations. All-<= inputs show the fallback warning.
