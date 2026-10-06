# Foundation 2 gate report

Foundation 2 implements the numerical library independently of Qt. Foundation
5 was approved when the user authorized this step on 2026-10-07. The approved
foundation order remains **1 → 5 → 2 → 3 → 4**. Stop after this gate for review;
Foundation 3 is not authorized until the user approves Foundation 2.

## Implemented

- C++17 static `or_core` library: binary64 scalar and checked, value-owned
  row-major matrices with replaceable storage scalar.
- Ordered tableaux with objective/RHS conventions, basis labels, independent
  snapshots and display metadata for blocked artificial objective cells.
- Caller-requested pivots with explicit tolerance, consistent compiled
  operation order, finite-result checks and transactional failure handling.
- Finite absolute/relative comparisons, including exact-zero policy and
  overflow-safe comparisons. Engineering defaults are not asserted to be
  hidden TORA thresholds.
- Locale-independent decimal/scientific formatting, verified halfway rounding,
  tiny-negative signs, D=0 trailing point, scientific exponent padding and
  carry, scientific N=0, and ten-place precision. Signed infinite sensitivity
  endpoints use `infinity` / `-infinity`.
- Safe diagnostics for nonfinite cells, invalid settings, invalid dimensions,
  duplicate basis/labels and overflowing pivots.

No pivot-selection algorithm, LP model, file codec or solver session is added
at this foundation. The reviewed native navigation preview is retained.

## Tests and oracle evidence

Tests were written before the corresponding implementation. `core.numerics`
compares all cells of the normal initial tableau and two requested pivots
against independently reviewed TORA Iterations 2/3, with ordered labels/basis.
It also checks unrounded fractions, custom thresholds, immutable copies,
signed-zero preservation, overflow rollback and failure diagnostics. A
computed quotient .036125/.017 remains unrounded in storage and matches
TORA's objective display of 2.13.

`core.formatting` compares 80 exact display strings with TORA, repeating each
under a comma decimal locale. It checks signed infinite endpoints, NaN/invalid
settings and isolation from arithmetic storage. The normal arithmetic input
and original transcribed numbers are committed; raw captures are not.

| Input / setting | Actual TORA | Clone golden expectation |
| --- | --- | --- |
| -2.125, decimal D=2 | -2.13 | -2.13 |
| -1.005, decimal D=2 | -1.01 | -1.01 |
| -0.0001, decimal D=2 | -0.00 | -0.00 |
| -2.5, decimal D=0 | -3. | -3. |
| 9.995, scientific N=2/D=1 | 1.00e+1 | 1.00e+1 |
| 1e-12, scientific N=5/D=3 | 1.00000e-012 | 1.00000e-012 |
| -2.5, scientific N=0/D=2 | -3.e+00 | -3.e+00 |
| 16777217, scientific N=10/D=2 | 1.6777217000e+07 | 1.6777217000e+07 |
| .00000000005, decimal D=10 | 0.0000000001 | 0.0000000001 |

The completed Phase 0 corpus was reused without a new sweep. Nineteen focused
inputs were observed through TORA under Wine at 800×600: seventeen successful
output paths and two captured runtime errors. Four additional final summaries
resolve cell clipping. TORA's decimal N=0 setting produced runtime error 5;
the 1e-100/1e100 input produced runtime error 6 (Overflow). The clone diagnoses
invalid settings/overflow safely rather than reproducing crashes. TORA was
reused for successive cases; it exited after its runtime-error dialogs were
acknowledged. GUI driving needed window-placement recovery and a Save As retry;
one reference window also disappeared during positioning and was relaunched.
Successful captures were visually reviewed, not inferred from script completion.

Capture hashes, cell mappings, the representation decision and remaining
limits are in [NUMERIC_FOUNDATION.md](tora-analysis/NUMERIC_FOUNDATION.md).
Source/test/capture mapping is in [TRACEABILITY.md](TRACEABILITY.md).

The initial shortest-spelling formatter failed the newly captured neighbour
case (-2.12 versus TORA -2.13). Targeted cutoff/scientific probes isolated
a 15-significant-digit display intermediate; the corrected formatter passes
all 80 strings, including more distant values that must round downward.

## Verification state

Local Debug tests passed **9/9**, including the existing four-scale GUI suite.
The Qt-free headless build/tests passed **3/3**; core formatting matched all
80 reviewed strings under both locales. Final code has no known failing checks.
The three-OS CI build/test/package result is pending. This report must be
updated from actual CI results before declaring the gate ready for review.

## Recorded limits and next gate

Binary64 matches the verified foundation comparisons; TORA's internal type
and every arithmetic edge are not established. No rational solver backend is
currently required by these comparisons. Full N/D field extrema and every
legacy diagnostic still belong to input/output gates. Solver near-tie/zero
thresholds, cycling and iteration caps remain explicit Q-01 module obligations.
There are no known mismatches in the reviewed golden cells/strings; these tests
do not claim that all LP solvers have been implemented or verified.

After approval, the next step is Foundation 3: LP model and validation.
