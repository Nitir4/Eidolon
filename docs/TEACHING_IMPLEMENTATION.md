# Recorded Assignment and Games calculations

This is an implementation guide for worked solutions, not a claim that Eidolon
uses TORA's pivot path. The sensitivity repairs remain in `src/lp/sensitivity.cpp`.

## Assignment: shortest augmenting paths with dual potentials

Entry point: `assignment::solve` in `src/assignment/assignment.cpp`.
Public model/result/history: `src/assignment/include/tora/assignment/assignment.hpp`.

1. Validate finite costs, objective direction, dimensions (1–100), and forbidden
   indices. Forbidden edges are absent from the search; no penalty constant is used.
2. Transform maximization by negating profits. Pad a rectangular matrix to
   `n=max(workers,jobs)` with zero-cost dummy rows/columns. A solution contains
   exactly `min(workers,jobs)` pairs from the supplied matrix. Declared dummy
   metadata identifies supplied rows/columns; their entered costs remain data.
3. Initialize one-based potentials `u,v` and the column-to-row matching to zero.
   Column 0 is an augmenting-path sentinel, not a real job.
4. Process roots in ascending row order. Start a fresh distance array at infinity
   and an empty visited-column set. Visit a column, then scan unvisited columns
   in ascending order. For an admissible edge compute `c'(i,j)-u(i)-v(j)`.
   A strict improvement updates the distance and predecessor column.
5. Choose the smallest tentative distance `delta`. Strict comparisons retain
   the first column on ties. Update `u[matched[j]] += delta, v[j] -= delta` for
   visited columns; subtract delta from finite unvisited distances. Follow the
   selected column until an unmatched column is reached.
6. Reverse predecessor links to augment the matching. Continue with the next root.
   If all reachable distances are infinite, return **infeasible**, without an
   objective or a purported complete assignment in the UI.
7. After n augmentations, recover real pairs and sum their original costs. The
   transformed matched edges are tight and the complete primal/dual objectives
   agree. Before all roots are processed, zero initialization need not be dual
   feasible for unprocessed negative-cost rows.

Comparisons are exact double comparisons; there is no Hungarian epsilon.
Arithmetic overflow is an error. A finite edge of cost 1000 remains admissible.
Alternative optima are resolved deterministically, not enumerated.

Each actual initialization, conversion, root search, relaxation, potential update,
augmentation and terminal event is recorded in `Result::history`. Records retain
potentials, matching, distances, visited columns, predecessors and the selected
delta. Original and transformed matrices are retained separately. Reduced matrices
in the view are computed from those recorded potentials, not by solving again.
This algorithm does **not** perform a fabricated row-reduction/zero-cover sequence.

## Games: saddle test, then a positive-payoff LP

Entry point: `games::solve` in `src/games/games.cpp`.

- The row player A maximizes; column player B minimizes the entered payoff A.
- Compute each row minimum and column maximum. If maximin equals minimax
  exactly, enumerate saddles in row-major order and choose the first. The history
  contains that actual calculation; there are no simplex iterations for this path.
- Otherwise let `scale=max(abs(Aij))` and `H=A/scale+2`. The actual LP is
  `max sum(y), H*y<=1, y>=0`. Its dual is `min sum(x), H^T*x>=1, x>=0`.
  H lies in [1,3], so a slack basis with RHS 1 is feasible. There is one primal
  optimization phase and **no artificial-variable Phase I**.
- Tableau objective coefficients for y start at -1. `core::Tableau::pivot`
  (`src/core/tableau.cpp`) divides the leaving row by the pivot and eliminates
  the entering column from every other row, including the objective.
- Enter the first nonbasic column with a negative reduced cost (including
  slacks). Use the smallest nonnegative RHS/coefficient ratio among positive
  pivot coefficients. Tied ratios choose the smallest basic-column index.
- Default equilibrium tolerance: absolute `1e-9`, relative `1e-10`.
  Pivot/support tolerance is each component multiplied by `.001`.
  Stop when no eligible negative reduced cost remains. Default safety limit:
  1024 pivots. Limits, arithmetic failures and invalid certificates raise errors;
  they are not reported as optima or supplied with invented completed histories.
- Read column weights y from basic values and row weights x from objective-row
  slack prices. Normalize each vector separately. In exact arithmetic,
  `v=scale*(1/sum(y)-2)`.
- `reconstructSupport` improves cancellation-prone probabilities using the
  positive LP support when both support sizes agree. `supportSolution` solves
  sum=1 and indifference equations by partial-pivot Gauss–Jordan elimination.
  Choose the largest absolute pivot, first on ties. Singular/negative candidates
  leave the corresponding LP probabilities unchanged. Augmented systems and
  each actual elimination update are recorded.
- Normalize again; compute `min_j p^T*A[:,j]` and `max_i A[i,:]*q` in scaled
  arithmetic. Require agreement within equilibrium tolerance. Restore units and
  report their midpoint as the original value, with both guarantees and the gap.

The history contains the formulation, exact initial tableau, every pivot,
LP termination, raw weights, normalization, support systems/updates, and final
strategies/certificate. It does not expose the separate `rowPlayerLp` teaching
formulation as if that were the tableau actually used.

## Graphical Games

`games::solveGraphical` in `src/games/graphical.cpp` retains its existing envelope
calculation. It evaluates endpoints and all finite interior intersections of
normalized payoff lines, then maximizes the lower envelope (2×n) or minimizes
the upper envelope (m×2 backend). Exact ties retain the earliest breakpoint.
Active slopes determine the opposing mix. Both payoff guarantees are checked.
The recorded history contains real breakpoints, candidate evaluations and
recovery, rather than simplex pivots. The existing GUI graph remains the row
player's 2×n route; m×2 remains available through LP solving in the GUI.

## Presentation and output

`src/desktop/calculation_history.cpp` formats saved records.
`HistoryView` in `src/desktop/history_view.cpp` supplies the keyboard-accessible
selector, Previous/Next/All, UTF-8 TXT export and complete-history printing.
Assignment keeps its Assignment and Dual certificate tabs. Games keeps its
solution/graph summary. Both add **Steps / Iterations** on their result page.
The existing result Print commands also include the complete recorded history.
Viewing, navigation, export and printing call no solver or model setter.

Histories use finite-precision doubles. Large histories can consume memory and
produce long exports; they remain subject to the existing 100×100 model limits.
Text views scroll horizontally for wide matrices. Physical printer drivers and
other operating systems require their own acceptance checks.

## Regression checks

- `tests/teaching_algorithms.cpp`: exhaustive permutations for 500 small
  restricted min/max models, independent dual/feasibility checks, finite-cost
  semantics, 87 game saddle certificates, and arithmetic trace transitions.
- Existing `tests/assignment.cpp`, `tests/games.cpp`, `tests/games_graphical.cpp`
  retain random, rectangular, negative, tied, large, tiny and huge-value checks.
- `tests/desktop_teaching.cpp`: paired imports, the original 26/28 results,
  validated LP conversion, malformed-input transactions, navigation and exports.
- `tests/sensitivity_coordinates.cpp` retains all five mathematical regressions
  and distinguishes retained-basis intervals from full point-optimality ranges.

Input schemas and examples: [TEACHING_INPUTS.md](TEACHING_INPUTS.md).
