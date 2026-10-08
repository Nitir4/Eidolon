# Phase 0 traceability

This maps observed behavior to requirements, code and tests. Gate 0 is approved.
The original analysis table records the full module plan; the foundation tables
below identify implemented build, shell, numerical, model and file portions.
Remaining LP screen/controller and solver behavior belongs to Phase 2.
Paths in the evidence
column are relative to reference/tora-captures/. Each full-trace test must use
ordered labels, number strings, phase/iteration headings, selections and
terminal messages, not just the objective. Raw OCR is a draft, not a golden.

| ID | Requirement / observed behavior | Capture or analysis evidence | Planned code | Planned verification |
| --- | --- | --- | --- | --- |
| NAV-01 | Welcome/Click Here and complete MAIN label order | smoke/basic-lp-800/05-installed-main-menu.png; tora-analysis/APPLICATION_SHELL.md | ApplicationShell, ModuleDescriptor | Label/order and navigation tests |
| NAV-02 | New/existing → grid → Solve/Modify → Graphical or Algebraic → Final or Iterations | smoke/basic-lp-800/18-solve-submenu.png, 19-algebraic-submenu.png, 20-iterations-submenu.png; LP_SOLVE_MENU.md | LpController, screen stack | Exact menu path tests |
| NAV-03 | Five iteration choices; Sensitivity in final view; Revised/Parametric absent from observed hierarchy | same complete hierarchy; LP_SENSITIVITY_PARAMETRIC.md | ToraMethodPolicy, module registry | Method ordering and summary-section tests |
| IN-01 | New/decimal defaults N=5/D=2; scientific alternative | methods/lp/corpus/input-mode.png; targeted new-mode captures | LpInputModeScreen, NumberFormatter settings | Radio/default and notation tests |
| IN-02 | Title, counts, objective, names, constraint relations/RHS, bounds, unrestricted flags and default rows | methods/lp/corpus/01_normal/all_slack/input.png; roundtrips/probe-1var-1con/input-reopened.png; LP_INPUT.md | LpProblem, LpValidator, input presenter | Ordered field/default binding tests |
| IN-03 | 100×100 accepted; 101×101 exact Student-version size warning | probes/dimensions-n100-d100/ and dimensions-n101-d101/after-grid-initialize.png | LpValidator | Boundary and exact-message tests |
| IN-04 | File menu Save, Save As, Main Menu, Exit TORA | probes/input-menus/01_normal/file-menu.png | LpController, input screen | Command order/enabled/navigation tests |
| IN-05 | EditGrid labels/shortcuts; selected row red; Copy/Paste enablement | probes/input-menus/01_normal/editgrid-row-selected.png; probes/closing-20261004/grid-focused/copy-menu.png | GridEditController | Selection/menu state tests |
| IN-06 | Row copy/paste replaces values; Delete updates count/order; Insert creates zero row before target | probes/closing-20261004/grid-focused/observations.json | GridEditController | Data/order/count tests derived from observed commands; Undo/column/shortcut execution remains Q-03 |
| FILE-01 | Text open/save pickers; open read-only checkbox; save prompt when leaving grid | smoke/basic-lp-800/09-save-prompt.png, 10-save-dialog.png; roundtrips/*/save-as-dialog.png | ProblemDocument, native dialogs | Save prompt/open/save flow; read-only Q-04 |
| FILE-02 | 3000, N/D/mode/title/n/m/sense, quoted row-major grid, CRLF; preserved numeric lexemes | FILE_FORMAT.md; roundtrips/*/tora-saved.txt, observations.json | ToraLpCodec | Nineteen byte-level round trips, field-order and compatibility-open tests |
| FILE-03 | Decimal mode 1/scientific 2; zero strings 0.00/0.00000e+00 | probes/targeted-20261004/new-decimal/ and new-scientific/ | ToraLpCodec, NumberFormatter | UI-created-file mode/format tests |
| FILE-04 | Marker mismatch Retry dialog; truncated/extra/LF-only specific behavior | probes/file-errors-20261004/*/attempted-input.txt and after-open.png; FILE_BEHAVIOR.md | ToraLpCodec, diagnostic presenter | Specific malformed fixtures; safe diagnostics without copying crashes |
| FMT-01 | Decimal D=2/D=3; scientific N=5 lowercase e/sign/two exponent digits; infinity | probes/closing-20261004/scientific/, decimal-three/; LP_OUTPUT_FORMAT.md | NumberFormatter | Exact display-string fixtures; limits/halfway rounding Q-01/Q-02 |
| M-01 | M default 100; 0/-1/0.5/1e2/bogus accepted to initial tableau with recorded coefficients | LP_M_VALUE.md; probes/closing-20261004/m-validated-*/ | MValueScreen, ToraMethodPolicy, MMethodStrategy | Input acceptance and observed objective-row tests |
| ITER-01 | Ordered Basic/variables/Solution, objective/basis/bound/sign rows; sx/Sx/Rx spellings | methods/lp/corpus/*/*/transcribed-trace.json; LP_ITERATION_SCREEN.md | SolverTrace, IterationTablePresenter | Full ordered table/string golden comparisons |
| ITER-02 | All-slack/finite M/Two-phase/Bounded/Dual headings and actual redirected method | methods/lp/corpus/progress.json and per-run first-table captures | ToraMethodPolicy, solver strategies | Requested/actual method and warning tests |
| ITER-03 | All-slack ≥/= redirect defaults to Two-phase; Cancel enters M=100 | corpus/03_infeasible/all_slack/; probes/validation-20261004/cancel-to-m/ | ToraMethodPolicy, LpController | Both warning branches |
| ITER-04 | All-≤ M/Two-phase redirect to All-slack; inapplicable Dual redirects; equality Dual error | corpus/01_normal/{m_method,two_phase,dual_simplex}/; 06_equality/dual_simplex/ | ToraMethodPolicy, LpValidator | Exact precondition/warning messages and navigation |
| PIVOT-01 | Most-improving primal cost/leftmost tie; positive ratio/topmost tie; zero steps allowed | corpus/pivot-policy-audit.json; LP_PIVOT_POLICY.md; 11_zero_rhs traces | SolverStrategy, PivotRequest validator | 144 entering/129 leaving observed selections and targeted ties |
| PIVOT-02 | Direct Dual most-negative RHS/eligible minimum ratio; five selected pivots | corpus/10_dual/dual_simplex/; pivot-policy-audit.json | DualSimplexStrategy | Selected row/column and complete dual sequence |
| PIVOT-03 | Incorrect manual entering/leaving dialogs; accepted x1/sx4 produces next table | probes/targeted-20261004/manual-pivot/; LP_ALL_SLACK.md | PivotRequest validator, iteration controller | Exact rejection text/colors/Next behavior |
| PHASE-01 | Phase 1/2 headings, objective restoration, visible blocked artificial objective cells, infeasible phase message | corpus/06_equality/two_phase/ and 03_infeasible/two_phase/; LP_TWO_PHASE.md | TwoPhaseStrategy, IterationTablePresenter | Phase transition and infeasible stopping tests |
| BOUND-01 | Finite bounds cause Ux/Lx/ULx complemented columns; original bound rows retained | corpus/finite-upper/, finite-two-upper/, finite-lower/; LP_BOUNDED_SIMPLEX.md | LpPreprocessor, BoundedSimplexStrategy | Six complete bounded/all-slack comparison traces |
| TERM-01 | Exact optimal/unbounded/infeasible/alternative messages; degenerate alternative notice | corpus/*/*/terminal-message.png; LP_PIVOT_POLICY.md | Termination presenter, strategies | Exact message/iteration and degenerate golden tests |
| TERM-02 | M=100 infeasible model stops optimal with positive artificial Rx5 and z=-194.00 | corpus/03_infeasible/m_method/; LP_M_METHOD.md | Explicit Tora compatibility policy | Legacy regression without silently changing output |
| ALT-01 | Select zero-cost x2 after notice; x1 leaves; Iteration 3 alternate endpoint (0,4), z=4 | probes/alternative-20261004/observations.json | SolverSession, iteration controller | Manual alternate pivot and appended-table test |
| SCROLL-01 | Accumulated vertical iterations; artificial columns require horizontal scroll; full last table | corpus/06_equality/m_method/tableau-scroll-*-right.png; coverage-audit.json | QTableView models, iteration presenter | Every table/column accessible and correctly ordered |
| OUT-01 | Final header/objective/count, value/coefficient/contribution, RHS/slack/surplus | output-studies/*/final/transcribed-summary.json; LP_FINAL_SOLUTION.md | FinalSolutionPresenter | Fifteen final-output fixture comparisons |
| OUT-02 | Summary count can differ from interactive Two-phase (case 06: 6 versus 5) | output-studies/06_equality/final/final-output.png; corpus/06_equality/two_phase/ | Final-solution policy, SolverTrace | Distinct-path count regression |
| SENS-01 | Thirteen numeric sensitivity sections; two terminal-only summaries; infinity/dual/reduced-cost strings | output-studies/summary-transcription-audit.json and per-case transcriptions | SensitivityPresenter, analysis strategy | All observed range/price/string tests; bound transforms Q-09 |
| GRAPH-01 | Fifteen independent graphs with model, constraints, feasible dots, objective and marker/results | output-studies/*/graphical/ and transcribed-graph-result.json | GraphicalPresenter, native QPainter view | Result strings and native drawing comparisons |
| GRAPH-02 | Three-variable Graphical disabled; abnormal graph messages; one-stroke action | probes/validation-20261004/graph-three-vars/; 03/04 output studies | LpSolveMenu, GraphicalScreen | Eligibility/state/message tests; other eligibility Q-05 |
| GRAPH-03 | Two zooms Normal spans about -1…5 → -1…4 → -1…3; return captured | probes/targeted-20261004/graph-zoom/ | Graph viewport model | Observed zoom/return; general ticks Q-05 |
| PRINT-01 | Write to Printer and Print Graph controls; successful output unavailable under Wine | probes/targeted-20261004/printing/write-to-printer.png; FILE_BEHAVIOR.md | Native PrintAdapter | Successful printer/preview/pagination requires Q-08 resolution |
| AUDIT-01 | 81 selections: 79 full traces + 2 warnings, 273 tables, 7,711 numeric cells | methods/lp/corpus/coverage-audit.json; phase0-evidence-audit.json | Future golden fixture loader | Coverage/provenance audit; independent review Q-10 before module implementation |
| ARCH-01 | Qt-free numerical/model core, replaceable strategies, module registration, preserved file lexemes | ARCHITECTURE.md | or_core, or_lp, tora_compat, desktop_shell | Dependency/registration/round-trip tests |
| DPI-01 | Native Qt layouts/font metrics, 1366×768…4K, four scales and mixed monitors | UI_SPEC.md; user requirements (not tested against clone yet) | ApplicationShell, view models, native graph | DPI/monitor layout and access matrix |
| BUILD-01 | One branch/tree, C++17+/CMake, one Windows/Linux/macOS CI matrix and native artifacts | ARCHITECTURE.md; user requirements | CMake and one GitHub Actions workflow | Build/full test suite and portable exe/AppImage/app-in-dmg artifacts; Foundation 1 |

