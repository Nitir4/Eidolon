# Practice problems for the comparison session

Every listed workflow has at least five exercises. These are newly prepared
practice problems with independent mathematical answer keys. **Original TORA
comparison is pending.** A shared exercise is deliberately reused across methods
so we can compare their final answers and intermediate behaviour on the same data.

Open a case's `input.txt` through its module's Open Existing File action; Assignment
uses `input.assignment.json`. Read `answer.json` for the problem, expected answer
and any method settings. `grid.tsv` can be copied from a text editor or spreadsheet.
For LP, paste only editable blocks (objective coefficients, constraint rows, bounds)
to avoid the protected blanks. Planning hides unused CPM/PERT columns; paste each
visible block separately. Network files include names and diagonal placeholders;
open them as files, or paste editable off-diagonal blocks only.

Use `comparison.csv` to record both applications' results, constraints/flows and
notes. Exact fractions are included where useful. Compare objective and feasibility
before variable/pairing identity: equally optimal solutions and tie order can differ.
For Transportation compare the initial heuristic separately, then the final optimum.
Cj/RHS extensions compare to a manually changed copy re-solved in original TORA.
Assignment uses the unit-supply/demand Transportation equivalent in original TORA.
The negative-edge Bellman-Ford extension uses positive graphs here for a common baseline.

Answer keys use exact rational 2D vertex enumeration, bounded integer enumeration,
Gaussian elimination, exhaustive assignments/transportation allocations/spanning
trees/cuts/simple paths, DAG path lengths, birth-death balance with analytic tails,
and 2x2 mixed-game formulas. No recorded original result is invented. Decimal
outputs should be compared at the same notation/precision (files use four places).
Pure-death means are specified to displayed precision; its boundary is absorbing zero.
The generator is `tools/generate_practice_bank.py` and uses only Python's standard library.

These are baseline problems. We will separately test invalid input, infeasibility,
unboundedness, degeneracy/ties, singular matrices, disconnected/negative-cycle
networks, cyclic planning, unstable queues, numerical extremes and large tables
once the UX work is accepted. Native Windows/macOS and physical mixed-DPI checks
remain separate from this comparison session.

