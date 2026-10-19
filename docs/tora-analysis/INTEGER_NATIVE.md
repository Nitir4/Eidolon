# Integer programming implementation and evidence

The clone now supplies a native mixed-integer input and branch-and-bound result workflow. `IntegerController` owns its own `LpInputController`; opening an Integer model cannot replace the Linear Programming module's document. The input table reuses the established LP fields (names, objective, rows, bounds and unrestricted flags), with a separate editable integer-flag row. Integer defaults to `y` for each new variable. The result shows the incumbent, termination status, every visited node, parent/depth, LP bound, branch split and selected relaxation values.

## Original evidence and compatibility

The retained REA inventory establishes the LP-like original Integer grid (`ev_fa604afe1424e6551c882a7f08584afba818f6371bdcb13b10ddf44ba606541f`) and an original saved file (`ev_9c5224b33fc744370cb1abaccf7241d3a48006446047ca20862f4c75cffa1f17`, private `reference/tora-captures/full-inventory/actions-int-save/tora-saved.txt`). The file begins marker `5000`, contains the same eight header fields and `(m+5)*(n+2)` LP grid records as marker3000, then one integer row of `n+2` records. Flags occupy the first `n` entries; the final two entries are preserved.

`compat::IntegerDocument` delegates the LP portion to `ToraLpCodec`, preserving original precision, quoted strings, and untouched raw spellings. It preserves original uppercase integer flags and the reserved final records. Decode rejects mismatched dimensions/record counts, invalid integer flags, missing CRLF and files over4MiB. An untouched original capture passed byte-exact decode/encode in a standalone probe. The later bounded original-runtime pass confirms the two automatic results and core guided interaction documented in [INTEGER_REA_ORACLES.md](INTEGER_REA_ORACLES.md). Exact iteration numbering, branch ordering and rendering remain distinct from the clone's independently validated solver.

## Execution flow and numerical decisions

1. Initialize a model or decode an original5000 file. The controller installs the LP document and integer flags transactionally.
2. Saving and leaving commit a visible table editor without applying solve-only validation. Saving an unfinished zero model is allowed. Solving also applies the original zero-row input guard.
3. Solve a continuous relaxation. Fixed variables are eliminated, finite lower bounds shifted to zero, free variables split, finite upper bounds represented as inequality rows, and equality constraints represented as two inequalities.
4. A separate dense two-phase simplex applies Bland entering and ratio tie rules. Every returned relaxation is checked against the original node's bounds and rows. The separate solver is necessary because the interactive LP engine intentionally preserves legacy arithmetic and returned an infeasible alternative optimum on a generated regression described below.
5. A relaxation whose normalized objective cannot beat the incumbent is pruned. Otherwise, branch on the first fractional designated integer variable: `xj <= floor(xj)` and `xj >= ceil(xj)`. Continuous variables remain continuous. Integer candidates are independently checked after rounding.
6. Exhausting the pending tree proves an optimum or infeasibility within numerical tolerance. Exhausting the user-selected node limit returns a partial incumbent and explicitly unproved status. Exceeding500 internal branch rows returns a separate model-resource status. An unbounded relaxation returns `UnboundedRelaxation`; it does not claim the integer program is unbounded.

The solver uses finite double arithmetic and feasibility tolerance1e-8 by default, not exact rational certificates. Tableau arithmetic overflow and iteration limits fail explicitly. Input dimensions remain1..100; internal transformations may have more rows/columns. The default node cap is4096; the UI allows1..100000. These are computational limits, not original-product behavior observations.

The LP regression has objective maximize `-3x1-2x2`, bounds `-2 <= x1,x2 <=4`, rows `x1+3x2 <= -3`, `-x1-x2 >=1`, `2x1+x2 >=0`. With explicit shifts and upper rows, the existing final LP engine returned Alternative with transformed values `(0,0)`, restoring `(-2,-2)` and violating the third row. The independent Integer solver passes this case. No shared LP implementation was changed by this work.

## Native behavior

Open/Save As uses original5000; writes are atomic. A read-only document requires a different save target. Leaving dirty input supports Yes/No/Cancel and preserves the previous model when replacement fails or is cancelled. Objective sense and number formats are editable. EditGrid copy/paste/insert/delete and one-level undo preserve integer flags when columns move, are copied or restored. Printing wraps title, model and visited-node trace through `QTextDocument`. Native screens expose a node limit and distinguish an incumbent from a proven optimum.