The completed repair plans are empty and must not be used to schedule another
corpus sweep. `OPEN_QUESTIONS.md` assigns remaining observations to their
foundation/module gates. All proposed test areas above are future work; the
current Python audits check reference evidence only. BUILD-01 now maps to
`CMakeLists.txt`, `.github/workflows/build.yml`, `tools/package_desktop.py`,
`tests/core_smoke.cpp` and the `desktop.smoke` CTest startup check. Actual CI
results and artifact evidence are recorded in `FOUNDATION1_REPORT.md`.

## Foundation 5 implemented mappings

Only the following UI portions are implemented by Foundation 5. Numerical,
model and document libraries are covered by the later foundation tables;
printing, grid editing and solver portions remain planned.

| IDs | Implemented code | Verification | Observation source |
| --- | --- | --- | --- |
| NAV-01 | `src/desktop/application_shell.cpp`: welcome/MAIN/native menu and module registrations | `tests/desktop_shell.cpp`: navigationAndDefaults, moduleRegistration | smoke/basic-lp-800/04-installed-welcome.png, 05-installed-main-menu.png |
| NAV-02, NAV-03 | `src/desktop/lp_screens.cpp`: route frameworks and exact SolveModify hierarchy; solver leaves disabled | navigationAndDefaults, methodHierarchy | smoke/basic-lp-800/11-solve-modify-menu.png, 18-solve-submenu.png, 19-algebraic-submenu.png, 20-iterations-submenu.png |
| IN-01 | Input-mode radios and read-only N=5/D=2 defaults | navigationAndDefaults | smoke/basic-lp-800/15-input-mode-retry.png; LP_INPUT_MODE.md |
| IN-02 | Empty dimension/title screen framework; fields disabled, no model/grid yet | navigationAndDefaults, scaledLayouts | smoke/basic-lp-800/06-input-dimensions.png; LP_INPUT.md |
| IN-04, IN-05 | File/EditGrid labels, shortcuts and disabled data commands | navigationAndDefaults | probes/input-menus/01_normal/file-menu.png; LP_INPUT.md |
| FMT-01, M-01 | Format and M screen frameworks with observed defaults; no numeric acceptance/formatting yet | methodHierarchy, scaledLayouts | smoke/basic-lp-800/21-final-output-format.png; LP_M_VALUE.md |
| ARCH-01 | Qt-free existing core targets, desktop_shell descriptor/factory contract and duplicate route checks | core.smoke, moduleRegistration; headless preset | ARCHITECTURE.md; extension requirement |
| DPI-01 | Scroll-wrapped layout-managed stack, Qt DPR policy, font sizing, geometry persistence and screen event handling | scaledLayouts: 84 route/resolution/scale combinations per OS; geometryAndPreferences: negative origins, desktop gaps, unplugging and preferences | User's resolution/DPI requirements; UI_SPEC.md (physical mixed-monitor acceptance still pending) |
| DPI-02 | Application/widget font repair and classic native Qt styling | desktop.font; malformedThemeOnEveryScreen; live full-window screenshot with readable MAIN | User's missing-glyph screenshot; original clone UI captures under build/foundation5/screens/ (gitignored) |

