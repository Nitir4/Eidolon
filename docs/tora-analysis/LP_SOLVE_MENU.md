# LP Solve/Modify screen and menus

## Purpose and workflow

The input grid's `SOLVE Menu` command asks whether to save data. After the
prompt, TORA presents a blue screen with a `SolveModify` menu and a centered
popup headed `SOLVE/MODIFY`. The observed entries, in order, are `Solve Problem`,
`View/Modify Input Data`, `MAIN Menu`, and `Exit TORA`.

`Solve Problem` opens `Graphical` and `Algebraic`. Algebraic opens `Final
solution` and `Iterations`. Iterations opens `All-slack starting solution`,
`M-method`, `Two-phase method`, `Bounded simplex`, and `Dual simplex`.
The complete visible LP hierarchy contains no Revised simplex or Parametric
analysis entry. Sensitivity belongs to the final-summary table.

## State and navigation

Submenus open by hovering over their parent. View/Modify returns to the input
grid; MAIN returns to module selection. Selecting a method can show a warning
and choose another algorithm before reaching output format. The requested
menu choice and the algorithm actually shown must be recorded separately.

## Evidence

`reference/tora-captures/smoke/basic-lp-800/18-solve-submenu.png`,
`19-algebraic-submenu.png`, and `20-iterations-submenu.png`.

## Unknowns / things I could not verify

Menu mnemonics/keyboard navigation, all context-dependent disabled entries,
and any LP workflow reachable outside the captured hierarchy remain unverified.
