# REA continuation findings — 18 October 2026

This continuation reuses the installed original `.tora-installed/tora.exe`, owned
Wine prefix and dedicated X96 display. REA CLI `capture-process` retains each
bounded GUI driver call and its screenshots. It does not establish internal
algorithms or exhaustive parity. No new disassembly session was opened.
Raw screenshots, inputs and captures remain private under `reference/`; the
scenario/evidence ledger is `build/continuation-rea/`.

## Nonzero equations and inversion

Original marker2000 accepted `2x + y = 5`, `x - y = 1`. Its output shows solution
`(2,1)`, determinant `-3.00`, and inverse
`[[0.33,0.33],[0.33,-0.67]]` at two decimal places.
Evidence: `ev_6078bb7c6b4e0b06bf06127ba7042fc6f731ff37f88f0297ea3c54294e289730`.
The native unique square-system result now includes its associated inverse and
determinant. This uses a separate nontracing calculation; the equation teaching
trace remains distinct from the explicit inversion trace. An unrepresentable
optional inverse does not discard a valid equation solution.

Original marker1000 separately accepted `[[4,7],[2,6]]` and returned
`[[0.60,-0.70],[-0.20,0.40]]`.
Evidence: `ev_b22663a0f21ffffddafd10b79586379e21b9a37aa8df213ea5493ca9ffe66867`.
Its screen caption says “MATRIX INVERSION BY LU DECOMPOSITION”; that caption
is an observation, not a recovered implementation. Its determinant value is
clipped in this screenshot, so determinant10 is a mathematical calculation,
not a transcribed original value. Original Next/All Iterations are disabled
in these final views. Native Gauss–Jordan teaching traces are independently
checked and do not claim original LU iteration parity.

## Pure-death population cap

Original lambda0, mu3, c1 comparison output:

| Input source/system N | Original displayed Ls |
| --- | --- |
| 3 | 0.52276 |
| 30 | 25.50638 |
| 100 | 92.02124 |
| 9999 | 472.10614 |
| infinity (label9999) | 472.10614 |

Evidence: `ev_70f0aca672e0cca0913dc05e293de07d84f095c58cf78996fce55274ad42c154`.
The final probability rows for the infinity case are491..500, ending with
`p500=0.04979`, CDF1, despite the headingN9999.
Evidence: `ev_02d1e607f742e4060a1d6164cbe4e32e98304487d6cbb39e33240b621931d660`.

A boundary experiment compares mu1 atN500/501/infinity (all Ls315.05973) and
mu3 atN500/501/1000 (all Ls472.10614). The N501 output ends at500, where
mu1 gives `p499=p500=0.36788`, CDF1.
Evidence: `ev_5437a6ca4f4f8b2b5f32efa8e59c4ff45be00405d75044a0da2f9271dcfa5269`.
These observations support an effective `min(N,500)` original calculation
profile across the tested boundary. They are not a proof for every rate/N.

For constant-rate death at unit time, write `X=max(M-Poisson(mu),0)`. AtM500,
mu3, the correct full expectation is497; the partial moment excluding stateM
is `497-500*exp(-3)=472.106465816...`. Atmu1, it is
`499-500/e=315.060279414...`. Thus the large discrepancy is explained by an
effective500 cap plus exclusion of its terminal state. Small remaining
differences from original472.10614/315.05973 are legacy numerical discrepancies;
the original arithmetic causing them has not been recovered.

The native mathematical result continues to use the requested finite population
(or explicit9999 reference interpretation for infinity). The separate original
compatibility summary makes the capped population and partial moment explicit;
it does not replace the requested-model probability table or claim exact legacy
rounding parity.

## Printing and bounded failures

Original Equations Write to Printer produces Run-time error482, “Printer error”,
in this environment. Evidence:
`ev_8a5bb3e752f0f89fd2a954afa51dd51112bb0d0ce333b0ba8d08a7513899f90b`.
After dismissal the target exited. No original print-section/page-layout output
was obtained, and no physical-printer acceptance is claimed.

An earlier center-popup inverse navigation also exited the target without a
numeric output or diagnostic. Its cause remains unknown. Later explicit top
SolveModify → Solve Problem navigation succeeded; that changed path was used
for the numerical results above. The initial runtime attachment failure was
repaired by using the established named-desktop helper. Only owned resources
were recovered after confirming target absence; no unrelated Wine was stopped.

## Remaining boundaries

Broader tie/number/search-order profiles and original print layouts remain
bounded evidence gaps. Original PERT's previously captured VB9 fault has not
been retried unchanged. Windows/macOS packages and physical printer/DPI checks
require their respective environments; no GitHub Actions execution is authorized.
Local Release, package and report evidence is recorded separately from these
original-runtime observations.