CI uploads only original clone screenshots alongside its test XML. No raw
TORA screenshot or saved file is included in source/release artifacts.
Foundation 5 evidence and explicit preview deviations are in
`FOUNDATION5_REPORT.md` and `UI_SPEC.md`.

## Foundation 2 implemented mappings

| IDs | Implemented code | Verification | Capture / requirement |
| --- | --- | --- | --- |
| ARCH-01, NUM-01 | `src/core/include/tora/core/matrix.hpp`, `tableau.hpp`, `src/core/tableau.cpp`: ordered value-owned storage, caller-requested transactional pivots | `tests/core_numerics.cpp`, Qt-free headless preset | Normal all-slack initial and Iterations 2/3; NUMERIC_FOUNDATION.md source hashes |
| NUM-02 | `src/core/include/tora/core/tolerance.hpp`: explicit finite absolute/relative comparisons | Boundary, below-unit relative, exact-zero, extreme overflow, NaN/infinity and caller-policy tests | Engineering requirement; thresholds are explicitly not asserted to be TORA's |
| FMT-01, FMT-02 | `src/core/number_formatter.cpp`, public formatter header: locale-independent observed decimal/scientific display | `tests/core_formatting.cpp`, 80 exact TSV golden strings in classic and comma locales | Foundation 2 focused captures plus existing Phase 0 summaries, mapped and hashed in NUMERIC_FOUNDATION.md |
| FMT-03 | Formatter settings/nonfinite diagnostics and signed infinite sensitivity endpoints | N=0 decimal rejection, precision bounds, NaN/infinity tests | decimal-n0/runtime-error.png; signed-zero/summary.png; safe replacement for observed crash |
| BUILD-01 | Qt-free static core target, C++17 and consistent compiled floating operations | Full three-OS workflow and headless build | User requirements; FOUNDATION2_REPORT.md |

