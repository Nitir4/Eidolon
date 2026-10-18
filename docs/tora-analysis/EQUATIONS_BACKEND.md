# Linear equations and matrix inversion backend

The original application inventory observed **Linear Equations → Matrix Inversion / Linear Equations Solution** on 2026-10-16. The input presents a square system of N equations in N variables with an RHS column. The clone backend accepts rectangular systems as an additional capability; this does not claim that original TORA accepts them. Original zero-matrix output says “NO INVERSE: Determinant =0.00”. Nonzero original2000 and1000 outputs were subsequently captured on18October; see [continuation findings](REA_COMPLETION_20261018.md) for exact models, evidence IDs and limits.

## Implementation and meaning

`src/equations/include/tora/equations/equations.hpp` exposes `solve(A,b,options)` and `invert(A,options)` without Qt. `src/equations/equations.cpp` row-equilibrates the augmented system, chooses the largest available coefficient pivot in each column, and reduces all other rows using Gauss–Jordan elimination. This is an implementation choice; the inventory has not established original TORA's internal algorithm. Long-double intermediates improve arithmetic headroom on platforms with wider long double. Public outputs are binary64.

A full-column-rank consistent system returns `Unique`. A consistent rank-deficient system returns `Infinite`, a particular solution with all free variables zero, and one independent nullspace vector per free variable. The complete solution is x = x₀ + Σ tᵢ zᵢ. An inconsistent system returns a normalized row witness y: yᵀA is approximately zero, while yᵀb is nonzero. Coefficient rank and augmented rank are available separately. No fabricated solution is returned for inconsistency.

For example, x + 2y + 3z = 4 and 2x + 4y + 6z = 8 have particular solution (4,0,0) and nullspace vectors (-2,1,0), (-3,0,1). Replacing the second RHS with 9 produces a contradiction and the inconsistency witness. The tests independently multiply the original coefficients by returned solutions/nullspace vectors and verify the witness against the original system.

`invert` requires a square matrix. It first classifies rank; a deficient matrix returns `invertible=false`. A full-rank matrix uses a dedicated augmented-identity Gauss–Jordan reduction of [A|I], with actual row-operation snapshots. Both A A⁻¹ and A⁻¹ A populate `maximumIdentityResidual`. Determinant sign and logarithmic absolute magnitude are retained; an ordinary determinant is optional when binary64 cannot represent it. The main reduction is O(n³), bounded by the100-dimensional profile.

## Trace and numerical limits

Optional solve traces contain row scaling, row swaps, and elimination, including each resulting augmented matrix. Indices are zero based; an elimination step means row[row] += factor × row[otherRow]. A scale step multiplies the indicated row by its factor. Tracing starts with row equilibration; the caller retains the original matrix for an initial teaching view. The default records no steps. The configured cap truncates only trace storage, never the computation; `traceTruncated` reports it. Inverse traces contain the actual [A|I] reductions and share the native iteration controls.

Inputs must be finite, nonempty, at most 100 rows and 100 columns, with an RHS matching the row count. The tolerance must be finite and strictly between 0 and 1; the default is 10⁻¹². Trace caps above 10,000 are rejected. Outputs or requested trace factors outside binary64 range throw a runtime error. An exceptionally small coefficient can yield a finite solution without tracing but an unrepresentable equilibration trace factor with tracing; that condition is reported rather than clipped.

Rank is numerical and depends on tolerance after coefficient row equilibration. There is no exact-arithmetic rank guarantee or condition estimate. Nearly dependent matrices may be treated as rank deficient. Consistency checks compare transformed RHS cancellation against both a unit floor in equilibrated coordinates and the sum of transformed original RHS contributions. `maximumResidual` and `relativeResidual` describe a computed consistent solution against the original coefficients, with relative residual computed componentwise as |Ax-b| / (|A||x|+|b|). A small residual does not prove a small forward error for an ill-conditioned system. Witness coefficient residual and witness RHS similarly make the inconsistency assertion inspectable.

## Verification actually performed

Standalone compilation and execution passed on 2026-10-16:

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic -ffp-contract=off -fno-fast-math \
  -Isrc/core/include -Isrc/equations/include \
  src/equations/equations.cpp tests/equations.cpp -o /tmp/tora-equations-tests
