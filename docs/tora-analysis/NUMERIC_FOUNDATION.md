# Foundation 2 numerical policy and oracle evidence

These are clone implementation decisions backed by the observations below,
not claims about TORA internals. No executable was inspected. Completed
Phase 0 traces were reused; only focused formatting/precision inputs were
run through TORA's normal UI in its Wine 800×600 desktop on 2026-10-07.
Raw screenshots and TORA-saved files remain private under
`reference/tora-captures/probes/foundation2-20261007/`.

## Arithmetic and storage

`Scalar` is IEEE-754 binary64 (`double`), checked at compilation. A value-owned,
row-major `DenseMatrix<Value>` has checked dimensions/access and independent
copies. Its scalar parameter permits wider diagnostic storage; `long double`
is not guaranteed to be wider on every platform. A rational solver backend
is not needed by the verified comparisons at this gate. TORA's underlying
storage type has not been established by observing its UI.

`Tableau` contains ordered variable labels, basis indices, objective row zero,
and a final RHS column. It rejects invalid dimensions, duplicate labels/basis
indices and nonfinite cells. Optional blocked-objective metadata can describe
artificial-variable displays without putting a solver or phase transition in
this foundation. Missing LP bounds, objective sense and termination messages
belong to the model/solver layers, not this arithmetic container.

A pivot is supplied explicitly by the caller. The core divides the chosen
row, then eliminates that column in the other rows. It performs no pivot
selection, ratio test, snapping to zero, rounding or phase change. Floating
contraction/fast arithmetic is disabled for the compiled arithmetic target.
Results commit only after all cells remain finite; a rejected or overflowing
pivot preserves the previous values and basis. Copies provide independent
iteration snapshots. A caller may pivot with exact-zero tolerance or a
selected absolute threshold.

`Tolerance(abs, rel)` compares finite values using absolute plus relative
error scaled by `max(abs(a), abs(b))`; zero testing uses the absolute threshold.
The implementation avoids overflow when subtracting opposite extreme finite
values. NaN/infinity are never considered finite equal/zero values.
Defaults `1e-10`/`1e-12` are **engineering defaults, not observed TORA solver
thresholds**. Every numerical method must resolve and pass its own explicit
compatibility policy before its gate. Hidden near-tie/zero tests, cycling and
iteration caps remain Q-01 solver obligations.

## Number display

`NumberFormatter` accepts finite binary64 values and returns locale-independent
strings without modifying arithmetic. `formatBound` returns
`infinity` / `-infinity` for signed infinite range endpoints; NaN is diagnosed. Decimal settings use D
fractional digits. N does not truncate larger integer parts. D=0 retains a
trailing decimal point. Scientific settings use N mantissa fractional digits,
lowercase `e`, an explicit exponent sign and at least D exponent digits.
Larger exponents are preserved. Rounding carries into the next exponent.