| Workflow | Exercises |
|---|---|
| LP / Graphical | [lp-01](lp-01/README.md), [lp-02](lp-02/README.md), [lp-03](lp-03/README.md), [lp-04](lp-04/README.md), [lp-05](lp-05/README.md) |
| LP / All-slack | [lp-01](lp-01/README.md), [lp-02](lp-02/README.md), [lp-03](lp-03/README.md), [lp-04](lp-04/README.md), [lp-05](lp-05/README.md) |
| LP / Final solution | [lp-01](lp-01/README.md), [lp-02](lp-02/README.md), [lp-03](lp-03/README.md), [lp-04](lp-04/README.md), [lp-05](lp-05/README.md) |
| LP / Sensitivity | [lp-01](lp-01/README.md), [lp-02](lp-02/README.md), [lp-03](lp-03/README.md), [lp-04](lp-04/README.md), [lp-05](lp-05/README.md) |
| LP / Bounded variables | [bounded-01](bounded-01/README.md), [bounded-02](bounded-02/README.md), [bounded-03](bounded-03/README.md), [bounded-04](bounded-04/README.md), [bounded-05](bounded-05/README.md) |
| LP / Big-M | [cover-01](cover-01/README.md), [cover-02](cover-02/README.md), [cover-03](cover-03/README.md), [cover-04](cover-04/README.md), [cover-05](cover-05/README.md) |
| LP / Two-phase | [cover-01](cover-01/README.md), [cover-02](cover-02/README.md), [cover-03](cover-03/README.md), [cover-04](cover-04/README.md), [cover-05](cover-05/README.md) |
| LP / Dual simplex | [cover-01](cover-01/README.md), [cover-02](cover-02/README.md), [cover-03](cover-03/README.md), [cover-04](cover-04/README.md), [cover-05](cover-05/README.md) |
| LP / Change Cj | [change-cj-01](change-cj-01/README.md), [change-cj-02](change-cj-02/README.md), [change-cj-03](change-cj-03/README.md), [change-cj-04](change-cj-04/README.md), [change-cj-05](change-cj-05/README.md) |
| LP / Change RHS | [change-rhs-01](change-rhs-01/README.md), [change-rhs-02](change-rhs-02/README.md), [change-rhs-03](change-rhs-03/README.md), [change-rhs-04](change-rhs-04/README.md), [change-rhs-05](change-rhs-05/README.md) |
| LP / Dual Revised (RHS) | [change-rhs-01](change-rhs-01/README.md), [change-rhs-02](change-rhs-02/README.md), [change-rhs-03](change-rhs-03/README.md), [change-rhs-04](change-rhs-04/README.md), [change-rhs-05](change-rhs-05/README.md) |
| Integer / Pure automated | [integer-pure-01](integer-pure-01/README.md), [integer-pure-02](integer-pure-02/README.md), [integer-pure-03](integer-pure-03/README.md), [integer-pure-04](integer-pure-04/README.md), [integer-pure-05](integer-pure-05/README.md) |
| Integer / Guided branch-and-bound | [integer-pure-01](integer-pure-01/README.md), [integer-pure-02](integer-pure-02/README.md), [integer-pure-03](integer-pure-03/README.md), [integer-pure-04](integer-pure-04/README.md), [integer-pure-05](integer-pure-05/README.md) |
| Integer / Mixed | [integer-mixed-01](integer-mixed-01/README.md), [integer-mixed-02](integer-mixed-02/README.md), [integer-mixed-03](integer-mixed-03/README.md), [integer-mixed-04](integer-mixed-04/README.md), [integer-mixed-05](integer-mixed-05/README.md) |
| Equations / Solve linear equations | [equations-01](equations-01/README.md), [equations-02](equations-02/README.md), [equations-03](equations-03/README.md), [equations-04](equations-04/README.md), [equations-05](equations-05/README.md) |
| Equations / Matrix inverse | [inverse-01](inverse-01/README.md), [inverse-02](inverse-02/README.md), [inverse-03](inverse-03/README.md), [inverse-04](inverse-04/README.md), [inverse-05](inverse-05/README.md) |
| Transportation / North-west | [transport-01](transport-01/README.md), [transport-02](transport-02/README.md), [transport-03](transport-03/README.md), [transport-04](transport-04/README.md), [transport-05](transport-05/README.md) |
| Transportation / Least cost | [transport-01](transport-01/README.md), [transport-02](transport-02/README.md), [transport-03](transport-03/README.md), [transport-04](transport-04/README.md), [transport-05](transport-05/README.md) |
| Transportation / Vogel | [transport-01](transport-01/README.md), [transport-02](transport-02/README.md), [transport-03](transport-03/README.md), [transport-04](transport-04/README.md), [transport-05](transport-05/README.md) |
| Transportation / MODI improvement | [transport-01](transport-01/README.md), [transport-02](transport-02/README.md), [transport-03](transport-03/README.md), [transport-04](transport-04/README.md), [transport-05](transport-05/README.md) |
| Transportation / Final solution | [transport-01](transport-01/README.md), [transport-02](transport-02/README.md), [transport-03](transport-03/README.md), [transport-04](transport-04/README.md), [transport-05](transport-05/README.md) |
| Assignment / min | [assignment-min-01](assignment-min-01/README.md), [assignment-min-02](assignment-min-02/README.md), [assignment-min-03](assignment-min-03/README.md), [assignment-min-04](assignment-min-04/README.md), [assignment-min-05](assignment-min-05/README.md) |
| Assignment / Rectangular | [assignment-min-03](assignment-min-03/README.md), [assignment-min-04](assignment-min-04/README.md), [assignment-max-03](assignment-max-03/README.md), [assignment-max-04](assignment-max-04/README.md), [assignment-rectangle-05](assignment-rectangle-05/README.md) |
| Assignment / max | [assignment-max-01](assignment-max-01/README.md), [assignment-max-02](assignment-max-02/README.md), [assignment-max-03](assignment-max-03/README.md), [assignment-max-04](assignment-max-04/README.md), [assignment-max-05](assignment-max-05/README.md) |
| Networks / Minimum spanning tree | [mst-01](mst-01/README.md), [mst-02](mst-02/README.md), [mst-03](mst-03/README.md), [mst-04](mst-04/README.md), [mst-05](mst-05/README.md) |
| Networks / Shortest route | [route-01](route-01/README.md), [route-02](route-02/README.md), [route-03](route-03/README.md), [route-04](route-04/README.md), [route-05](route-05/README.md) |
| Networks / Dijkstra | [route-01](route-01/README.md), [route-02](route-02/README.md), [route-03](route-03/README.md), [route-04](route-04/README.md), [route-05](route-05/README.md) |
| Networks / Floyd | [route-01](route-01/README.md), [route-02](route-02/README.md), [route-03](route-03/README.md), [route-04](route-04/README.md), [route-05](route-05/README.md) |
| Networks / Bellman-Ford extension | [route-01](route-01/README.md), [route-02](route-02/README.md), [route-03](route-03/README.md), [route-04](route-04/README.md), [route-05](route-05/README.md) |
| Networks / Maximal flow | [flow-01](flow-01/README.md), [flow-02](flow-02/README.md), [flow-03](flow-03/README.md), [flow-04](flow-04/README.md), [flow-05](flow-05/README.md) |
| Planning / CPM | [cpm-01](cpm-01/README.md), [cpm-02](cpm-02/README.md), [cpm-03](cpm-03/README.md), [cpm-04](cpm-04/README.md), [cpm-05](cpm-05/README.md) |
| Planning / Schedule bar chart | [cpm-01](cpm-01/README.md), [cpm-02](cpm-02/README.md), [cpm-03](cpm-03/README.md), [cpm-04](cpm-04/README.md), [cpm-05](cpm-05/README.md) |
| Planning / PERT | [pert-01](pert-01/README.md), [pert-02](pert-02/README.md), [pert-03](pert-03/README.md), [pert-04](pert-04/README.md), [pert-05](pert-05/README.md) |
| Queuing / M/M/1 | [queue-mm1-01](queue-mm1-01/README.md), [queue-mm1-02](queue-mm1-02/README.md), [queue-mm1-03](queue-mm1-03/README.md), [queue-mm1-04](queue-mm1-04/README.md), [queue-mm1-05](queue-mm1-05/README.md) |
| Queuing / M/M/c | [queue-mmc-01](queue-mmc-01/README.md), [queue-mmc-02](queue-mmc-02/README.md), [queue-mmc-03](queue-mmc-03/README.md), [queue-mmc-04](queue-mmc-04/README.md), [queue-mmc-05](queue-mmc-05/README.md) |
| Queuing / Finite capacity K | [queue-finite-k-01](queue-finite-k-01/README.md), [queue-finite-k-02](queue-finite-k-02/README.md), [queue-finite-k-03](queue-finite-k-03/README.md), [queue-finite-k-04](queue-finite-k-04/README.md), [queue-finite-k-05](queue-finite-k-05/README.md) |
| Queuing / Finite source N | [queue-finite-n-01](queue-finite-n-01/README.md), [queue-finite-n-02](queue-finite-n-02/README.md), [queue-finite-n-03](queue-finite-n-03/README.md), [queue-finite-n-04](queue-finite-n-04/README.md), [queue-finite-n-05](queue-finite-n-05/README.md) |
| Queuing / Finite K and N | [queue-finite-kn-01](queue-finite-kn-01/README.md), [queue-finite-kn-02](queue-finite-kn-02/README.md), [queue-finite-kn-03](queue-finite-kn-03/README.md), [queue-finite-kn-04](queue-finite-kn-04/README.md), [queue-finite-kn-05](queue-finite-kn-05/README.md) |
| Queuing / Pure birth | [queue-pure-birth-01](queue-pure-birth-01/README.md), [queue-pure-birth-02](queue-pure-birth-02/README.md), [queue-pure-birth-03](queue-pure-birth-03/README.md), [queue-pure-birth-04](queue-pure-birth-04/README.md), [queue-pure-birth-05](queue-pure-birth-05/README.md) |
| Queuing / Pure death | [queue-pure-death-01](queue-pure-death-01/README.md), [queue-pure-death-02](queue-pure-death-02/README.md), [queue-pure-death-03](queue-pure-death-03/README.md), [queue-pure-death-04](queue-pure-death-04/README.md), [queue-pure-death-05](queue-pure-death-05/README.md) |
| Games / LP-based | [game-01](game-01/README.md), [game-02](game-02/README.md), [game-03](game-03/README.md), [game-04](game-04/README.md), [game-05](game-05/README.md) |
| Games / Graphical | [game-01](game-01/README.md), [game-02](game-02/README.md), [game-03](game-03/README.md), [game-04](game-04/README.md), [game-05](game-05/README.md) |

126 distinct cases; 42 workflows; 210 comparison rows.
