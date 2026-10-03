# Phase 0 Gate 0 review report — 2026-10-04

Gate 0 was approved by the user on 2026-10-04 (yes please go ahead).
The report below records the analysis presented at that gate; Foundation 1
is now active. Current implementation/CI status is in STATUS.json and
FOUNDATION1_REPORT.md.
Remaining unverified observations are explicitly listed in OPEN_QUESTIONS.md;
this report does not claim universal TORA compatibility or a passed gate.

## Oracle smoke test and reliability

TORA Windows 1.00, June 2002 launches under Wine 11.18 in a supported 800×600
virtual desktop. LP navigation, entering a model, saving/reading bytes,
reopening, graphical rendering, iterations, final summaries and sensitivity
all work. The user's native high resolution was rejected by TORA; the clone
must support native resolution/DPI independently of this legacy restriction.

The driver launches TORA if absent and reuses its visible instance between
cases. Its desktop is pinned/opaque across workspaces, so the user need not
continually switch desktops. Legacy focus/modal handling remains flaky;
explicit refocusing and state captures recover it without routine process
termination. Screenshots are validated and replaced atomically; interrupted
writes do not replace valid images. Both repair batches are complete and must
not be repeated. TORA itself exited after a printer runtime error; that is
an environment limitation, not a proposed clone behavior.

## Confirmed LP scope

The visible LP solve hierarchy has Graphical and Algebraic. Algebraic has
Final solution and five Iterations choices: All-slack starting solution,
M-method, Two-phase method, Bounded simplex, Dual simplex. Sensitivity appears
inside Final solution. Revised simplex and Parametric analysis are absent
from this observed hierarchy; neither has been invented as a product module.
Future OR modules use registration/plugin boundaries and are not built now.

## Completed evidence and checks

| Evidence | Result |
| --- | --- |
| Fifteen fixtures under five iteration selections | 73 full main traces plus two expected equality precondition warnings |
| Three finite-bound models, Bounded and All-slack each | Six additional full traces; corresponding pairs have identical observed tables |
| Combined table coverage | 79 full traces, 273 tables, 7,711 numeric cells; 734 contributing screenshot hashes checked |
| Colored pivot choices | 144/144 primal entering, 129/129 primal leaving, 5/5 direct Dual choices agree with the stated displayed-cell rules |
| Independent graphical studies | All 15: input, format, initial/one-stroke graph, zoom in/out; result panels visually transcribed |
| Independent final summaries | All 15: 13 complete numeric/sensitivity transcriptions plus two terminal-only summaries |
| Actual TORA Save As/reopen | All 19 round trips byte-identical, including varied dimensions/names/bounds/precision/scientific strings |
| UI-created decimal/scientific files | Flags 1/2 confirmed; default zeros 0.00/0.00000e+00 |
| Manual iteration probes | Exact entering/leaving validation, accepted normal pivot and alternate optimum (0.00,4.00), z=4.00 |
| Format/M/grid probes | Scientific output, decimal D=3, five M inputs with initial coefficients, row copy/paste/delete/insert |
| Specific file/graph/print probes | Marker/truncation/extra/LF behavior; gray three-variable Graphical item; two zooms; printer runtime error 482 |

`python3 tools/audit_phase0.py` passes evidence coverage/provenance with zero
issues. It runs the table and pivot audits, checks both repair plans are
empty, graphical/final capture completeness and hashes, all 19 recorded
round trips, required documents and unknowns sections. Observation scripts
also pass Python compilation. These are reference checks; no product tests,
OS CI runs, clone-versus-TORA comparisons or release artifacts exist yet.

Exact glyph transcription uses visually reviewed TORA digits/labels and
retains number strings, negative zero, blocked cells, ordering and source
screenshots. No LP arithmetic is used to fill missing golden values. Pivot
arithmetic only audits colored selections. Main traces remain labeled
assembled-transcription-needs-review; structural completeness is not blanket
independent human approval of all cells/labels/messages. Module tests-first
steps must review the corresponding golden data before implementation.

## Analysis and design deliverables

- docs/tora-analysis/: separate screen/method documents, shared pivot/file
  contracts, capture references and explicit unknowns; README.md is the index.
- ARCHITECTURE.md: Qt-free numerical core, LP model/preprocessing, solver
  strategies/session/immutable traces, presenters, codec and module extension.
- FILE_FORMAT.md: field-for-field successful CRLF/quoted-record schema and
  lexical preservation, with direct malformed-file findings.
- UI_SPEC.md: familiar menu/screen/table workflow, controls/states/palette and
  responsive native Qt layouts with the required DPI/monitor acceptance matrix.
- TRACEABILITY.md: requirements → captures → planned code → planned tests.
- OPEN_QUESTIONS.md: remaining observations assigned to foundation/module gates.

The architecture specifies one C++17+/CMake/Qt 6 Widgets codebase and one
GitHub Actions Windows/Linux/macOS OS matrix. Packaging is portable Windows
exe deployment, Linux AppImage, macOS app in dmg. Universal macOS and Windows
installer feasibility are assessed in Foundation 1. No web UI is permitted.

## Explicit review limits

Successful printing is blocked by this Wine configuration. Unknowns also
include Undo enablement/column operations, formatting limits/halfway rounding,
hidden numerical thresholds/cycling, unrestricted and inconsistent-bound
transformations, general graph rules, codec edge cases and automatic-final
iteration accounting. They are not silently claimed complete. Gate 0 approval
accepts the analysis/design with these recorded limits; any ambiguity affecting
a module must be resolved before that module's implementation/comparison gate.

The known M=100 infeasible trace reports optimal with a positive artificial
variable and objective -194.00. Preserve it as an explicit compatibility
regression, not an arithmetic correction. Positive final objectives 5.50/9.00
for cases 13/15 were confirmed; their equals sign had caused an earlier
transcription error and the manifests are corrected.

Raw captures, TORA files, binaries and runtime are gitignored and excluded
from release artifacts. None have been published. Publishing any TORA
screenshot/file requires the user's explicit approval.

## Gate decision and next authorized step

Review this report, ARCHITECTURE.md, UI_SPEC.md, FILE_FORMAT.md,
TRACEABILITY.md and OPEN_QUESTIONS.md. The user approved Gate 0 with these recorded limits.
The authorized next step is Foundation 1 only: CMake project skeleton and a single three-OS
CI workflow that builds and runs a trivial test, followed by its own gate.
