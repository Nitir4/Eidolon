# Queuing birth-and-death backend

Original REA on 2026-10-16 observed an M/M/c scenario with input arrival λ, per-server service μ, server count c, system limit K, and source limit N, defaulting to 0, 0, 1, infinity, infinity. The original output offers Scenario 1 / Comparative Analysis and a table of state probability pₙ and cumulative probability Pₙ. Next/All Iterations are disabled. This backend implements stationary birth-and-death probabilities; it does not claim transient or non-Markovian queue support.

## Mathematics and original evidence

For infinite source population, the offered arrival rate in every state below capacity is λ. For finite source N, the rate in state n is (N−n)λ: λ is the rate of each idle source. The departure rate in state n is min(n,c)μ. Neighboring stationary flows satisfy pₙ λₙ = pₙ₊₁ μₙ₊₁. These rates and detailed balance derive the stationary weights, without guessing a separate closed formula for each finite model.

The observed original baseline λ=2, μ=3, c=1, K=N=infinity gives p₀=1/3, p₁=2/9, effective λ=2, Ls=2, Lq=4/3, Ws=1, Wq=2/3. Original finite-source λ=2, μ=3, c=1, K=N=3 gives probabilities (9,18,24,16)/67, effective λ=174/67, Ls=114/67, Lq=56/67, Ws=19/29, Wq=28/87. Captures are `reference/tora-captures/full-inventory/actions-queue-nonzero/output.png` and `actions-queue-finite-output/output.png`; REA inventory owns the full evidence record.

The original displayed rho/c remains raw λ/(cμ) even with finite source. This is distinct from actual utilization E[min(n,c)]/c. The backend exposes both, using optional `offeredLoadPerServer` when the raw ratio exceeds binary64 output range. Original rejects K>N, including infinite K with finite N. The mathematical helper instead bounds the chain by min(K,N), or by N alone when K is omitted. A native parity flow must validate K<=N before invoking that additional mathematical capability.

## Computation and certificates

`src/queuing/include/tora/queuing/queuing.hpp` exposes `Model`, `Result`, `Metrics`, `solve`, and `probability`. Finite models normalize weights in log space after subtracting their maximum. Infinite-source/infinite-capacity models require λ<cμ and use the exact geometric tail starting at c; no truncation decides stability or moments. Probabilities at arbitrary later states are computed from the stored log-tail ratio, preserving near-critical ratios that could round to one in a plain binary64 ratio field.

Queue and system means are expected max(n−c,0) and n. Throughput is μ times expected busy servers; this avoids subtracting nearly equal offered and rejected rates under heavy blocking. For finite-source arrivals, both blocking and wait probabilities weight each state by its actual attempt rate. Time-full probability p_K is shown separately from rejected-attempt fraction; these generally differ with finite source. W and Wq divide L and Lq by admitted throughput. The relative neighboring-flow residual makes the stationary balance inspectable.

Zero λ yields the idle distribution, zero throughput and population, and the limit W=1/μ, Wq=0; `zeroArrivalLimit` explicitly labels these conditional waiting-time limits. Infinite capacity with λ>=cμ returns `Unstable`, without invented finite stationary metrics. A subsequent boundary implementation permits μ=0 with positive λ: finite capacity/population produces an absorbing full state and explicitly undefined waiting times at zero throughput; infinite population/capacity has no stationary distribution. Both rates zero are rejected because every state would be absorbing without a unique stationary distribution. Servers are limited to 10,000 and finite K/N to 100,000. Limits must be positive and rates nonnegative, with at least one rate positive. Nonfinite inputs are rejected. Binary64-unrepresentable metrics throw rather than saturate. Long-double intermediates provide extra headroom on the tested Linux platform; platforms where long double equals double may reject more extreme rate combinations.

## Verification actually performed

On 2026-10-16 this standalone command compiled without warnings and the executable passed:

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic -ffp-contract=off -fno-fast-math \
  -Isrc/queuing/include src/queuing/queuing.cpp tests/queuing.cpp \
  -o /tmp/tora-queuing-tests
