# LP output-format screen

## Purpose and selection

TORA displays this screen before graphical, final-summary, and iteration
output. It chooses how numerical output is presented. A method-selection
warning or M-value prompt can precede it.

## Inputs and defaults

The bordered group is titled `Select Output Format`. Radio labels are
`Decimal Notation (NNNNN.DD)` and `Scientific Notation (N.NNNNeDD)`. Fields
are labeled `How many N?` and `How many D?`; observed defaults are 5 and 2.
The command is `Go To Output Screen`. Lower buttons are `View/Modify Input
Data`, `MAIN Menu`, and `Exit TORA`. The default decimal radio is selected.

The input-mode notation choice is a separate screen and must not be confused
with model dimensions. Output format changes must not change solver arithmetic.

## Evidence and navigation

Examples: `reference/tora-captures/methods/lp/corpus/01_normal/all_slack/output-format.png`
and `reference/tora-captures/output-studies/01_normal/graphical/output-format.png`.
`View/Modify Input Data` returns to the LP grid. `Go To Output Screen` opens
the selected output view after calculation/preparation.

## Unknowns / things I could not verify

Exact N/D validation, rounding at ties, overflow width behavior, keyboard
default focus, and whether settings persist across unrelated documents still
require controlled probes. Decimal negative-zero strings are retained in the
iteration corpus; their underlying arithmetic cannot be inferred from the
formatted values alone.

## Scientific output and decimal precision probe

Case 13 was independently solved through Final solution with scientific output
and N=5/D=2. Objective 5.50 displays as `5.50000e+00`; x1/x2 display as
`3.50000e+00` and `1.50000e+00`. Coefficients are `2.00000e+00` and
`-1.00000e+00`; the final dual price 0.50 is `5.00000e-01`. Zero is
`0.00000e+00`, and open bounds stay `infinity`. This establishes lowercase e,
explicit exponent sign and two exponent digits for these values, and five
fractional mantissa digits for N=5. The constraint-slack suffix remains
separate: `4.00000e+00-`.

A second independent run used decimal N=5/D=3 and displays objective `5.500`,
values `3.500`/`1.500`, and dual price `0.500`. These settings change display,
while these observed solutions agree. Evidence:
`probes/closing-20261004/scientific/` and `decimal-three/`. Neither experiment
establishes rounding at halfway values or exponent overflow.

## Foundation 2 targeted observations (2026-10-07)

The focused [numerical evidence](NUMERIC_FOUNDATION.md) establishes
15-significant-digit display preparation followed by halfway rounding away
from zero, small negative values displayed as `-0.00`, trailing
points at D=0, exponent padding at D=1 and D=3, scientific N=0/1/2/5/10,
and decimal D=10. Eighty reviewed strings are portable golden fixtures.
Decimal N=0 caused Run-time error '5' (Invalid procedure call or argument);
scientific N=0 succeeds, for example `-3.e+00`. Extreme scientific input
1e-100/1e100 caused Run-time error '6' (Overflow). These findings narrow Q-01/Q-02;
full field extrema and all solver thresholds remain unverified. The core
rejects the observed invalid decimal setting safely. GUI acceptance/diagnostic
behavior remains an input/output module obligation.
