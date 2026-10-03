# M-method

## Purpose and selection

TORA presents the M-method as one of its iterative algebraic simplex choices. The observed flow asks for a penalty value M before the common output-format dialog; the screen itself does not state additional selection guidance.

## Direct observation

The method is selected from `Solve Problem → Algebraic → Iterations →
M-method`. TORA then asks `Enter value of M:`; the observed default is `100`.
It next shows the common output-format dialog (`Decimal Notation (NNNNN.DD)`
or `Scientific Notation (N.NNNNeDD)`, plus N and D fields) before opening the
tableau. Evidence: `reference/tora-captures/methods/lp/m_method/`.

The output is titled `SIMPLEX TABLEAU - (M-Method)`. The common instruction
text says to click a nonbasic variable to enter, click a basic variable to
leave, and use `NEXT ITERATION` or `ALL ITERATIONS`; the latter may execute
without choosing either or both pivots manually. The observed fixture was the
two-phase sample (minimize x1+x2, one equality and two ≥ constraints), with
M=100. Its first objective row was `199.00, 199.00, -100.00, -100.00, 0.00,
0.00, 0.00` for x1, x2, Sx3, Sx4, Rx5, Rx6, Rx7. The table labels the initial
state `Iteration 1`; `ALL ITERATIONS` reports `Iteration 4 is optimal` and
the alternative-solution notice. The exact shared notice, including TORA's
`soltuion(s)` spelling and `zj-cj=0`, is in `LP_PIVOT_POLICY.md`.

The captured `03_infeasible` M-method run ends with an `Optimal Tableau`
dialog saying `Iteration 3 is optimal`, despite the fixture title being
`Infeasible`. The tableau still contains an artificial variable with a
positive solution value. This is a direct observation of this run and must not
be generalized as correct infeasibility handling. The `04_unbounded` run ends
with an `Optimal Tableau` dialog saying `Last tableau shows solution is
unbounded`.

When M-method is selected for an all-`<=` model (`01_normal`), TORA shows a
`Warning` with OK and Cancel buttons: `Problem has all <= constraints. All-Slack option will be
used`. After acknowledging it, the output is the All-Slack tableau. The
warning and resulting tableau are both captured under
`reference/tora-captures/methods/lp/corpus/01_normal/m_method/`.

## Unknowns / things I could not verify

Alternate M acceptance and initial coefficients are captured in `LP_M_VALUE.md`;
these focused probes are not full terminal traces for every penalty.
All fifteen main M selections now have full numeric transcriptions, including
All-slack redirections. Shared pivot/termination observations and the repeated
positive-artificial stopping result are in `LP_PIVOT_POLICY.md`.
The finite-M objective construction fits the directly captured 100 penalty;
its reliability for arbitrary larger/smaller M has not been established.

## Shared input, UI and file contract

Input fields, relations, names, dimension limits and bounds/sign flags are
specified in `LP_INPUT.md`. Setup uses `LP_OUTPUT_FORMAT.md`; M alone adds
`LP_M_VALUE.md`. Ordered rows/columns, labels, number strings, highlighting,
scrolling and manual controls are specified in `LP_ITERATION_SCREEN.md`.
Observed automatic rules and exact validation/termination text are in
`LP_PIVOT_POLICY.md` and `LP_ALL_SLACK.md`. Model save/open behavior is shared
and specified in `FILE_BEHAVIOR.md` / `../FILE_FORMAT.md`. These shared
contracts apply to this method; unsupported edge cases stay explicit.
