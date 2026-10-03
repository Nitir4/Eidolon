# Phase 0 traceability

This maps observed behavior to requirements, proposed code and future tests.
Gate 0 is approved. LP behavior implementations/tests below remain planned;
Foundation 1 implements only build boundaries and native bootstrap tests.
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
