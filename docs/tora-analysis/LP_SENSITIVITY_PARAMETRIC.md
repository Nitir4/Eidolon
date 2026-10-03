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

Sensitivity for bound/unrestricted-variable transformations beyond the main
corpus, internal tolerance conventions, and any parametric-analysis path outside
the observed complete LP hierarchy remain unverified.
