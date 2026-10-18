# Zero-sum Game calculation backend

The Qt-free library `src/games` solves finite two-player zero-sum games with
1–100 strategies per player. Entries of A are payoffs to the row player,
who maximizes; the column player minimizes. Payoffs must be finite.
Native Games controls, production file editing and graphical presentation
remain integration work. The Graphical calculation backend is implemented in
[GAMES_GRAPHICAL.md](GAMES_GRAPHICAL.md). This is a mathematical implementation validated
against original output, rather than recovered TORA source.

For probability vectors p and q, the equilibrium satisfies

```
p >= 0, sum(p) = 1; q >= 0, sum(q) = 1
min_j (p^T A)_j = v = max_i (A q)_i.
```

These inequalities certify optimality against every opposing mixed strategy,
because any mixed payoff is a convex combination of pure-strategy payoffs.
`Result` returns both probability vectors, value, lower/upper payoff bounds,
duality gap and all pure saddle pairs when one exists. Nonunique equilibrium
strategies may differ from TORA; the reviewed examples have matching output.

## Algorithm and numerical profile

First compare exact pure maximin and minimax. Equality identifies a pure
saddle; choose the first row/column pair deterministically and retain all pairs.
Otherwise set s=max(abs(A)) and H=A/s+2, so H lies in [1,3]. Solve

```
maximize sum(y), subject to H y <= 1, y >= 0.
```

This has an initially feasible slack basis. Incremental tableau pivots use
Bland's smallest entering index, and smallest basic index for ratio ties.
At optimum the primal weights yield q=y/sum(y), while dual slack prices give
p=u/sum(u). Positive shifting preserves both players' optimal strategies.
The separate legacy LP trace engines remain unchanged.

When the positive supports have equal sizes and their indifference system is
nonsingular, solve the unshifted, scaled support equations with partial
pivoting, together with sum=1. This avoids shift cancellation; for example,
the symmetric +/-1e308 game retains exact 0.5 probabilities and value zero.
Non-square or singular supports keep the tableau strategies. All results
undergo normalization and full pure-strategy inequality checks afterward.

Default engineering tolerance is absolute1e-9/relative1e-10 in scaled payoff
units; pivot/support tolerance is 0.001 times those settings. This is not an
absolute error guarantee in original units or an exact-arithmetic solver.
Scaling may erase distinctions tiny relative to the largest payoff; full
finite IEEE input coverage is not claimed. Invalid data, failed certificates
or the default1024pivot safety limit produce explicit exceptions.
The tableau occupies O(m(n+m)) space; pivot count depends on the game.
Only final convex payoff bounds are clamped to [-1,1] before restoring units.

`rowPlayerLp(A)` separately generates the original displayed formulation:
maximize free v, v-A^T p<=0, sum(p)=1, p>=0. Variable order is v,x1,...,xm.
At100strategies this representation has101variables or constraints; the
Games solver supports100per player independently of the existing LP editor's
100-row/variable limit. Do not route this formulation through that editor
without handling its size limit.

## Verification and original observations

`tests/games.cpp` uses always-active checks in Debug and Release:
200seeded2x2games against independent closed-form solutions;
100general rectangular games with probability/inequality certificates,
positive scaling/translation and negated-transpose value identities;
pure/mixed/dominated/duplicate strategies, zero games,100x100symmetric game,
tiny and huge finite payoffs, invalid dimensions and nonfinite data.

Two actual original LP-based Game outputs were reviewed:

| Game A by rows | Value | Row mix | Column mix |
|---|---|---|---|
| 3,0 / 1,2 | 1.50 | 0.25,0.75 | 0.50,0.50 |
| 4,3 / 2,1 | 3.00 | 1.00,0.00 | 0.00,1.00 |

`tests/fixtures/games.json` retains independently transcribed values,
probabilities and displayed LP fields. The original <= glyph is normalized
to ASCII. `games.goldens` reconstructs authentic9000-format saved records,
checks SHA256 and compares52fields across both examples. Its reader is
test-only; it does not establish production import/export compatibility.
Private original files/screenshots remain excluded from shipping.

Mixed result Evidence:
`ev_46ffe6486760464b9c7f7f08a21d10019685149060476c4662a4dcc28a81d0dc`.
Pure input/result Evidence:
`ev_042d927c3c01be61401e7c9a69f3514a737612987a38ee6b1085f93f59b17626`.
Both producing captures exited0. The original Solve Problem submenu also
contains Graphical; its envelope backend now matches three original examples,
documented separately. Native plotting/zoom/printing acceptance remains.

The incremental engine passes the focused100strategy-inclusive test in0.23s,
compared with about27s for the initial trace-reconstruction implementation.
Latest integrated suite after Graphical backend:48/48headless in4.09s;
114/114desktop in107.94s;8/8focused Release in0.46s.
Logs:build/games/graphical-final-*.log and graphical-release-*.log.
These changes are not installed or packaged.

## REA and memory evidence

The constrained native import succeeded with two CPUs,4GiB memory high,
6GiB memory max and8GiB temporary swap allowance. Reuse the retained1201string
inventory and targeted xrefs in
`reference/tora-native/mcp-games-affinity-20261015/`; these are private.
The selected LP-result handler0x4fcaa0returned a non-retryable decompilation
failure; the native log reports an unresolved constructor at0x508889.
No recovered Game pseudocode is claimed and no identical retry ran.
The larger graphical handler0x477f10was identified but not decompiled.

`close_binary` saved `reference/tora-native/tora-games.snapshot.json`:
907554bytes,15Evidence records,zero primitive/workflow cache entries.
This preserves observations, not a promise to bypass future native import.
The owned session closed; its guard disabled and removed the temporary8GiB
swap file. Kernel swap table and receipt both confirmed cleanup. No fstab
or persistent swap setting changed.
