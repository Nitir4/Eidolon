# Phase 2 — all-slack simplex gate report

All-slack simplex now has a Qt-free numerical session and a working native
iteration screen. The original menu path opens output-format selection, then
manual Next or responsive All Iterations. Tables preserve TORA's labels, order,
bounds, number formatting, pivot colors and alternative-optimum workflow.
Finite-bound complements and unrestricted direction changes are implemented.

## Validation

Tests preceded implementation. New unrestricted tests and the rounded-zero
comparison failed before their fixes. Initial optimal-popup and inverse-cell
color tests also exposed failures before the UI corrections.

The golden fixture covers 27 classifications: 20 direct sessions across 19
unique problems, seven artificial-method redirects, 57 complete tables and
2396 exact cell strings (including bound/blank cells and two targeted notation
checks). All direct terminal messages match. The original completed corpus was
reused; only missing minimization/unbounded/tie/free-direction/complement-return
observations were run. Reference captures remain private.

Scientific notation revealed that the apparent -0.00 mismatch was actually
-5.55112e-17. Basis reconstruction reproduces it while preserving precision
independence. No cell-specific numerical patch or formatter change was used.

Qt tests compare every golden column, row and value in the native model at four
scale factors. They cover manual validation/highlighting, automatic iterations,
alternative pivots, initial optimal notices, real output-format controls/menu
selection, input invalidation, 100×100 scrolling and pausing, and native PDF
printing without numerical mutation. Populated layouts cover 1366×768,
1920×1080 and 4K at 100/125/150/200% scaling.

Local full desktop suite: 26/26. Explicit C++17 / Qt-free suite: 12/12.
Three-OS CI is pending; its final results will be recorded after verification.
This report is not yet a claim that the review gate passed.

## Scope and recorded limits

Seven existing selections redirect to Two-phase in TORA. Their warning and
OK/Cancel handoffs are tested here; Go remains unavailable for those solvers
until their separate method gates. They are not counted among the 57 direct
tables. The separate Bounded simplex menu remains disabled.

The native 1024-pivot guard and bounded formatter profile are explicit engineering
policies. Hidden tolerances/cycling, upper-bound exact ties, successful TORA
printing and physical mixed-monitor acceptance remain recorded unknowns, not
asserted compatibility. See the method specification and OPEN_QUESTIONS.md.

## Review on the laptop

Run `build/dev/src/desktop/ToraClone`. Choose Linear Programming, open an existing
LP or enter a problem, then SOLVE / Solve Problem → Algebraic → Iterations →
All-slack starting solution → Go To Output Screen. Next Iteration and All
Iterations now compute tables. Models requiring artificial variables reach the
pending M/Two-phase handoff described above.

Clone-only screenshots are generated in build/phase2-all-slack/ui-captures/.
The module stops for review after successful CI; the next separate method is
M-method, subject to approval.