/tmp/tora-queuing-tests
```

Tests cover independently known M/M/1 and M/M/2 values, equal-rate finite-capacity uniform probabilities, a finite loss system, a three-source/three-server binomial distribution, the captured original three-source/one-server values, distinct time-full and arrival-blocking values, zero arrivals, unstable infinite queues, 300 generated finite models with detailed-balance/normalization/Little/service-rate/time-scaling certificates, extreme heavy and light traffic, near-critical infinite traffic, and input validation. An initially mistyped expected finite-source fraction failed and was corrected to the independently derived and original-captured denominator 67; no implementation was altered to match the incorrect expectation.

## Primary references

The state-dependent birth/death interpretation and neighboring-flow equations are explained by [MIT, Urban Operations Research, §4.5](https://web.mit.edu/urban_or_book/www/book/chapter4/4.5.html). [MIT §4.6.3](https://web.mit.edu/urban_or_book/www/book/chapter4/4.6.3.html) explains finite-capacity stationarity, loss probability, and Little's law using admitted rather than offered arrivals. The finite-source weighting, numerical log normalization, implementation limits, and oracle comparisons above are this project's explicit derivation and validation, not quotations from those references.

## Native scenarios and file implementation

New independent desktop files are `src/desktop/queuing_document.{hpp,cpp}`, `queuing_screens.{hpp,cpp}`, and `queuing_controller.cpp`. `queuingModule()` exposes `queuing/input` and `queuing/result`, with a scenario grid (λ, μ, c, K, N), add/delete controls, numerical-format configuration, per-scenario and comparative summaries, and configurable First state / State count controls for stationary probability/cumulative tables. The clone profile supports 1..100 scenarios and up to 1000 displayed states starting at 0..1,000,000. Finite chains are stored completely; the display range never truncates the mathematical solve. Print wraps and paginates the selected summary and displayed state range. No transient iteration controls are invented.

REA subsequently captured the original finite scenario save bytes in `reference/tora-captures/full-inventory/actions-queue-saved/tora-saved.txt`: marker `6000`, N/D/notation, quoted title, scenario count, followed by five quoted λ/μ/c/K/N strings per scenario. The new codec preserves an unchanged source exactly, writes CRLF after edits, validates field count and finite/whole-number values, and bounds dimensions before allocating. Native K<=N validation matches the observed original rule. Input infinity is currently represented explicitly as the string `infinity`; exact original saved infinity spelling still awaits a capture, so original infinite-model file parity is not yet claimed. Finite-record parity is established by that captured schema.

Controller operations use atomic `QSaveFile`, a 4 MiB input file limit, read-only source protection with Save As to a distinct file, pending grid revalidation, dirty tracking, and mathematical-result invalidation after data changes. Failed input/open operations preserve accepted state. Numerical-format changes retain mathematical results and mark the document dirty. Unstable infinite models display an explicit absence of stationary distribution. Zero-arrival waiting-time limits are labeled.

`tests/desktop_queuing.cpp` contains captured finite-record byte roundtrips, malformed/bounds checks, known original finite/infinite metrics, multiple scenarios/comparison, native K>N validation, read-only/save/open state preservation, unstable/zero-arrival behavior, invalid visible input refusal, state-range navigation, and PDF checks. Those desktop tests are authored but await parent registration and execution. `queuing_document.cpp` also compiled standalone with warnings enabled successfully. The backend standalone tests passed again after raw-load/log-tail refinements. Shared CMake/main files remain parent-owned.


The μ=0 boundary was subsequently implemented and the standalone backend suite passed again. Finite capacity/population returns the full-state point mass, zero admitted/completed throughput, occupied-server and queue counts, and attempted/rejected rates at that full state. Waiting times and admitted-arrival delay probabilities are undefined, explicitly flagged by `absorbingNoService`; no zeros are presented as meaningful waiting times. When finite source population equals capacity, no idle sources remain, so even blocking as a fraction of attempted arrivals is undefined (`arrivalBlockingUndefined`). Infinite capacity/population with positive λ and zero μ is unstable. Both rates zero are rejected. Tests cover each case; these are mathematically established stationary boundaries pending original-output capture, not claimed original transient parity. The native summary labels undefined values accordingly, and its regression has been extended but awaits execution.

Further REA inventory found an original λ=0, μ=3, infinite-N run displaying **Pure Death Model (N=9999)**, Ls=472.10614, and zero probabilities for states 0..10 (`reference/tora-captures/full-inventory/actions-queue-scenario/scenario.png`). Those values do not describe the stationary idle distribution. Original pure-death/transient/default-truncation semantics therefore remain an explicit gap: the clone's λ=0 output is labeled as a stationary limit, and mathematical boundary support must not be called original pure-death parity. The inventory released the GUI to parent/network work; further boundary captures require that owner. Integer/network records use the literal `infinity`, but a queue-specific infinite-limit save still has not been captured.

Additional verification actually performed: a standalone Qt-free codec executable read the actual original finite capture file (not only an embedded fixture), verified byte-identical unchanged encoding and λ/μ/K values, added an infinite-source scenario, and verified an edited encode/decode roundtrip. It compiled and ran successfully with `g++ -std=c++17 -Wall -Wextra -Wpedantic -Isrc/core/include -Isrc/queuing/include -Isrc/desktop src/core/number_formatter.cpp src/desktop/queuing_document.cpp /tmp/tora-queuing-codec.cpp -o /tmp/tora-queuing-codec` followed by `/tmp/tora-queuing-codec`. The additional infinite-source roundtrip establishes clone internal consistency, not original infinite-save spelling. Warning-enabled standalone Qt syntax checking of `queuing_controller.cpp` and `queuing_screens.cpp` also passed against the installed Qt6 headers; native linking/QTest execution remains parent-owned.

## Unit-time pure-birth/death behavior: 18 October 2026

Additional REA runtime observations resolve the earlier boundary gap. These use
the original executable and existing owned Wine/Xvfb runtime; they do not rely on
static strings as execution evidence.

* Death: lambda0, mu1, c1, K=N3 gives probabilities
  `(1-2.5/e, 0.5/e, 1/e, 1/e)` for states0..3. Original displays
  `0.08030,0.18394,0.36788,0.36788`, cumulative ending1.00000, and Ls0.91970.
  Evidence `ev_93b520c673dd615c80f2011cb7761d1fa09fa3d03985c6da1afdfdd19655dceb`.
  This is the constant-death distribution `X=max(N-Poisson(mu),0)` at unit time.
  Its mathematical mean is5.5/e≈2.02333693. Original Ls is the partial moment
  excluding stateN,2.5/e≈0.91969860; it must not be described as the full mean.
  A subsequent comparison with c2 retains the same p0/Ls despite c2 in the
  comparative row (`ev_01e8c1fed6e1953832935e8ac0df074c665acde7812e66e3a0508b0922a050ef`).
* Birth: lambda2, mu0, c1, K=N=infinity gives Poisson2 probabilities, p0.13534,
  p1.27067,p2.27067,p3.18045 and Ls2.00000. Evidence
  `ev_a5648c781c557fdd2c40ff58f70e5b9fa47276e023d3eaab7e47959d1b1abf33`.
* Birth with K=N3 still gives those Poisson probabilities and Ls2; the displayed
  table ends at3 with cumulative.85712. This is a display cutoff, not a censored
  distribution. Evidence
  `ev_ca6bc571fb54b473ca0e2cd262133df42c1614403867cde570abd8bdebb3539b`.

`transient.hpp/.cpp` computes constant-rate pure birth/death distributions with
an explicit time parameter (default1), using a Poisson recurrence seeded at its
mode. It also reports the mathematical mean, CDF and an omitted-tail bound. The
native zero-rate branches choose this observed unit-time profile; general
positive-rate scenarios still use the stationary birth-and-death calculation.
The requested mathematical pure-death profile uses N as its initial population;
for infinity the clone selects an explicit reference population of 9999. The
original application's displayed 9999 label does not establish that population:
the bounded continuation below observed an effective population of 500.
The display clearly identifies transient results, constant rates independent of
c, and pure-birth independence from K/N. Waiting times and stationary throughput
are inapplicable. The partial death moment is explicitly labeled; no arbitrary
legacy numerical bug is silently substituted for a mathematical expectation.
The infinite-N original Ls472.10614 is now explained by the effective population
cap and omission of its terminal-state contribution, with a small remaining
legacy numerical discrepancy described below. Exact legacy arithmetic parity
is not claimed.

The backend limits rate*time to100000 and initial population to100000. Source
allows an explicit nonnegative time as an extension; the native original profile
uses t1. Large means use a mode-centered PMF rather than exp(-mean) recurrence
from0, avoiding collapse to zero by underflow. The tail beyond
mean+16sqrt(mean)+100 has a geometric upper bound. Independent tests verify500
seeded distributions/moments, time scaling, extreme rates and original rounded
values. The standalone strict-warning compile and test passed before native
integration. Positive-rate stationary tests remain separate.

Original marker6000 files using literal `infinity` were accepted when opened in
the original application. Two original Save As attempts remained disabled while
a grid editor was active; no actual infinite-record export is claimed by those
attempts. Original finite export parity remains supported by its retained save.

The Poisson counting-process interpretation is supported by
[MIT's primary Poisson process lecture](https://ocw.mit.edu/courses/6-041sc-probabilistic-systems-analysis-and-applied-probability-fall-2013/pages/unit-iii/lecture-14/).
The death PMF, expected value and observed original discrepancy above follow
from the explicit stopped-counting-process derivation, rather than a steady-state
queue formula. Source/UI behavior and limitations are this project's evidence.

## Effective pure-death population cap — continuation on 18 October 2026

The retained comparative observation
`ev_70f0aca672e0cca0913dc05e293de07d84f095c58cf78996fce55274ad42c154`
shows original partial Ls values 0.52276, 25.50638, 92.02124, 472.10614 and
472.10614 for mu=3, c=1 and N=3,30,100,9999,infinity. The terminal-state capture
`ev_02d1e607f742e4060a1d6164cbe4e32e98304487d6cbb39e33240b621931d660`
still labels N=9999 but ends at states 491..500, with p500=0.04979 and CDF=1.

The boundary follow-up
`ev_5437a6ca4f4f8b2b5f32efa8e59c4ff45be00405d75044a0da2f9271dcfa5269`
observes equal original Ls for N=500 and 501 at mu=1 (315.05973), equal values
for N=500,501,1000 at mu=3 (472.10614), and the same mu=1 result for infinity.
The N=501 terminal table still ends at 500, where p500=0.36788 and CDF=1.
This directly supports a 500 effective-population cap in these tested profiles.
The native compatibility comparison applies `min(N,500)` and effective N=500
for infinity, while retaining the requested mathematical model separately.
The cap outside the observed profiles is a compatibility inference, not an
exhaustive measurement of every original rate/server/input combination.

For effective N=500, the independently computed true mean is 497 at mu=3;
excluding the terminal state's contribution gives
`497 - 500*exp(-3) = 472.1064658...`. The original 472.10614 differs by about
0.00033. At mu=1, the corresponding independently computed partial moment is
`499 - 500/e = 315.0602794...`, differing from original 315.05973 by about
0.00055. The old large discrepancy is explained; those small numerical errors
are retained as observed legacy results rather than copied into the backend.

`QueuingController::solve` caches requested and compatibility distributions
separately. `outputText` labels the requested mean/population and requested
probability table, then provides a clearly separated original comparison with
effective population, true mean, partial mean and terminal probability/CDF.
Captured original Ls is shown only for effective N=500, c=1 and mu=1 or 3.
Other inputs show the independent compatibility calculation without inventing
an original numerical oracle. The same text feeds native output and printing.
The Qt-free numerical algorithm and original input documents are unchanged.

`tests/desktop_queuing.cpp::originalDeathCapComparison` covers the requested and
capped populations at N=499,500,501,1000,9999,infinity, mu=1/3, and unmatched
rate/server cases; it checks requested-state support and invalidation too.
The separate Release build in `build/continuation-queue/` completed with no
warnings. Backend/transient CTest checks passed 2/2 in 0.07 seconds; native
Queuing checks at scales 1,1.25,1.5,2 passed 4/4 in 1.67 seconds. The captured
requested/compatibility screens were visually reviewed at 100% and 200%, and a
representative PDF was reviewed. All four emitted PDFs are single-page with
nonempty text and no off-page word boxes, including the requested N=9999 state
table and separate original N=500 comparison (`pdf-audit.json`). These are
offscreen native checks, not a fresh Wayland or physical-printer claim.


## Final compact layout and combined gate

The continuation's final output form uses separate scenario/range rows, bounded
combo-box sizing, wrapped result text and stacked print/back actions. Actual
200% Wayland review confirmed all controls fit horizontally; the mathematical
comparison remains scrollable. A compact-width regression protects this layout.
Final affected scale checks pass 4/4 in 0.93s, followed by the complete local
Release 161/161 in 112.14s. Actual Queue Wayland tests pass all 12 slots at both
100% and 200%, including setup/cleanup. Final logs and PDF audit are under
`build/continuation-layout/`; six-module/scale receipt is
`build/continuation-rea/verification.json`. Earlier pending descriptions above
are historical. No physical-printer or cross-platform release claim is made.
