# TORA LP module — observed overview

Reference: TORA Windows-version 1.00, June 2002, running under Wine 11.18 in
an 800x600 virtual desktop. Evidence is under
`reference/tora-captures/smoke/basic-lp-800/`.

## Menu structure observed

TORA starts with a splash screen titled `Windows-version 1.00, June 2002`
and a `Click Here` button. It opens a centered popup headed `MAIN`. The menu
lists `Linear Equations`, `Linear Programming`, `Transportation model`,
`Integer programming`, `Network models`, `Project Planning`, `Queuing
analysis`, `Zero-Sum Games`, and `EXIT TORA`. The startup and main-menu states
were relaunched and captured on 2026-10-03 at 800×600 under Wine 11.18; see
`reference/tora-captures/methods/lp/corpus/startup-screen.png` and
`welcome-screen.png`.

Selecting `Linear Programming` opens the input-mode flow. After loading an LP, the
`SOLVE/MODIFY` menu contains `Solve Problem`, `View/Modify Input Data`,
`MAIN Menu`, and `Exit TORA`.

`Solve Problem` contains `Graphical` and `Algebraic`. `Algebraic` contains
`Final solution` and `Iterations`. `Iterations` contains `All-slack starting
solution`, `M-method`, `Two-phase method`, `Bounded simplex`, and `Dual
simplex`.

Sensitivity analysis is present inside the Algebraic final-output summary; it
is not a separate solve-menu item. The complete visible Iterations submenu
lists the five methods above; `Revised simplex` and `Parametric analysis` are
not present in the observed LP solve hierarchy. This describes the observed
paths only.

## Verified smoke workflow

1. Welcome screen requires a supported legacy display size. 800x600 worked;
   1024x768 reached the menu but was less reliable before installation repair.
2. Main menu → `Linear Programming` opens input-mode selection.
3. Input-mode selection offers `Enter New Problem` or `Select Existing File`,
   decimal notation (`NNNNN.DD`) or scientific notation (`N.NNNNeDD`), and
   `How many N?` / `How many D?` fields. The observed defaults were N=5 and D=2.
4. The LP grid accepts title, variable/constraint counts, objective sense,
   coefficients, relation strings, RHS values, lower and upper bounds, and an
   unrestricted-variable flag.
5. Clicking `SOLVE Menu` asks `Do you wish to save data?` even after a loaded
   or newly entered problem. `Yes` opens a native save dialog with file type
   `Text`; the smoke file was saved as `smoke.txt`.
6. The graphical screen displays the model text, `Zoom In`, `Zoom Out`, and
   `Print Graph`. For the smoke case, the user-supplied capture confirms the
   constraint lines, dotted feasible region, objective line, and `Optimum`
   point at (2,2).

## Verified smoke result

```
Maximize z = 3.00x1 + 2.00x2
x1 + x2 <= 4.00
x1      <= 2.00
     x2 <= 3.00
x1, x2 >= 0
```

TORA displays `Objective value = 10.00`, `x1 = 2.00`, and `x2 = 2.00`.

## Unknowns / things I could not verify

See the method-specific documents for direct observations of all five methods.
The 15 fixtures have been run under each of the five visible iteration
choices. The per-run captures are under
`reference/tora-captures/methods/lp/corpus/`. All 37 planned repairs have completed; all 73 solving selections reach tableaux,
with two expected equality warnings. Exact screenshot-glyph transcriptions now cover all 73 main traces and six
additional finite-bound traces without missing rows/columns. Both repair plans
are empty; `coverage-audit.json` checks 273 tables and 7,711 numeric cells.
The selected method may also fall back after a warning. Bounded simplex is directly observed on
a finite-upper-bound fixture and in this 15-case corpus. The corpus screenshots
have been transcribed cell-by-cell with source hashes. Pivot/tie validation,
abnormal termination, the 100×100 size limit, scientific input/output notation
and manual alternate-optimum navigation are documented in their dedicated
screen documents. Hidden numerical thresholds, successful printing and
behavior outside the captured cases remain explicit unknowns.
