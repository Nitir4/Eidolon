# Revised dual backend contract

Class extension, not an observed TORA menu method. Standard form is
minimize c^T x, A x=b, x>=0, with a supplied nonsingular dual-feasible basis.
A has 1..100 rows and at most 200 columns. No implicit primal/Phase-I fallback.
The general LP adapter and explicit baseline bootstrap are described below; native integration remains outstanding.

At each iteration solve B xB=b and B^T y=cB, then price r=c-A^T y.
Choose the negative basic variable with smallest column index (Bland rule).
Solve B^T w=e_p. The pivot row is w^T A. Among negative nonbasic
row entries choose minimal r_j/(-w^T A_j), breaking exact ties by column.
Replace the leaving basis column and refactor on the next iteration.
No full tableau is maintained or pivoted. Dense partial-pivot elimination is
used afresh: intentionally simple for teaching-scale problems, not a sparse
production optimizer. Default absolute tolerance 1e-9; badly scaled/singular
systems, nonfinite arithmetic and loss of dual feasibility are errors.
An iteration cap returns Limit with the current trace, never a false optimum.

Optimal certificate: x>=0, Ax=b, A^T y<=c, c^T x=b^T y.
Infeasible certificate: w^T A>=0 and w^T b<0. Numerical tolerance applies;
this is floating-point evidence rather than a formal exact-arithmetic proof.
Trace retains basis, xB, y, reduced costs and selected pivot at each step.

Reference: K. Cheung, Carleton University, Revised dual simplex method:
https://people.math.carleton.ca/~kcheung/math/notes/MATH5801/06/6_3_revised_dual_simplex.html

Validation implemented: hand-worked optimal/infeasible examples, supplied nonidentity
basis, invalid bases, cap behavior, seeded covering LPs independently enumerated
in two dimensions, and comparison with existing observed Dual LP examples.
Current verification: full desktop 117/117 in 104.40s; headless 51/51 in 5.52s;
local Release revised-dual 3/3 in 0.22s. Standard backend: 300 seeded models
independently enumerated and compared with DualSession; adapter: 150 independent
bounded RHS changes plus bounds/free/equality/certificate regressions.
Two new REA-captured original outcomes compare 16 display fields. Older captured
10_dual/du-decimal also supply objective expectations 10/3.5. Native integration,
rank reduction and broader numerical/original profiles remain outstanding.

## General LP adapter and RHS reoptimization

`dual_revised_model.hpp/.cpp` converts a validated LP into stable standard form.
Lower shifts use x=L+z, free variables use x=zPlus-zMinus, finite upper bounds
become rows, and >= rows are negated. Equalities receive no artificial columns
in the returned standard form. Canonical model limit:100 nonnegative decision
variables/100 rows including upper bounds; at most200 total standard columns.
Unrestricted variables ignore their bound cells, consistently with the existing
model convention. Shifted objective constants and min/max signs are restored.

`solveDualRevisedModel` accepts explicit basis indices in the returned standard
form. Empty basis means inequality slacks; that basis must be dual feasible.
`prepareDualRevisedBasis` is an explicitly named bootstrap using the existing
primal/two-phase final solver on the nonnegative baseline. It maps real basis
columns, removes zero artificial basics when a real pivot exists, and certifies
the mapped basis with revised systems. Rank-dependent equality rows that cannot
be retained without artificials are rejected; no row-dropping policy is invented.

`reoptimizeDualRevisedRhs` bootstraps only the unchanged baseline, then uses its
basis directly for the changed RHS. No changed-model fresh primal solve occurs.
RHS sign changes never change A, slack column identities or variable mappings.
Changing Cj can invalidate dual feasibility: callers using a supplied basis get
an explicit error; no automatic Cj fallback is claimed. Infeasible/Limit results
have no restored optimum/values. Baseline infeasible/unbounded models are rejected.

## REA findings ledger — 16 October 2026

- **Observed:** restored dedicated original TORA window on privateXvfb96.
  REA `ev_63e5c57ea76f55bfee70d51ab3e947f49799c2e4b3abc42b98b84cbeb284c6d7`.
- **Observed:** max3x+2y with x+y<=6,x<=2,y<=3 has original final value12,
  x2,y3, contributions6/6,gaps1/0/0. REA
  `ev_a58891b8e2a07b510ccad38ff6b7c8084e3af6ef90a2036b733a0a657b47462a`.
- **Observed:** min2x+3y with x+y>=7,x+2y>=6,2x+y>=5 has original final
  value14,x7,y0,contributions14/0,gaps0/1/9. REA
  `ev_2796dda8dc246f91df3033e2e07fe18cd3830f1c5eccbedb2c2d3e99830c41e4`.
- **Inference/design:** these equivalent LP outcomes validate the revised method's
  answers; they do not establish an original revised-dual menu or implementation.
- **Bounded coverage:** two new original outcomes,16display fields; broader original
  formats and general bound/free/equality profiles are not claimed from these captures.
  Mathematical profiles also have separate independent/certificate regression tests.
- **Unknown/outstanding:** native integration/printing, cross-platform packaging,
  rank reduction for dependent equalities and wider numerical scaling profiles.

Private raw captures and REA records:reference/tora-captures/dual-revised/.
Public fixtures retain reviewed mathematical inputs/results and SHA256 provenance.
No new deep decompilation/import or swap was needed. CLI doctor reported missing
MCP registration; existing REA CLI supplied actual capture-process Evidence.

Final original MAIN verified after dismissing its popup: REA
`ev_8d889c9bfd5bb28ba50cf902e775de8c05253c2a5eff3cdba1a2cab04afff947`.
An earlier Transportation-helper return action left the summary unchanged;
its Evidence is not accepted as a MAIN transition. Corrected LP MAIN coordinates
were used, and both popup MAIN and clear MAIN captures were visually reviewed.


## Native integration update — 16 October 2026

The local native workflow now provides SOLVE Menu -> Class extensions -> Dual
Revised (RHS), with fixed Cj, draft RHS, restored result, selected canonical trace,
pivot feedback/infeasibility witness, explicit use-as-input and selected-step printing.
See LP_CHANGES_NATIVE.md for121desktop/51headless/four-scaleRelease/actualWayland gates
and limitations. The numerical/dependent-equality limits above still apply.
This is a new class-extension screen; no original TORA Revised menu is inferred.
Cross-platform packages and physical-printer acceptance remain separate work.
