# Sensitivity and parametric analysis

## Purpose and observed location

The observed sensitivity information describes allowable changes to objective coefficients and constraint right-hand sides while reporting reduced costs and dual prices. It is shown within the Algebraic final-output summary. The observed menus provide no separate parametric-analysis choice.

## Direct observation

Sensitivity analysis is verified as part of Algebraic → Final solution. The
output summary includes variable objective-coefficient ranges and reduced cost,
plus constraint RHS ranges and dual prices. It is not a separate menu item in
the observed solve menu.

The complete visible Algebraic → Iterations submenu contains `All-slack
starting solution`, `M-method`, `Two-phase method`, `Bounded simplex`, and
`Dual simplex`; it contains no `Revised simplex` or `Parametric analysis`
entry. The Solve Problem submenu contains only `Graphical` and `Algebraic`.
Sensitivity remains visible only in the Algebraic final-output summary in the
captured workflow. This confirms menu absence for the observed LP paths, not
absence from every possible TORA screen or menu.

## Complete summary study

All fifteen final summaries were independently captured. Thirteen show numeric
sensitivity sections; infeasible case 03 and unbounded case 04 show only their
termination message, with no sensitivity table. Every numeric summary cell is
transcribed by reviewed glyph matching in
`reference/tora-captures/output-studies/<case>/final/transcribed-summary.json`.
The audit contains thirteen complete numeric summaries and two reviewed terminal
summaries. It includes degenerate and alternative-optimum ranges, literal
`infinity`/`-infinity`, reduced costs, and dual prices. These values come from
TORA pixels, not a recomputed inverse basis.

## Unknowns / things I could not verify

Bound/unrestricted transformations outside the 35 captured summaries, internal
tolerance conventions, and any parametric-analysis path outside
the observed complete LP hierarchy remain unverified.


## Accepted module contract and tests-first evidence

The original fifteen independent summaries plus ten focused final summaries
provide the initial sensitivity golden corpus. Two terminal summaries have no
numerical sensitivity section. Exact cells, including scrolled LB/UB ranges,
come from private pixel transcriptions with source hashes, not recomputation.

The native screen places `***Sensitivity Analysis***` between the constraint
summary and variable coefficient table. Coefficient/RHS headers and row labels
follow the original ordering. NumberFormatter supplies the selected decimal or
scientific display; open endpoints are literal infinity/-infinity.

The implemented Qt-free calculator consumes the retained final basis. It intersects
linear reduced-cost inequalities for coefficient changes and basic-feasibility
inequalities for RHS changes, including finite variable ranges. Artificial
columns blocked in Phase 2 do not constrain coefficient optimality. These are
one-at-a-time intervals, not a simultaneous-change guarantee. MIT's primary
[Lecture 7](https://live.ocw.mit.edu/courses/15-093j-optimization-methods-fall-2009/bd61ce1f4f56a5010cd7b1d9ab90ea8c_MIT15_093J_F09_lec07.pdf)
supports the standard derivation; actual TORA determines compatibility.

Native tests fail before UI changes on the missing sensitivity heading (23
numeric cases); terminal-only cases still pass. Numerical API and safety tests
fail on the missing sensitivity interface. Evidence is retained in
build/phase2-sensitivity/. Additional targeted bound/orientation cases are
being transcribed to resolve legacy conventions before accepting this module.


## Completed captured compatibility profile

The accepted corpus contains 35 summaries: the original 15, ten final-output
probes and ten sensitivity probes. The two terminal summaries contain no
numeric sections. New tests preserve signed minimization reduced costs,
active/inactive bounds, negative lower bounds, oriented negative free variables,
active negative RHS, multiple bound ordering and mixed lower/upper behavior.
The case paths and source hashes are in `tests/fixtures/sensitivity.json`.
All 35 actual saved-file round trips are byte-identical; scrolled cells retain
separate screenshot hashes. Private module directories link existing captures.

The calculator intersects reduced-cost optimality inequalities for each
objective coefficient, excluding basic and blocked artificial columns. RHS
changes use the final transformed identity column and intersect basic lower
and finite upper feasibility limits. Bounds follow the retained basis, with
LB and UB rows interleaved per variable. These intervals preserve the current
basis one parameter at a time; they do not promise simultaneous feasibility
or uniqueness at endpoints. The native screen and printer consume the same
ordered, formatted `SummaryDisplay`.

The following conventions are observed compatibility rules, including legacy
results that differ from mathematically corrected original-model sensitivity:

- Current coefficient/RHS entries are the originally entered values. A negative
  free-variable coefficient stays negative; its interval follows the oriented
  tableau (captured current -3.00, interval -6.00 to infinity).
- Reduced cost is signed zj-cj. The minimization nonbasic example displays
  -3.00. Dual prices retain normalized tableau signs.
- A negated negative RHS anchors the normalized delta at the entered RHS.
  The active -1.00 row has range -1.00 to 2.00 and dual price -5.00.
- Positive lower rows display their original bound, with a zero lower endpoint
  where captured. Bound rows follow each variable's LB then UB order.
- A nonbasic upper-complement variable does not impose a separate nonnegative
  bound-width floor on the displayed interval: the minimization upper case
  shows -infinity to 3.00.
- In automatic final solving, a negative lower bound is treated as zero without
  increasing a finite upper bound. Original lower shifts are restored to values
  and objective but not constraint slack calculations. The combined -1/1 case
  shows x1=0.00, x2=3.00, objective 6.00 and UB slack 1.00-. Its UB row appears
  in the final constraint summary but is absent from sensitivity. Original
  input/file data and interactive simplex methods are preserved.

Tests first recorded missing API/heading failures. The expanded combined-bound
regression recorded objective 7.00 versus TORA 6.00 before the final correction;
all captured strings now match. Safety tests independently re-solve inside and
outside normal coefficient/RHS intervals. Native tests verify model invalidation,
PDF nonmutation, actual window geometry and exact cells at four DPI scales.
Successful original printer output and physical mixed-DPI monitor handoff remain
unverified. No parametric or revised-simplex method was invented from menu absence.
