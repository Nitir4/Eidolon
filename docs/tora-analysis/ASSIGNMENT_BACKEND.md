# Assignment calculation backend

The Qt-free library src/assignment implements min/max assignment using a
shortest augmenting-path Hungarian variant. No Assignment frontend exists yet.
It accepts a finite cost matrix with1..100rows/columns and diagnoses arithmetic
overflow. Rows are workers, columns jobs, indices zero-based.

For a square minimization problem, choose binary x(i,j) minimizing
sum c(i,j)x(i,j), with each row and column sum1. Maximization negates costs.
Rectangular models are square-padded with zero-cost dummy workers/jobs,
matching exactly min(rows,columns) real pairs. Extra workers are explicitly
unassigned; extra jobs unused. Forbidden pairs and non-unit capacities are
outside this API.

Potentials u/v satisfy u(i)+v(j)<=normalized c(i,j). Matching tight edges yields
primal cost=sum(u)+sum(v), an optimality certificate. Every augmentation updates
potentials by the smallest uncovered reduced cost and follows predecessors to
an unused column. Complexity is O(n^3), memory O(n^2)including input. Ascending
columns break equal candidates deterministically; no unique-optimum claim.

Tests independently enumerate600seeded small min/max matrices, then check
one-to-one feasibility, all dual inequalities, complementary slackness and
primal/dual equality. Fractional/negative/rectangular cases,100x100diagonal,
invalid dimensions/sense/nonfinite costs and overflow are included. CHECK stays
active in Release. Input immutability and repeatability are checked.

Two actual TORA equivalent-model examples were saved and reviewed:

| Model | Matrix by rows | Real pairs (one-based) | Answer |
|---|---|---|---|
| Minimum cost | 9,2,7 / 6,4,3 / 5,8,1 | 1→2,2→1,3→3 | 9 |
| Maximum profit | 10,5,13 / 3,9,18 / 10,7,2 | 1→1,2→3,3→2 | 35 |

Unit supply/demand Transportation verifies minimum9.00. Negated profits verify
minimum-35.00; the sign is restored for Assignment. TORA's basic zero shipments
are excluded from real assignment pairs. Independent fixtures authenticate the
original file hashes before checking solver output.

Evidence:ev_f40dcab5f3a6b34a10867ed80af3c35b7809a2ec9fc81fd88120d984b8f26dc0
and ev_1c81ef43742d11b92dd156780f949bf6ab8e05bb7184d1b312dc239211bc1a55,
both producer exit0. Private captures are under application-inventory/actions-assignment-*
inside reference/tora-captures/. Observed golden data is in tests/fixtures/assignment-examples.json.

Final local suites:108/108desktop in95.82s;42/42Qt-free in4.25s.
Release Assignment checks1/1in0.01s. Packaging/three-OS validation pending.
The first hand-calculated profit expectation33was wrong: exhaustive enumeration
showed35; corrected before accepting implementation. Failed log preserved.

Primary mathematical reference:[Kuhn,The Hungarian Method for the Assignment Problem(1955)](https://www.math.utoronto.ca/mccann/1855/KuhnNRL55.pdf).
The implementation is a modern augmenting-path variant, not a reproduction of
that paper's flowchart or a recovered TORA Assignment screen.
