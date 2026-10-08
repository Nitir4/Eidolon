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
printing without numerical mutation. Offscreen Qt layout tests cover 1366×768,
1920×1080 and 4K at 100/125/150/200% scaling.

Local full desktop suite: 26/26. Explicit C++17 / Qt-free suite: 12/12.
Final [three-OS CI](https://github.com/Nitir4/TORA-Clone/actions/runs/37731683552)
verifies source `09615b8a7c526b48b170ff40e00e7af07128046c`. Windows,
Linux and macOS each pass 26/26 CTest checks (78/78 overall), with no failed or
skipped tests. Downloaded complete logs confirm 29/29 simplex Qt cases at each
of four scales per OS and all 3241 side-by-side fields matching.
Packaging, deployed startup and artifact upload pass on all three OSes: portable
Windows EXE ZIP, Linux x86_64 AppImage and universal arm64+x86_64 macOS app-in-DMG.
Full downloaded test evidence is under build/phase2-all-slack/ci-final/; the
verified summary is build/phase2-all-slack/ci-verified.json. All three downloaded
packages match their SHA-256 sidecars; Windows ZIP integrity passes and the Linux
AppImage passes local offscreen extract-and-run startup. Package hashes are
recorded in build/phase2-all-slack/packages-verified.json.

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
The complete side-by-side numerical/label comparison is in
build/phase2-all-slack/comparison.tsv (3241 matching fields). Each CI report
also contains all-slack-comparison.tsv.
The direct all-slack module is ready for its review gate. There are no known
golden mismatches. Phase 2 as a whole remains in progress. Work stops here
for review; the next separate method is M-method, subject to approval.
