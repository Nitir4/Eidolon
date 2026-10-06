# Foundation 3 gate report

Foundation 3 supplies the Qt-free LP model and validation. Foundation 2 was
approved when the user authorized this step. The approved foundation order
remains **1 → 5 → 2 → 3 → 4**. Stop here for review before Foundation 4.

## Implemented

- Static C++17 `tora::lp` library linked only to the numerical core.
- Value-owned ordered objective, variables, aliases, constraints, relations,
  RHS, bounds and independent unrestricted flags.
- Observed new-problem defaults, checked x/constraint labels, copies and
  before-allocation Student-edition dimension limits.
- Ordered diagnostics with field locations and an explicit distinction between
  observed TORA messages and engineering safety messages.
- Structural/enum/nonfinite safety and bound consistency without formatting,
  solver tolerances, preprocessing or changes to original input values.

The existing native navigation preview is retained. GUI editing/binding,
file codecs, preprocessing and solvers are outside this gate. Infeasible and
unbounded models remain accepted inputs whose statuses belong to solvers.

## Evidence and verification

Tests were written before implementation. `lp.model` validates all 22 original
transcriptions from completed reference cases: the 15-case LP corpus, three
finite-bound inputs and four saved/reopened field probes. No corpus sweep was
repeated. Ten targeted dimension/bound observations reused running TORA under
Wine at 800×600. Successful saved files and every claimed dialog/acceptance
frame were reviewed directly. One Save As attempt needed a retry; explicit
window placement prevented desktop focus changes from resizing another app.
The reference instance was retained.

The captures resolve zero counts and reversed bounds, including unrestricted
y. Tests match the captured Problem Size and Student size-limit messages and
the two-line Input Data Error. Equal/signed bounds and flags retain raw values;
no solver transformation is asserted. A zero-constraint follow-up resolved an
initial blank frame. The negative-upper method-format frame remains incomplete;
its input and Solve/Modify acceptance are observed, its algorithm behavior is
assigned to the solver gate.

Local Debug passed **11/11** including the prior GUI scale/font checks.
The Qt-free configuration explicitly compiled C++17 and passed **5/5**.
Three-OS CI is pending until the committed source completes the existing
workflow. Raw TORA files, screenshots and executables remain excluded from
committed source and CI artifacts.

Capture paths/hashes, diagnostics and precise limits are in
[LP_MODEL_VALIDATION.md](tora-analysis/LP_MODEL_VALIDATION.md); requirements map
to code/tests/evidence in [TRACEABILITY.md](TRACEABILITY.md).

**Gate status: local checks passed; three-OS CI pending.** Foundation 4 must
wait for this gate report and user approval. Its scope is the observed TORA
file codec and round-trip compatibility, not solver implementation.