Pivot-selection, file compatibility and LP result construction
remain behind their own gates. Formatting is available in the library; the
Foundation 5 GUI remains the same navigation preview until the relevant module
can supply real model/session values.

## Foundation 3 implemented mappings

| IDs | Implemented code | Verification | Capture / requirement |
| --- | --- | --- | --- |
| IN-02, ARCH-01 | `src/lp/problem.cpp`, `include/tora/lp/problem.hpp`: value-owned ordered LP fields, aliases separate from x labels, observed defaults | `tests/lp_model.cpp`: all 22 original transcriptions, default model, independent copies, ordered/checked labels | Existing 15-case corpus, three finite-bound inputs and four Save As/reopen probes; LP_MODEL_VALIDATION.md lists every source/hash |
| IN-03 | `src/lp/validation.cpp`: dimensions checked before allocating, zero-count and 100-variable/constraint Student limits | `tests/lp_validation.cpp`: zero/max/overflow-size cases and exact captured messages | Existing 100/100 and 101/101 probes; foundation3/dimensions/zero-variables, zero-both, zero-constraints-two |
| MODEL-01 | Finite coefficient/RHS and shape/enum safety; ordered located diagnostics | lp.validation: NaN/infinity, ragged objective/rows, bad enums, aggregation/order | Engineering safety requirements; message origins explicitly tagged, no guessed legacy numeric parser |
| MODEL-02 | Raw bound/sign preservation, bound consistency independent of y | lp.validation: equal/reversed/signed bounds, y, signed zero and unsnapped small coefficients | Six foundation3/bounds input/Save As/menu checks; reviewed reversed-bound dialogs, including y |
| MODEL-03 | Infeasible/unbounded inputs admitted; validation does not normalize or solve | lp.model: captured infeasible/unbounded, equality, minimization, negative RHS and unrestricted inputs | Existing corpus and probe-3var-2con round trip |
| BUILD-01, ARCH-01 | Qt-free `tora::lp` static library in the existing workflow | C++17 headless tests and full Windows/Linux/macOS suite | FOUNDATION3_REPORT.md |

