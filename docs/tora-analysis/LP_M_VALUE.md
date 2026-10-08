# M-value screen

## Purpose and controls

Selecting M-method, or cancelling the All-slack artificial-variable redirect,
opens the M-Method group. It has `Enter value of M:`, an edit with selected
100, and `Go To Output Format Screen`. The surrounding LP form has the
SolveModify menu. M is a finite numeric penalty, separate from display settings.

## Direct observations and validation

The default 100 is used by all main M corpus runs. Cancelling case 03's
All-slack warning enters this screen; Return alone did not advance the
captured route. Explicit focus/Go reaches the common format screen.
Evidence: `methods/lp/m_method/M-value-dialog-default-100.png` and
`probes/validation-20261004/cancel-to-m/after-cancel.png`.

Five additional probes replace the field contents explicitly, capture them,
activate Go, then open the initial M tableau for equality case 06. All five
proceeded with no validation dialog in these states:

| Entered M | Initial z-row x1, x2, Sx3, Sx4 |
| --- | --- |
| 0 | -3.00, -2.00, 0.00, 0.00 |
| -1 | -1.00, 0.00, -1.00, -1.00 |
| 0.5 | -4.00, -3.00, 0.50, 0.50 |
| 1e2 | -203.00, -202.00, 100.00, 100.00 |
| bogus | -3.00, -2.00, 0.00, 0.00 |

Thus `1e2` behaves as 100 in these visible coefficients; `bogus` behaves as
zero. Do not infer arbitrary string parsing from just these inputs. Captures,
input values and hashes: `probes/closing-20261004/m-validated-*/`.
The earlier `m-*` attempts appended text to 100 and are explicitly marked
invalid test inputs; they are excluded from these findings.

## Unknowns / things I could not verify

Maximum magnitude, whitespace/prefix/comma parsing, symbolic M (none observed),
overflow and all terminal consequences of small/negative penalties. These
probes establish input acceptance and initial coefficients, not mathematical
reliability for arbitrary M. Full M=100 traces remain the compatibility corpus.

## Phase 2 focused terminal observations

Full traces now exist under probes/phase2-m-method: zero/bogus reach iteration 3
with alternatives, -1 becomes unbounded after three tables, 0.5 stops at iteration
3 optimal despite a positive artificial value, and 1e2 reproduces the default
iteration 4 equality optimum. See LP_M_METHOD_MODULE.md for exact scope.
