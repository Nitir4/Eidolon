# TORA algebraic LP solution

## Purpose and selection

`Algebraic` is the observed menu branch for a final solution summary or an iteration-by-iteration tableau. Its `Iterations` submenu offers five methods.

## Direct observation

The observed menu exposes `Final solution` and an `Iterations` submenu. The
iteration choices are `All-slack starting solution`, `M-method`, `Two-phase
method`, `Bounded simplex`, and `Dual simplex`.

The final-solution output screen was captured for the smoke file. It is titled
`LINEAR PROGRAMMING OUTPUT SUMMARY` and shows title, final iteration number,
objective value, a variable table, a constraint table, and a `Sensitivity
Analysis` section. The variable table columns are `Variable`, `Value`, `Obj
Coeff`, and `Obj Val Contrib`. The constraint table shows `Constraint`, `RHS`,
and `Slack-/Surplus+`. The sensitivity section has variable columns `Current
Obj Coeff`, `Min Obj Coeff`, `Max Obj Coeff`, `Reduced Cost`, and constraint
columns `Current RHS`, `Min RHS`, `Max RHS`, `Dual Price`.

The final output has `Next Iteration`, `All Iterations`, and `Write to Printer`
controls. On the smoke case it reports final iteration 3 and objective value
10.00. It reports x1=2.00, x2=2.00; objective coefficients 3.00 and 2.00;
objective contributions 6.00 and 4.00; constraint slacks 0.00, 0.00, and
1.00; and sensitivity ranges visible in capture 22.

The all-slack method screen is titled `SIMPLEX TABLEAU - (Starting All-Slack
Method)`. Its instructions say: click a nonbasic variable column to enter,
click a basic variable row to leave, and click `Next Iteration`; the current
iteration may be executed without steps 1 and/or 2. The table has a `Basic`
column, variable/slack columns, and `Solution`, plus lower/upper-bound and
unrestricted rows. Iteration 1 and Iteration 2 were captured. Clicking Next
Iteration a third time displays `Optimal Tableau` / `Iteration 3 is optimal`.

Additional direct captures verify that M-method asks for an M value (default
100), Two-phase labels Phase 1 and Phase 2, Bounded simplex uses an upper-bound
row and `Ux1` label for a finite x1 upper bound, and Dual simplex rejects
equality constraints with a dedicated warning. Each method was also exercised
on the 15-fixture corpus; see
`reference/tora-captures/methods/lp/corpus/progress.json` and the method-specific
documents.

## Unknowns / things I could not verify

Hidden precision, near-tie/cycling thresholds and artificial-variable cleanup
outside the observed cases remain unknown. The 15-case corpus is fully
transcribed; `LP_PIVOT_POLICY.md` records
the observed rules and `coverage-audit.json` validates its numeric coverage.


## Observed variable allocation in mixed models

Fresh M-method inputs reveal why ordering cannot be supplied by a generic LP
library. In case 03, the columns after x1/x2 are `Sx3`, `sx4`, `Rx5`, `sx6`;
the initial basis is sx4/Rx5/sx6 in original constraint order. Case 06 has
Sx3/Sx4 followed by Rx5/Rx6/Rx7. Case 04 has Sx3/Sx4/Sx5 followed by
Rx6/Rx7/Rx8. This is consistent with first allocating surplus columns for all
>= rows, then allocating an initial-basis column for each original row:
ordinary slack for <=, artificial for >= or =. Equality introduces no surplus
column. This inferred allocation rule is grounded in these three direct traces;
finite bounds and unrestricted-variable expansion need separate confirmation.
Evidence: each named case's `m_method/iteration-01.png`.

For case 03 at M=100, the first objective row is -101.00, -1.00, 100.00,
0.00, 0.00, 0.00 with Solution -300.00. Its basis RHS values are 1.00, 3.00,
5.00. The latest repeat again reaches an Optimal Tableau dialog while retaining
a positive artificial value. This behavior is recorded for compatibility;
it does not establish a correct general feasibility test.