## Verification

Observed standalone checks before shared integration:

- `g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic ... src/integer/integer.cpp tests/integer.cpp ... && /tmp/tora-integer-tests`: passed300 seeded bounded two-variable models against independent complete grid enumeration and300 mixed models against independent per-integer-variable feasible-interval enumeration, plus mixed/free/negative/fixed bounds, zero rows, infeasible fractional equality, unbounded relaxation, the LP regression and terminal-node limits.
- A strict standalone `compat_integer` executable passed exact roundtrip, high precision, doubled quotes, mixed/uppercase flags and malformed framing/count rejection.
- `/tmp/tora-integer-original-probe reference/tora-captures/full-inventory/actions-int-save/tora-saved.txt`: passed byte-exact original capture roundtrip.
- Both native implementation sources passed `g++ -fsyntax-only -Wall -Wextra -Wpedantic` using Qt6 Widgets/PrintSupport includes.

`tests/desktop_integer.cpp` covers pure/mixed optimum, partial search, visible keyboard edits, original5000 save/open, module-state isolation, visible-editor Save As, read-only grid/API guards, read-only/dirty cancellation, failed-open preservation, flag-aware grid operations, one-level undo, PDF text and sizes. Shared CMake/Qt tests and rendered captures are pending root integration; those outcomes must be recorded after execution.

The subsequent [runtime oracle pass](INTEGER_REA_ORACLES.md) closes the original method/pure/mixed/core guided observation gaps. Remaining parity limits include exhaustive original guided behavior, exact output layout and original print-section selection.

## Guided-search work in progress (2026-10-18)

A bounded read of original printable strings found `Automated B&B`, `User-guided B&B`, branching-variable prompts, active-bound fathoming, green search-tree node selection and original-data/summary/tree print captions. This is static artifact evidence only; runtime method controls and outputs still require observation. No cutting-plane text occurred in the focused branch/cutting keyword scan; this bounded negative search does not establish absence.

An incremental `integer::Session` now separates processing a selected pending subproblem from choosing its fractional integer branching variable. The automatic solver uses the same session. Optional bound pruning affects only future node processing; a worse integer point cannot overwrite the incumbent when pruning is disabled. Existing independent 300 pure and 300 mixed tests passed after this refactor. New guided backend cases also passed: alternate x2-first branching, explicit frontier selection, integer/continuous rejection, pruning enabled/disabled, correct best incumbent and node limits. Native controller/screens passed syntax checking before the final incoming-branch display addition; shared Qt tests for new controls are pending. Native UI now supplies guided branching/frontier selection, active-bound fathoming, automatic continuation, a search-tree tab, and defining-branch paths.

Guided-search invariants: invalid frontier indexes, continuous variables and already-integral variables are rejected without changing the frontier or trace. A relaxation failure leaves the pending subproblem available rather than silently dropping it. The chosen incoming inequality is stored independently of finite variable bounds, so unrestricted-variable branch paths can also be inspected and printed. Bounds are used only to prune nodes that cannot improve the incumbent; disabling pruning never permits a worse integer point to replace the best point. Strict standalone guided/backend checks passed after these changes.

The parent observed the combined guided Integer/Queue boundary/all-module focused gate pass14/14 in4.42s. A subsequent review found that unrestricted pending lower bounds must render as `-infinity` without passing infinity to the finite number formatter. This was fixed and `guidedUnrestrictedBounds` added; its final shared test result is owned by parent closeout. Original runtime now confirms automated6/mixed7 and guided x2-first child bounds7.25/5.50; see the oracle ledger for retained evidence IDs.


## Final continuation verification

Pending guided children now appear in the tree immediately, with their real
parent/branch paths and explicit unsolved labels. Selecting one synchronizes
the pending choice. Native solving still investigates LPs incrementally;
original child pre-solving/search-order/color parity is not claimed.
The active-save regression now waits for real window activation and editor focus
and verifies pending text, saved model and reopened file. Diagnostics proved
the earlier actual-Wayland failure was a test focus race; no product change was
needed for editor commitment. Final local Release passes 161/161 in 112.14s;
actual Integer Wayland runs each pass 10 slots at 100% and 200%, including
setup/cleanup. See `build/continuation-parity/wayland-diagnosis.md` and the final
receipt `build/continuation-rea/verification.json`.
