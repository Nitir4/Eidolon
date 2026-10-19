# Network models backend

`src/networks` provides Qt-free Minimum Spanning Tree, Shortest Route and
Maximal Flow calculations. These are the three observed original Network
models menu entries. Numerical algorithms are independently implemented;
native Qt workflows now use the observed original7001/0/7004 file formats.
REA numerical examples corroborate MST, Dijkstra, Floyd and maximum flow.
Iteration-order parity is bounded to the captured models; independent
certificates and generated regression oracles cover the new calculations.

The bounded graph profile uses 1–100 nodes and at most 10,000 explicit edges.
Each edge has zero-based endpoints and one finite binary64 value. Absent
edges are absent records, not infinite values. Parallel edges retain their
input indices. Undirected edges automatically supply both orientations for
routing. The 100-node bound is an engineering profile; an original upper
limit has not yet been measured.

## Methods and guarantees

Minimum spanning forest requires an undirected graph. Kruskal's stable
weight ordering chooses edges through disjoint sets. Ties retain input order.
A disconnected graph returns its minimum forest, components and connected
status. Self-loops cannot enter a tree. Negative weights are valid. A final
cycle-property check confirms that no input edge is cheaper than the maximum
edge on its unique forest path. Complexity is O(E log E), plus the bounded
O(EV) certificate traversal.

Shortest routes use Bellman–Ford, including negative edges. Every target has
one of three explicit states: finite distance/path, unreachable, or reachable
from a negative cycle and hence unbounded below. Unreachable negative cycles
do not invalidate finite routes. A reachable negative-cycle witness includes
edge indices and a closed node sequence. Finite routes carry predecessor
paths; total path length and feasible-potential edge inequalities are checked.
Directedness is explicit. An undirected negative edge induces a negative
closed walk, correctly affecting all nodes reachable from it. Complexity is
O(VE), with O(V+E) extra storage and returned-path storage O(V²).

Maximum flow requires a directed graph, distinct source/sink and nonnegative
capacities. Edmonds–Karp selects shortest residual augmenting paths by BFS.
Each input arc gets its own reverse residual arc, preserving parallel and
antiparallel flows. Self-loop flows are zero. Results include every input arc's
flow and the reachable side of a minimum cut. Capacity bounds, conservation,
source/sink value and cut equality are checked before returning. Optional
trace records actual paths, forward/backward arc use and augmentation amounts.
The default 100,000 augmentation limit throws rather than returning partial
flow as optimal. Trace storage is separately bounded to 10,000 steps and
reports truncation. Algorithmic complexity is O(VE²), excluding trace storage.

Wide floating arithmetic reduces overflow in sums; returned scalar overflow
throws `std::overflow_error`. Invalid dimensions, endpoints and data throw
`std::invalid_argument`; exhausted limits or certificate failure throw
`std::runtime_error`. This is a floating implementation, not exact arithmetic:
long-double precision is platform-dependent, and contributions very small
relative to a large accumulated value may be lost. Flow certificate checks
use relative 1e-12 tolerances on local magnitudes. Ordering and cycle detection
use strict comparisons; near-zero cycle classification may reflect numerical
rounding. No universal original-input compatibility or exact-decimal promise
is made.

## Verification

`tests/networks.cpp` independently enumerates cuts for 300 seeded small flow
graphs, compares shortest routes with Floyd–Warshall including negative-cycle
reachability, and exhaustively enumerates tree subsets for 200 seeded graphs.
Tests replay forward and reverse residual augmentations, including a matching
example that requires cancellation. They cover disconnected/isolated nodes,
parallel/antiparallel arcs, negative edges/cycles, unreachable cycles, trace
truncation, exhausted limits, 100-node chains, tiny/huge capacities, explicit
output overflow and rejected input. A standalone optimized C++17 build and
execution passed; an initial test incorrectly assumed random graphs would
exercise cancellation and was replaced with an explicit forcing example.
Integrated CMake/CTest registration remains with the parent workstream.

