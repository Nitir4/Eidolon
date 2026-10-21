# Manual edge cases

Use fresh problems or copies. Variables are nonnegative unless stated otherwise.
Record actual behavior, not just the expected result. A documented unsupported
domain is distinct from a crash or an incorrect answer. Do not enter enormous
tables or wait indefinitely: record a suspected hang and the exact input.

| ID | Model / action | Expected mathematical result or validation |
|---|---|---|
| LP-E1 | Max x+y; x>=2, x<=1. | Infeasible. |
| LP-E2 | Max x+y; x-y<=1; no upper bounds. | Unbounded. |
| LP-E3 | Max x+y; x+y<=4 and 2x+2y<=8. | Objective4; any nonnegative pair summing4 is optimal. |
| LP-E4 | Multiply every constraint and RHS in lp-01 by1000. | Objective10 at(2,2); primal feasibility preserved. |
| CHANGE-E1 | lp-01: change objective from(3,2) to(1,1). | Objective4 with tied optima. |
| CHANGE-E2 | lp-01: change first RHS from4 to-1. | Infeasible; no stale baseline optimum. |
| CHANGE-E3 | lp-01: change first RHS to5; test RHS and Dual Revised. | Objective12 at(2,3). |
| TRANS-E1 | Costs[[1,4],[3,2]], supply[2,3], demand[1,1]. | Dummy demand3 with zero cost; optimum3. |
| TRANS-E2 | Costs all1, supply[2,2], demand[2,2]. | Objective4; many valid allocations. Manual loops must not corrupt totals. |
| TRANS-E3 | Add a zero-supply row to transport-01. | Same optimum21; no allocation from that row. |
| ASSIGN-E1 | Costs[[-4,3],[2,-1],[5,0]], minimize. | Objective-5; third worker unassigned. |
| ASSIGN-E2 | Same matrix, maximize. | Objective8: third worker/job1 and first worker/job2. |
| ASSIGN-E3 | 2x2 matrix of ones. | Objective2; either perfect pairing is valid. |
| INT-E1 | Max x+y; x+y<=2.5; both integer. | Objective2. |
| INT-E2 | Max x+2y; x+y<=2.5; only x integer. | Objective5 at(0,2.5). |
| INT-E3 | Start guided solving; stop with pending nodes, resize and export PDF. | Pending nodes remain unsolved; print does not advance the search. |
| EQ-E1 | x+y=2; 2x+2y=4. | Infinitely many solutions; no unique inverse. |
| EQ-E2 | x+y=2; 2x+2y=5. | Inconsistent. |
| EQ-E3 | Invert[[1,1],[2,2]]. | Singular; no misleading inverse. |
| MST-E1 | Four nodes; edges1-2 weight1 and3-4 weight2 only. | Disconnected forest of total3; not a connected spanning tree. |
| ROUTE-E1 | Directed edges1->2:-1,2->3:2,1->3:5. | Bellman-Ford distance1 from1 to3; Dijkstra must not silently accept a negative-edge model. Original support may differ. |
| ROUTE-E2 | Directed edges1->2:-2 and2->1:1. | Reachable negative cycle; shortest finite distances undefined. |
| FLOW-E1 | Directed source1/sink3; edges1->2:4,2->3:3,1->3:1. | Maximum flow4, min cut4; verify capacities/conservation. |
| PLAN-E1 | A:1->2 duration2; B:2->1 duration3. | Cycle rejected; no project schedule. |
| PLAN-E2 | A:1->2 duration2; B:1->3 duration2; C:2->4 duration3; D:3->4 duration3. | Duration5, both paths critical, all activity floats0. |
| PERT-E1 | One activity1->2 with a=1,m=2,b=3. | Mean2, variance1/9. |
| QUEUE-E1 | lambda=mu=1,c=1,K=N=infinity. | No stationary distribution. |
| QUEUE-E2 | lambda=2,mu=3,c=1,K=1,N=3. | p0=1/3,L=2/3,Lq=0,admitted rate2,W=1/3. Original attempted rate14/3 gives W=1/7. |
| QUEUE-E3 | Pure death lambda=0,mu=1,K=N=2,t=1. | Full mean3/e; original displayed partial mean1/e. |
| QUEUE-E4 | Pure birth lambda=2,mu=0,t=1. | Transient mean2; stationary waiting times do not apply. |
| GAME-E1 | 2x2 payoff matrix all2. | Value2; many optimal strategies, each probability vector sums1. |
| GAME-E2 | Payoffs[[3,1],[2,0]]. | Saddle point row1/column2; value1. |
| GAME-E3 | Payoffs all-2. | Value-2; negative payoff accepted. |
| INPUT-E1 | Enter abc, blank required fields, malformed infinity, or invalid dimensions. | Clear validation, no crash, current saved input preserved. Draft validation may occur at Solve. |
| INPUT-E2 | Paste a ragged TSV or beyond the editable grid. | Rejected safely; inspect every affected cell. |
| FILE-E1 | Cancel Open/Save As; open a missing/malformed file; try saving over a read-only source. | Current work preserved; read-only source not overwritten. |

Use the full baseline workflow list for normal cases. Add more case IDs whenever
you find a new failure; retain the smallest input that reproduces it.