Near-bound thresholds, free-variable expansion and negative-bound solver
transforms remain later preprocessing/module obligations. File compatibility
is covered by Foundation 4 below; GUI editing/binding belongs to Phase 2.

## Foundation 4 implemented mappings

| IDs | Implemented code | Verification | Capture / requirement |
| --- | --- | --- | --- |
| FILE-02, FILE-03 | `src/compat/lp_codec.cpp`, `include/tora/compat/lp_file.hpp`: header/grid parsing, lexical precision, defaults/settings, preserved reserved/trailing records | `tests/codec_roundtrip.cpp`, `tests/verify_codec_hashes.py`: 27 exact saved-file transcriptions, byte counts and actual SHA-256 values; optional actual-private-file comparison | `tests/fixtures/tora-file-settings.tsv` lists all private TORA save paths/hashes; FILE_FORMAT.md |
| FILE-04 | Located typed errors, observed marker warning, strict bounded malformed-input parsing | `tests/codec_errors.cpp`: exact marker title/body/origin, count/shape/framing/number/relation/flag errors, signed zero/reserved preservation | probes/file-errors-20261004; engineering safety policies in LP_FILE_IO.md |
| FILE-05 | Default doubled-quote recovery and writer, opt-in rejection | compat.errors: decoded title/full model, exact storage, malformed-quote rejection | probes/foundation4/text/text/{text-entered,reopened}.png and tora-saved.txt; user explicitly approved recovery |
| FILE-01, FILE-06 | `src/compat/lp_file_store.cpp`: source/read-only/dirty checkpoints, native binary paths, Save/Save As and atomic replacement | `tests/file_store.cpp`: exact Unicode filename, read-only Save redirect/copy, complete precision, replacement rollback, invalid serialization leaves destination intact | probes/foundation4/readonly and readonly-save; engineering filesystem policies in LP_FILE_IO.md |
| FILE-07 | Codec preserves opaque legacy text bytes; native picker suffix, current Save destination and overwrite decisions specified for the future controller | compat.roundtrip/hash golden for byte E9; compat.files exact-path test; future controller tests for picker and cancel behavior | probes/foundation4/unicode-title, extension, save-target and overwrite; FILE_BEHAVIOR.md |
| BUILD-01, ARCH-01 | Qt-free C++17 static `tora::compat` with the existing single OS-matrix workflow | Headless and full desktop suite; F4 CI results in gate report | FOUNDATION4_REPORT.md |