Exact halfway cases and decimal-sensitive inputs round away from zero in the
reviewed captures. A small negative number rounding to zero retains `-0.00`.
The implementation first converts the magnitude to 15 significant decimal
digits, then applies half-away rounding at the requested precision. Targeted
neighbours distinguish this policy from rounding the shortest round-trip
spelling: 2.1249999999999996 displays as 2.13, whereas 2.124999999999994 displays
as 2.12. This is an explicit compatibility policy inferred from the observed
cutoff, not an assertion about TORA's conversion implementation or numerical
storage. Only display conversion is rounded; matrix/tableau values are retained.
The C++ conversion operation is specified in the
[standard character-conversion wording](https://eel.is/c++draft/charconv.to.chars).

The initial library precision envelope is decimal N=1…10, scientific N=0…10,
D=0…10. This is a safe
implementation bound, **not a claim that every setting or TORA's extrema have
been observed**. N=0 decimal format produced TORA Run-time error '5',
`Invalid procedure call or argument`; the core reports invalid settings safely.
Scientific N=0 is accepted: the mantissa retains a trailing decimal point
(e.g. `-3.e+00`). The `1e-100`/`1e100` scientific input produced Run-time error '6', `Overflow`.
Neither crash is a golden number string or a behavior to imitate by crashing.
The extreme inputs are finite binary64 values; the core formatter can represent
them. This observation does not define LP model acceptance limits, which must
be investigated at the model/input gate.
Output/input modules must establish their full acceptance and diagnostic
policy before enabling these fields in the GUI.

## Independently reviewed comparisons

`tests/fixtures/tora-number-format.tsv` contains original numeric transcriptions,
not screenshots or TORA problem files. Every row records notation, N/D, the
numerical value and the exact observed display. Decimal-sensitive coefficients
appear negated in the initial objective row; RHS inputs appear unchanged.
Partially clipped cell text is excluded from golden strings.

| Probe | Reviewed cells and findings |
| --- | --- |
| ties-exact | objective x1/x2: -2.125 → -2.13, -2.375 → -2.38; RHS 1.125/1.375/2.625 → 1.13/1.38/2.63 |
| ties-decimal | objective -2.675/-1.005 → -2.68/-1.01; RHS .005/.015/.025 → .01/.02/.03 |
| negative-near-zero | objective x1/x2 -0.0001/+0.0001 → -0.00/0.00 |
| decimal-width | N=1: objective -12345.125 → -12345.13, first RHS .125 → .13; clipped large values excluded |
| decimal-integer | D=0: objective -2.5/-3.5 → -3./-4.; RHS .5/1.5/4.5 → 1./2./5. |
| decimal-three | D=3: objective -1.0625/-1.1875 → -1.063/-1.188; RHS .0005/.0015/1.2345 → .001/.002/1.235 |
| scientific-n1-d2 | objective -1.25/-1.35 → -1.3e+00/-1.4e+00; RHS 9.95/.0125/12345 → 1.0e+01/1.3e-02/1.2e+04; zero 0.0e+00 |
| scientific-n2-d1 | objective -1.125/-1.375 → -1.13e+0/-1.38e+0; RHS 9.995/1e-12/1e25 → 1.00e+1/1.00e-12/1.00e+25 |
| decimal-high-precision | D=10: objective -1.23456789012345/-1.00000000005 → -1.2345678901/-1.0000000001 |
| Phase 0 scientific / decimal-three summaries | N=5/D=2 scientific and D=3 decimal values 5.5, 3.5, 1.5, 0.5; scientific -1 and zero; infinity bound |

The final-summary captures resolve previously clipped cells:

| Probe | Fully readable cells |
| --- | --- |
| signed-zero | x1 objective coefficient from source `-0.0` displays `0.00`; sensitivity uses `-infinity` |
| scientific-n5-d3 | RHS .0001/1e-12/1e25 → 1.00000e-004/1.00000e-012/1.00000e+025; coefficients 2.675/1.005 → 2.67500e+000/1.00500e+000; zero 0.00000e+000 |
| scientific-precision | N=10: coefficients 1.23456789012345/16777217 → 1.2345678901e+00/1.6777217000e+07; RHS 3 → 3.0000000000e+00 |
| decimal-high-precision | D=10: RHS .00000000005/1.00000000015 → .0000000001/1.0000000002, each with leading 0/1 respectively |
| scientific-n0 initial tableau | objective -2.5/-3.5 → -3.e+00/-4.e+00; RHS 9.5/.1/100 → 1.e+01/1.e-01/1.e+02; zero 0.e+00 |

The high-precision final summary preserves integer 16777217 and its objective
contribution 50331651 (3×16777217). This rules out rounding these particular
values to binary32 throughout that calculation; it does not identify the
implementation or establish every intermediate precision. Negative-zero source
normalization does not establish how every solver-generated signed zero is shown.

Three targeted cases isolated the significant-digit boundary:

| Probe | Reviewed initial objective/RHS cells |
| --- | --- |
| halfway-neighbours | -2.1249999999999996/-2.1250000000000004 → -2.13/-2.13; RHS 1.0049999999999997/1.0050000000000001/.12499999999999999 → 1.01/1.01/.13 |
| halfway-significant-boundary | -2.124999999999994/-2.124999999999995 → -2.12/-2.13; RHS 1.004999999999994/1.004999999999995/.1249999999999994 → 1.00/1.01/.12 |
| scientific-neighbours | -1.1249999999999998/-1.1250000000000002 → -1.13e+00/-1.13e+00; RHS 9.994999999999999/9.995000000000001/1.3749999999999998 → 1.00e+01/1.00e+01/1.38e+00 |

The first new neighbour fixture failed against the initial shortest-spelling
formatter (-2.12 instead of -2.13), then passed after the isolated display
precision correction. The more distant values remain below the rounding
boundary; a blanket epsilon adjustment would not describe these observations.

The normal-case golden arithmetic is independently reviewed in
`methods/lp/corpus/01_normal/all_slack/tableau-top.png` (initial/Iteration 2)
and `tableau-scroll-01.png` / `tableau-scroll-03.png` (Iteration 3). Tests request
row 2/x1 followed by row 1/x2 and compare all 24 cells of each resulting table,
plus basis/column ordering. This is a pivot-arithmetic comparison, not a solver
implementation or a claim that all Phase 0 solver traces now pass.

The computed-halfway case uses objective max x1, `.017*x1 <= .036125`,
x2 constrained to zero, and x1+x2 <= 3. TORA's automatic final output displays
objective/x1 `2.13`. The core's explicitly requested pivot retains binary64
`.036125/.017` (2.1249999999999996 on the tested platforms) and formats it as
`2.13`. This checks arithmetic-to-display integration as well as input cells;
it does not implement the automatic LP solver.

## Provenance

Paths below are relative to `reference/tora-captures/`. Foundation 2 table
sources have suffix `iteration-01.png`, unless otherwise stated. Screenshot
hashes identify the reviewed evidence without publishing its contents.

| Capture | SHA-256 |
| --- | --- |
| probes/foundation2-20261007/ties-exact/iteration-01.png | `6e71ca0bebb343df3468ebde48848bfbb4cc29e78ef1ce2551658ec97e319245` |
| probes/foundation2-20261007/ties-decimal/iteration-01.png | `7c0427850ba125cf767f10ed893e9dc2697c4df4440cc1cfbbf8e4f6728faf63` |
| probes/foundation2-20261007/negative-near-zero/iteration-01.png | `10ef5113ee5b2613686910ebd56e1096d7b6184cb91f2a0fec24c593f6dd3415` |
| probes/foundation2-20261007/decimal-width/iteration-01.png | `b8250e305cfda14bc22f5cecb6323b1cc199f5a79c41d5add5340f116ac8dfd4` |
| probes/foundation2-20261007/decimal-integer/iteration-01.png | `a9e31fb5eec5ed86cebb36f7ded60edff2be902d0a3215beb9450c8ab5eddaef` |
| probes/foundation2-20261007/decimal-three/iteration-01.png | `216896522a8d8048557009fc5e3438a6b70da1eeb3139c3190b455c2f35632bf` |
| probes/foundation2-20261007/scientific-n1-d2/iteration-01.png | `841bde47787f14b9d09b6fe6de43579fa5ed287df93e1784c5d71b35ed761711` |
| probes/foundation2-20261007/scientific-n2-d1/iteration-01.png | `e3d6cb86b24f3bfb70ac074744e096f8d9f82be20a56526541022102a3852354` |
| probes/foundation2-20261007/decimal-high-precision/iteration-01.png | `eb210343328da24a07694d2b88520941709674cdef6b3a892af12d4331ed4ccb` |
| methods/lp/corpus/01_normal/all_slack/tableau-top.png | `1da5e2b4f8151fad5d74a977014ea6f7bf23c02e8ca229f8b1d75555d8c99c75` |
| methods/lp/corpus/01_normal/all_slack/tableau-scroll-01.png | `9ad532eea091251f1e88b200222859903ede168fce046e4eb8e70ab246869ca1` |
| methods/lp/corpus/01_normal/all_slack/tableau-scroll-03.png | `acabdc3d3549562e5b8161dda4a0d4631043bd0c125572091868d28116df8a6d` |
| probes/closing-20261004/scientific/summary.png | `9a2e1c28cd3dc88274812e54dbdd5a662948f3a09eb7fc5ddb289a4005103329` |
| probes/closing-20261004/decimal-three/summary.png | `7796f290aa0826a9400da24a5e89eb1b0ca6500f221ab8c6511312fc30e28ba7` |
| probes/foundation2-20261007/signed-zero/summary.png | `8ac54674f4f391f974413112bbd95be759fddec1c86749ede7e58aa094eeddda` |
| probes/foundation2-20261007/scientific-n5-d3/summary.png | `15dd1f2043f2f764c9109aee0fab94b317ffdd692266355f640e8ce3264fc9fb` |
| probes/foundation2-20261007/scientific-precision/summary.png | `6a6a6ba924e391e0f6d616bf583314c198b78083af4c74720c4dae386760e092` |
| probes/foundation2-20261007/decimal-high-precision/summary.png | `9ce7afc3d6170a99cb566a6617e18b2e87b82e8075fc6c7081017c134707acd3` |
| probes/foundation2-20261007/scientific-n0/iteration-01.png | `596e1fbb30e5208963248e1f35769881ffad25d083eb8897c4b38832c421f3cc` |
| probes/foundation2-20261007/halfway-neighbours/iteration-01.png | `02d2dcea8a962e0239a4da85612fba0f4ae03d1d02180220989f95b928d6c18c` |
| probes/foundation2-20261007/halfway-significant-boundary/iteration-01.png | `b30d9f6a6995cb948682e8e7980924add17d4a3ca2b1146cc3f6c36bb12b769c` |
| probes/foundation2-20261007/scientific-neighbours/iteration-01.png | `a087497eaf2ebff753543590a313b81b60ff5e7557919f11eb45b8d5aa83950d` |
| probes/foundation2-20261007/computed-halfway/summary.png | `cc8d0711060968ec627c9b01d05a667c9bc92e4a51ad6cd189b5f296983464a6` |

## Unknowns / things I could not verify

TORA's internal precision, its full numeric range and intermediate arithmetic
order are not observable directly from UI captures. Binary64 and explicit
operation order are clone choices that pass the observations used here.
Scientific precision/exponent extrema and invalid-field handling are not fully
characterized. Any new displayed mismatch must be investigated with a targeted
input before the affected module gate. Clipped high-precision cells are not
silently reconstructed. Solver tie thresholds and cycling are still Q-01;
this foundation never chooses a pivot or declares a solution optimal.
