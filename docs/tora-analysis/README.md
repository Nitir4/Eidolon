# Phase 0 analysis index

All behavioral claims are based on the running TORA Windows 1.00 reference
under Wine 11.18 at 800×600. Proposed clone design and inferred rules are
identified separately. Raw captures/files remain private under reference/.
Each screen/method document records unknowns; shared contracts avoid repeating
input/file definitions for every solver. Foundation implementation status is
recorded in ../STATUS.json; this index retains the observation documents.

| Screen or method | Document |
| --- | --- |
| Startup / MAIN | APPLICATION_SHELL.md |
| LP overview / scope | LP_OVERVIEW.md |
| New/existing / notation | LP_INPUT_MODE.md |
| Input grid, limits, editing | LP_INPUT.md |
| Solve/Modify menus | LP_SOLVE_MENU.md |
| M-value setup | LP_M_VALUE.md |
| Output format | LP_OUTPUT_FORMAT.md |
| Algebraic branch / variable allocation | LP_ALGEBRAIC.md |
| Shared iteration view | LP_ITERATION_SCREEN.md |
| Automatic/manual pivot and terminal rules | LP_PIVOT_POLICY.md |
| All-slack | LP_ALL_SLACK.md |
| M-method | LP_M_METHOD.md |
| Two-phase | LP_TWO_PHASE.md |
| Bounded simplex | LP_BOUNDED_SIMPLEX.md |
| Dual simplex | LP_DUAL_SIMPLEX.md |
| Graphical | LP_GRAPHICAL.md |
| Final solution | LP_FINAL_SOLUTION.md |
| Sensitivity / menu absence of Parametric and Revised | LP_SENSITIVITY_PARAMETRIC.md |
| File/print workflows and error probes | FILE_BEHAVIOR.md |

See ../ARCHITECTURE.md, ../FILE_FORMAT.md, ../UI_SPEC.md, ../TRACEABILITY.md,
../PHASE0_REPORT.md and ../OPEN_QUESTIONS.md for the Gate 0 review package.

Foundation 2 arithmetic/display decisions and targeted capture provenance:
[NUMERIC_FOUNDATION.md](NUMERIC_FOUNDATION.md).

Foundation 3 input-model decisions and targeted validation capture provenance:
[LP_MODEL_VALIDATION.md](LP_MODEL_VALIDATION.md).