Dialogs, overwrite confirmation and grid bindings remain the first Phase 2
module's controller work. File-library checks do not claim those preview UI
actions are enabled.

## Phase 2 LP input implemented mappings

| IDs | Code | Verification | Capture / requirement |
| --- | --- | --- | --- |
| IN-01, IN-02, IN-03 | lp_screens.cpp, LpInputController: New/Existing, title/count initialization and limits | desktop_input: defaults, native navigation, all 27 saved input bindings | Existing 15-case corpus; count-return/count-tab; Foundation 3 dimensions |
| IN-04, MODEL-02 | LpGridModel, InputDelegate: aliases, sense, numeric/relation/bound/flag edits | 22 exact saved-file hashes at four scale factors; real Return/Tab keyboard editing | tests/fixtures/tora-input-edits.tsv; phase2-input matching directories |
| IN-05 | Typed axis clipboard/reshape, native EditGrid actions | Row/column Copy/Paste/Delete/Insert, two-variable Delete floor, ordered inserted defaults | closing/grid-focused; column-copy-paste/delete-three/insert-menu; undo-menu-state |
| IN-06 | SOLVE all-zero LHS check and generic module leave guard | Zero-row exact message; MAIN allowed; Yes/No/Cancel/picker failure retains input | column-copy-paste/save-unavailable.png; original Save? captures |
| FILE-01, FILE-06, FILE-07 | Native Qt dialogs, controller, CP1252/Unicode-path boundary | Real Open read-only checkbox and Save Cancel; failed Open/Save; read-only copies and preserved checkpoints | Foundation 4 picker corpus; three edited-file live bridges in same clone capture folder |
| FILE-05 | Existing doubled-quote recovery used by alias edits | Quoted alias exact bytes and full model preservation | quoted-alias-keyboard; reopen-quoted-alias/reopened.png, approved recovery exception |
| FILE-04, IN-01 | Controller retry loop and blank Existing input route | Wrong-marker Yes/No/retry-picker Cancel; Existing picker Cancel; retained last Save destination | file-errors-20261004/wrong-marker/after-open.png; FILE_BEHAVIOR.md |
| DPI-01 | Dialog font polishing | Real Qt picker with malformed application font remains readable | User's black-box screenshots and native-font requirement |
| MODEL-02, FILE-02 | Validator/codec accepts unrestricted negative-infinite lower bound | lp.validation, compat.errors, exact free-flag file hash | flag-replace-keyboard, flag-restore |
| FORMAT-01 | NumberFormatter/codec support profile extended to 99 | 86 exact observed number strings, six new profile saved-file outcomes | format-n11/d11/d11-value/scientific-n11/n99/d99; N=999 failure recorded separately |
| UI-01, BUILD-01, ARCH-01 | Native table, font metrics, persistent controller and module guard | Normal/100×100 inputs at 3 resolutions × 4 scales; full single-workflow OS matrix | PHASE2_INPUT_REPORT.md; clone-only CI screenshots; hardware limits retained |

No reference screenshots, files or executable are product/release resources.

## Phase 2 all-slack implemented mappings