/tmp/tora-equations-tests
```

The test executable independently verifies a known 2×2 solve and inverse, overdetermined unique solutions, underdetermined nullspaces, inconsistent row witnesses, zero/singular systems, row swaps and trace truncation, mixed 10⁻²⁰⁰/10²⁰⁰ equation scales, finite/dimension/tolerance validation, a 100×100 identity solve, 150 generated strictly diagonally dominant unique systems and their inverse identities, and 100 generated rank-deficient systems followed by known contradictory RHS changes. The deterministic random seed is 20261016. This is clone mathematical verification, not original-output parity verification. No GitHub Actions were run.

## Parent integration

Add `add_subdirectory(src/equations)` after `src/core` in root CMake. Register `tests/equations.cpp` as a test executable linked to `tora::equations`, following the existing tests CMake conventions. Shared root/tests registration is deliberately left to the parent agent to prevent concurrent edits. Native controls and nonzero original captures remain outside this backend deliverable.

## Native integration implementation

The additional files `src/desktop/equations_document.{hpp,cpp}`, `equations_screens.{hpp,cpp}`, and `equations_controller.cpp` provide an independent controller and square coefficient/RHS grid. `equationsModule()` registers `equations/input` and `equations/result`; this leaves shared shell/main registration with the parent. The two solve controls use the observed names **Linear Equations Solution** and **Matrix Inversion**. Solve output includes the original equations, numerical rank, solution classification, a particular solution/nullspace when applicable, and independent residual certificates. The clone's elimination trace supports Previous Iteration, Next Iteration, and All Iterations. It is explicitly a clone teaching trace, not established original iteration parity. Inverse output reports numerical singularity without inventing a determinant.

The observed `2000` save schema has N, D, notation code, quoted title, square dimension N, and N×(N+1) quoted numeric cells in row-major order; there is no second dimension record. The native codec preserves unchanged source bytes, rejects malformed/trailing fields and nonfinite values, and emits CRLF records after edits. Raw input text remains separate from parsed doubles. Save uses `QSaveFile`; read-only sources require saving a distinct copy before editing. A failed open or invalid cell edit preserves the last accepted model. Before saving/solving/navigation, visible cells are revalidated so invalid text cannot silently solve the old accepted values. Opening is limited to 4 MiB.

File → Numeric Format configures decimal/scientific notation and N/D through the established formatter bounds. Format changes preserve mathematical results but mark the document dirty. Print generates paginated result text with the active numeric format. The printer does not print the teaching trace. A subsequent QTextLayout update wraps arbitrarily wide matrix rows and titles and paginates the wrapped lines; parent verification of that latest printer update is pending.

`tests/desktop_equations.cpp` adds codec, controller, file/read-only, classification, invalid GUI text, iteration navigation, and PDF checks. These desktop tests were authored but not yet executed at the time of this entry; standalone backend tests including independent trace replay have passed. Parent build/test registration and original nonzero model captures remain outstanding.

Parent combined verification subsequently passed equations.backend and four desktop.equations scale checks (1, 1.25, 1.5, 2), within a nine-check focused group including Games. The last printer wrapping change and 3000-character PDF title regression were made after that pass and await rerun.


## Final native closeout — 18 October 2026

Original Matrix Inversion marker1000 was captured and byte-identically roundtripped;
Equations2000 remains the augmented-system format. Matrix-only input displays no RHS.
The inverse backend now records actual [A|I] Gauss--Jordan row operations, numerical
rank, left/right identity residuals, determinant sign/log magnitude and an optional
representable determinant. Native Prev/Next/All works for inversion as well as
equation solving, and Print includes the currently visible iteration selection.
New tests replay inverse operations and check signed/overflow-range determinants.
VB doubled quotes/literal backslashes replace C++ escaped quote encoding.

Final native focused22/22 checks passed6.98s across Equations/Games/Integer/Planning/
all-module acceptance; actual Wayland captures cover populated inverse trace.
The original1000 matrix was observed as input/save; the later continuation adds observed original inverse output and an associated
inverse/determinant summary to normal unique-system results. Exact original
determinant/tie/layout parity remains bounded by observed evidence.


## Final continuation verification

The nonzero original oracles and associated inverse/determinant result change
are documented in [the REA ledger](REA_COMPLETION_20261018.md). Focused checks
verify result preservation when only the optional inverse overflows, independent
trace dimensions and cache invalidation. Final combined local Release passes
161/161 in 112.14s; actual Equations Wayland runs each pass 10 QtTest slots at
100% and 200%, including setup/cleanup. The current report and native PDFs pass
page-bound audits. These supersede the earlier unexecuted native descriptions,
without claiming exhaustive original LU/tie/numerical profile parity.
