# Graphical LP solution

## Purpose, eligibility and inputs

`Solve Problem → Graphical` provides a visual two-variable solution after the
common output-format screen. All fifteen two-variable models were selected
successfully. The three-variable probe has a gray disabled Graphical item;
selecting it leaves the menu open, rather than showing an error dialog.
Evidence: `probes/validation-20261004/graph-three-vars/after-selection.png`.
Other dimension/bound/sign eligibility rules remain unknown. Model fields,
relations and the 100×100 input limit are documented in `LP_INPUT.md`.

## Layout and drawing directly observed

The heading is `GRAPHICAL LINEAR PROGRAMMING SOLUTION`. A left cyan panel
contains the title, objective, numbered constraints and sign restrictions.
It says `To graph the LP below, click constraints one at a time, then click
objective function`. `Click here to graph LP in one stroke` is a separate
selectable row. Below is an `Optimal Solution` panel. A square cyan graph is
on the right, with `Zoom In`, `Zoom Out`, `Print Graph` above. Lower controls
are `View/Modify Input Data`, `MAIN Menu`, `Exit TORA`.

One stroke draws red constraint lines, numbered labels on yellow backgrounds,
blue coordinate labels, brown/red feasible-region dots, a thick blue objective
line marked `Obj`, and a blue optimum marker marked `Optimum`. Normal shows
objective 10.00 and x1/x2=2.00/2.00. Case 05 plots (4.00,0.00), objective
4.00, rather than a second optimum. Its marker label clips at the graph edge.
The mixed-sign expression preserves a plus before a negative coefficient.

All fifteen studies have input, output format, initial graph, one-stroke,
Zoom In and Zoom Out screenshots in `output-studies/<case>/graphical/`.
An initial plot can contain dense dots (case 09). Earlier blank captures
caused by clicking in the output-format screen were superseded.

Case 03 draws incompatible constraints, no feasible dots, and displays
`Problem has no feasible solution` in the result panel and graph. Case 04
shows `Optimum solution is unbounded` in the panel and `Optimum solution is
UNBOUNDED` on the graph. Its blue marker is at the origin despite unboundedness;
it must not be interpreted as a finite optimum. Case 08 displays (3.00,2.00),
objective 5.00. Graph result fields are separately transcribed in each
`transcribed-graph-result.json` once visually reviewed.

## Zoom and numerical interpretation

Two successive Zoom In clicks on Normal show horizontal ranges approximately
-1…5, -1…4, -1…3 and unit spacings about 66, 79, 99 captured pixels.
Two Zoom Out clicks record the return. This supports a one-unit span change
in this example, not a universal multiplicative zoom rule. Evidence:
`probes/targeted-20261004/graph-zoom/`. Zoom preserves result values.
Oracle pixel measurements describe the legacy screen; Qt drawing must use
layout/font metrics and scale with the viewport.

## Unknowns / things I could not verify

Constraint-by-constraint sequencing, general range/tick rounding, exact dot
placement, scientific plot labels, graph-print output, and bound/unrestricted
eligibility remain unverified. Geometry can be inferred from observed lines
and input equations, but TORA's internal graph algorithm cannot be claimed
from screenshots. The graphical-module gate must compare native drawing to
these captured inputs/states.
