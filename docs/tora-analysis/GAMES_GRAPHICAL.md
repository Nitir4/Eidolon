# Graphical Game calculation backend

`src/games/graphical.cpp` implements the envelope method independently of the
LP-based Game solver. Native Graphical controls, zoom calibration and printing
remain integration work. This document distinguishes original observations
from mathematical extensions and numerical design choices.

## Mathematical model and execution

For a 2-by-n payoff matrix to maximizing row player A, let p be the probability
of A1. Column j gives payoff f_j(p)=A(2,j)+(A(1,j)-A(2,j))p. Player A maximizes
the lower envelope min_j f_j(p), over 0<=p<=1. The `RowA` mode follows the
observed original convention. In `ColumnB` mode for m-by-2 matrices, q is the
probability of B1; player B minimizes the upper envelope of the row payoffs.
Column mode is a mathematical extension: Graphical was visibly grayed and
unselectable for the observed original3-by-2input; LP-based remained available.

Endpoints and pairwise line intersections partition the probability domain
into affine pieces. Evaluating a midpoint identifies the active strategy on
each piece; adjacent pieces using the same line are merged. An optimum occurs
at an endpoint or intersection. The result includes line endpoints, active
segments, chosen probability, optimal interval and both equilibrium mixtures.
Exact floating-point ties choose the smallest probability; original tie
selection and uniqueness are not claimed.

At an interior optimum the active slopes bracket zero. A horizontal active
line supplies a pure opposing strategy; otherwise two active lines supply a
mixture whose weighted slope is zero. At an endpoint use a line satisfying
the appropriate one-sided payoff inequality. The implementation constructs
valid probability vectors and checks every opposing pure payoff, independently of the
LP-based algorithm. These inequalities suffice for mixed strategies because
a mixed payoff is a convex combination of pure payoffs.

The calculation accepts exactly two strategies for the selected player and
1–100 for the opponent, with finite data. Scaled endpoints avoid overflow
when subtracting payoffs such as +/-1e308. Full-precision stored probabilities
are formatted only for display. Invalid dimensions/probabilities/data or a
failed equilibrium certificate throw exceptions. Tolerance defaults are
absolute1e-9/relative1e-10 in scaled payoff units; extremely unequal magnitudes
may lose distinctions. This is not an exact-arithmetic or absolute original-unit
accuracy guarantee. Exact arithmetic would require a different numeric model.
The bounded implementation uses O(k^3) work and O(k^2) temporary storage, k<=100.

## Original TORA verification

Actual original Graphical output automatically draws all payoff lines and the
lower envelope. It offers Zoom In, Zoom Out and Print Graph after the standard
Decimal/Scientific N/D selector. The left panel lists affine payoffs, value and
both mixtures. Negative slopes display a literal plus followed by a negative
number; fixtures compare their numeric coefficient fields.

| Matrix by rows | Value | A mixture displayed | B mixture displayed |
|---|---|---|---|
| 4,3 / 2,1 | 3.00 | 1.00,0.00 | 0.00,1.00 |
| 3,0 / 1,2 | 1.50 | 0.25,0.75 | 0.50,0.50 |
| 5,2,0 / 1,3,4 | 2.50 | 0.38,0.63 | 0.50,0.00,0.50 |

In the rectangular example the true stored mixture is(.375,.625).
Rounding each entry independently produces a displayed total1.01; do not
renormalize these strings or round stored calculations.

`tests/fixtures/games-graphical.json` transcribes three readable original
results, including every affine intercept/slope. Original9000saved files are
reconstructed and SHA256-authenticated before comparison. Thirty reviewed
fields are checked by `games.graphical.goldens`; the reader remains test-only.
Private screenshots/files stay excluded from the product.

Evidence, each producing capture exit0:

- Pure: `ev_514675da46a83a6a2f2da22c0cc6fa65daeed87a058fce58ef93b0d392da1771`.
- Asymmetric: `ev_a303eb106825301a02fc6505a747564dd4d6c8619e7722129430cd590b2931f5`.
- Rectangular: `ev_a669c27aefe66d397ae7dd1157076eb85b1b9ed356a99b10ee620c5a8365caec`.

Original3x2eligibility observations:
`ev_77831dfde7e4fea1a88db6157d696aa89b09a86ad1d34c32c788956a88685c55`
and `ev_48ae170ffd6a809f40d3122b2b4859eac798f003112d12381caea2d19c9ad1e3`.
The attached-menu helper returned no item flags for this popup; visual gray
state and the retained menu after selection are the evidence, not numeric flags.

An earlier negative-payoff2x3capture was black. It is excluded from numerical
goldens, and its cause is unresolved. The private runtime watcher formerly
stopped after one absent-window probe; it now requires five absences plus
actual absence of its owned target process. A readable nonnegative example
succeeded afterward; this does not isolate the black capture's cause.

## Verification and remaining work

Always-active tests cover300seeded2-by-n/m-by-2models against the separate LP
backend and full payoff certificates; segment coverage/dominance; endpoint,
flat-range, duplicate and parallel cases;100opposing strategies in either
orientation; huge/tiny payoffs and invalid input. Tests-first linking failed
on the missing implementation, then the first focused three checks passed.
Final headless suite48/48in4.09s; desktop114/114in107.94s;
focused Release8/8in0.46s, including both Graphical checks.
Logs:build/games/graphical-final-*.log and graphical-release-*.log.

Native Games input/files, Graphical plotting/zoom/printing acceptance, broader
eligibility/numeric profiles and package validation remain.
No new native decompilation/import or temporary swap was needed in this cycle.

Primary teaching reference: [CMU15-451, Recitation8, zero-sum game payoff
graph](https://www.cs.cmu.edu/~15451-f22/recis/reci8-kunal.pdf). The indexed
graphical example supports the worst-reply interpretation; implementation,
equilibrium certificate and tests here were derived independently.