Primary references consulted:

- [MIT 6.046J, Minimum Spanning Tree](https://www.ocw.mit.edu/courses/6-046j-design-and-analysis-of-algorithms-spring-2015/resources/lecture-12-greedy-algorithms-minimum-spanning-tree/).
- [MIT 6.006, Bellman–Ford](https://www.ocw.mit.edu/courses/6-006-introduction-to-algorithms-spring-2020/resources/lecture-12-bellman-ford/).
- [MIT 6.046J, Network flow and Edmonds–Karp](https://live.ocw.mit.edu/courses/6-046j-design-and-analysis-of-algorithms-spring-2015/bdafe05f4f3c20cabfd08d2fbd099f63_MIT6_046JS15_lec14A.pdf).

## Native integration and original text records (2026-10-18)

`networks_document`, `networks_controller`, and `networks_screens` implement the
three Network models in Qt Widgets. Inputs include title, node count, custom
names, directional cost/capacity matrix, symmetry mirroring, starting node and
source/sink. The result contains calculated edges, complete shortest routes,
or per-arc flows and a minimum-cut certificate. Node insertion/deletion has
one-level undo; files support atomic Save As, cancellation and a read-only
source guard. Invalid visible cells block solve/save/navigation rather than
silently reusing the previous valid model.

The original saved format is a marker, N/D/notation, quoted title, node count,
full quoted `(count+1)²` matrix and symmetry flag. Original observed markers
are **7001** for MST, **0** for Shortest Route, and **7004** for Maximal Flow.
The marker-0 decoder requires an explicit Shortest Route selection. Untouched
files retain the complete original bytes; modified files write quoted CRLF
records, retaining unmodified cell lexemes. Embedded quotes are doubled.
Files and model sizes have explicit 4 MiB and 100-node limits.

The original format observations are retained in
`reference/tora-captures/full-inventory/actions-network-{mst,shortest,flow}-save`.
The original MST output capture `ev_7849828a6982ad65535c05f6a1a9b8ac8f98923cb64b5b7db0a3c081ccdeee49`
shows a starting-node selector and Next/All Iterations. Prim's edge order is
therefore used in the clone, and its final total is independently checked
against the Kruskal certificate. The next capture
`ev_a366f141742c38c9a242f312c55e31ad1f2a0b05e499cdb0e35893fa54ff77a7`
observed an optimum modal and updated tree length 3.00 for the three-node
1/2/3-cost triangle. Its subsequent attempt to open a different model failed
at a UI control; that failure does not invalidate the already captured MST
image but does not establish shortest-route behavior.

Next/All calculations show actual Prim selections, bounded Bellman–Ford edge
relaxations, and Edmonds–Karp augmentations with explicit reverse cancellation.
These are independent implementations: identical original iteration order is
not claimed for shortest routes or flow. Each algorithm's final result is
certified; a trace storage limit is distinguished from calculation completion.
Printing wraps Unicode and unbroken names through QTextLayout and includes
calculation steps on subsequent pages as necessary.

`tests/desktop_networks.cpp` covers all three original-format structures,
byte-preserving round trips, explicit marker-0 context, quote escaping,
mirroring, node edits/undo, min-tree length3, shortest distance5/pathA-B-C,
flow5/min-cut5, disconnected/negative-cycle classifications, read-only files,
cancelled saves, failed opens, invalid active input, keyboard commits,
iteration controls and long-title PDF text. Integrated execution is pending
shared build registration. The standalone command below passed again after
adding bounded relaxation trace storage:

```sh
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic \
  -Isrc/core/include -Isrc/networks/include \
  src/networks/networks.cpp tests/networks.cpp -o /tmp/tora-network-tests
/tmp/tora-network-tests
```

The retained original input image from `ev_bd4c500fa35ffd6167d8a1bfe0d72e793651188ddc3ca5ef87732d22a1d8c9b0`
explicitly instructs heading selection for Delete/Insert/Copy/Paste and
single-click insertion after versus double-click insertion before. Native
EditGrid now exposes these operations and keyboard Copy/Paste/Undo shortcuts.
Paste is validated on a copy before changing the model, respects mirrored
costs, and skips the reserved diagonal. Node edits resize both matrix axes;
clipboard data is discarded after a dimension change. The exact original
behavior when pasting a row onto a different diagonal remains unmeasured;
this safe reservation rule is an implementation decision.

## Dijkstra and Floyd requirement confirmed by REA

The 2026-10-18 revisit discovered the original Shortest Route submenu:
`Shortest routes`, and `Iterations -> Dijkstra's algorithm / Floyd's algorithm`.
This is observed in `ev_cba1c10905a8b1c15d59a66e1f5d846780c3fbc693dd6f99c19280cdba5b837d`.
The earlier Bellman–Ford-only native route implementation was incomplete for
this workflow. It has been extended with independently implemented Dijkstra
and Floyd–Warshall backends and native method selection.

Dijkstra records each settled node, current distance/predecessor labels and
permanent/temporary flags. It rejects negative costs with a message pointing
to Floyd or the Bellman–Ford extension. The original three-node test has
arcs A->B2, A->C8, B->C3. REA observed Dijkstra's first iteration labels
0/2/8 (`ev_81b6b832f4577186118952a1010791373fd253ee3b32a13db3dc3d87295762bc`),
then 0/2/5 and all nodes permanent
(`ev_e1f9bb52931db5515a91168e62c82b66bb8e5885f9c34fc1a9e6db5387679431`).
The final route view shows distance5 and path1-2-3
(`ev_bae635c26072ed510dd230e113e9c8a5385d2bbd5d141607dc8bb072ef5f1d4a`).
The clone gives these same distances and predecessor path.

Floyd computes all pairs, records initial and each intermediate-node distance
and next-node matrix, and reconstructs finite paths. The original starts at
D0/S0 and the original D2 improves A->C to5 using B. These matrix views are
observed in `ev_bae635c26072ed510dd230e113e9c8a5385d2bbd5d141607dc8bb072ef5f1d4a`
and `ev_838c33ea534c982fab28455fe54b4b562a7b303cfd4ca592fb6a50e0cb16df18`.
The clone's next-node matrix explicitly uses absent markers for unreachable
pairs; the original initializes S0 entries even for unreachable pairs.
Diagonal next-node placeholders therefore differ while the reachable routes
and distance matrices agree on this observed model. Negative-cycle pairs
return explicit unbounded status rather than a misleading finite route.

All-pairs final routes use Floyd without storing matrices. Dijkstra and Floyd
results receive an independent Bellman–Ford distance/classification check,
while their displayed paths and traces come from the selected method.
The standalone regression run passed with 300 seeded nonnegative Dijkstra
models, 500 seeded directed/undirected Floyd models, independent route/cycle
oracles, per-pivot matrix replay and permanent-label checks. Existing flow,
spanning-tree and input-boundary checks also passed. Native tests cover method
selection results and original three-node Dijkstra/Floyd examples; integrated
execution is pending the parent's current shared build.

Original Maximal Flow's output selector lists Maximum flows / Iterations.
The original three-node capacities A->B3, A->C2, B->C4 produce total5 with
arc flows3/2/3, observed in
`ev_b0a55bf9d6460989c6df2f4c9232a3fb4b56b817f059c65805c671d95477a43e`.
The clone returns the same total and per-arc flows, plus a minimum cut of5.
A later keyboard-only attempt to select Iterations did not change the output
view; no original augmenting-path order is inferred from that attempt.

A clean original MST revisit (`ev_c58a9e40689c5dba82851e32af09d2b8267ded7197f440b2f9357b8f12e110e1`) captured the final table without a modal: A-B1, B-C2 and total3.00. Its final MAIN image verifies the reference was released for the next workstream.