| IDs | Code | Verification | Capture / requirement |
| --- | --- | --- | --- |
| LP-AS-01 | AllSlackSession::requiresArtificial / normalized | 27 exact classifications; GUI OK/Cancel handoff | all-slack.json capture fields; seven original Two-phase redirects; negative-RHS case12 |
| LP-AS-02 | AllSlackSession recommendation/choice/next | 57 complete table strings, selected pivots; manual-error rollback | 20 direct sessions / 19 unique inputs; LP_ALL_SLACK_MODULE.md |
| LP-AS-03 | Lower shifts, complements, basis reconstruction | Finite upper/lower goldens and reverse Ux1 transition | Existing finite-upper/two-upper/lower; new slack-complement-return |
| LP-AS-04 | Free-column orientation and basis reconstruction | x1+/x1- traces, crossing; D=12 and scientific residual checks | slack-unrestricted/negative/crossing, crossing-precision/scientific |
| LP-AS-05 | Session status/terminalMessage and controller selection | Exact optimal/alternative/unbounded text, manual alternative pivot | Original degenerate/alternative; alternative-20261004; slack-unbounded |
| LP-AS-06 | LpSimplexController / TraceModel | Every golden label/column/value bound to Qt at four scales; manual colors/errors; initial optimal popup | LP_ITERATION_SCREEN; original normal/zero-RHS; Foundation2 signed-zero |
| LP-AS-07, DPI-01 | Native layouts, font metrics and scroll bars | 3 resolutions × 4 scales; 100×100 scroll/pause; native menu/output settings | User's full-window and font-glyph requirements; clone-only UI captures |
| LP-AS-08 | Timer, cached display rows and input invalidation | Pause on navigation, stale session discarded, resumable numerical cap | Engineering responsiveness/safety; cap not asserted as TORA policy |
| LP-AS-09 | QtPrintSupport / printSimplexTrace | Native PDF creation and trace non-mutation | Native printing requirement; Q-08 retains unverified TORA print fidelity |
| BUILD-01, ARCH-01 | Existing single OS-matrix workflow and Qt-free library | Full core/GUI tests, deployed startup and three packages | PHASE2_ALL_SLACK_REPORT.md records verified final run |

Golden selection metadata is checked where a captured choice exists; absent
selection metadata is not invented from an unselected initial frame. Direct
table labels/numbers and terminal messages are fully compared. Raw captures
are excluded from repository and package contents.

## Phase 2 M-method implemented mappings

| IDs | Code | Verification | Capture / requirement |
| --- | --- | --- | --- |
| LP-MM-01 | requestMMethod/setPenalty, native M group | Default reset, real menu/format, bogus/exponent/zero/negative/fraction values | m-method.json, LP_M_VALUE; phase2-m-method/m-* |
| LP-MM-02 | MMethodSession / PrimalSimplexSession initializer | Exact initial cost/RHS, surplus-first and original-row basis allocation | 03/04/06/07/08/10/15 original M traces |
| LP-MM-03 | Shared recommendation/choice/next | 78 tables and observed choices; manual rollback/resumable cap | LP_PIVOT_POLICY; 15 direct M/eight fallback sessions |
| LP-MM-04 | Cross-product inversion, dual-first costs, shared bounds/free normalization | Four scientific residuals, Ux1/Lx1/x1+ traces | m-dual-scientific-short, m-upper/lower-bound, m-free-variable |
| LP-MM-05 | Shared terminal/alternative selection | Exact optimal/alternative/unbounded; positive artificial stopping retained | Original 03_infeasible; zero/negative/fraction/nonnumeric |
| LP-MM-06 | Request handoffs, TraceModel, native M/output controls | All table cells at four scales; OK/Cancel fallback; inverse sx/Rx colors | Original normal fallback; m-slack-cancel-button/solve |
| LP-MM-07, DPI-01, SCROLL-01 | Shared native screen/timer/printing; synchronized Basic/number views | Three resolutions/four scales, 100×100 horizontal pinning/vertical sync/basis-label click/pause, visible Solution, stale data/PDF checks | desktop_m_method; original wide-table right-scroll captures; native requirements; Q-08 retained |
| ARCH-01, BUILD-01 | Common Qt-free primal base; one OS-matrix workflow | Full suite/deployed packages for all three OSes | PHASE2_M_METHOD_REPORT.md |

Original reference files/screenshots stay excluded from source and releases.
