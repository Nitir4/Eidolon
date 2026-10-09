# Graphical LP solution

## Purpose, eligibility and inputs

`Solve Problem → Graphical` visualizes a two-variable LP after the common output-format screen. Input validation, relation entry, the 100-variable/100-constraint limits and default nonnegative variables belong to the input module. Exactly two variables enable this menu item; one and three variables leave it gray. Explicit finite upper bounds, nonzero lower bounds or unrestricted variables produce title `Graphical LP` and message `LPs with unrestricted vars and/or explicit upper/lower bounds are not supported by graphical method. Click OK then click screen.` No graph or format screen is opened for those inputs. Bounds expressed as ordinary constraint rows are supported.

The original fifteen graphical studies are reused. Twenty focused numerical/display cases and separate manual/eligibility observations resolve gaps without repeating the original sweep. Private `reference/tora-captures/graphical/<case>/` links the accepted TORA-saved file, input/format/initial/one-stroke/zoom screenshots and `transcribed.json`. Canonical original saves come from `roundtrips/`, not historical `.tora-installed` scratch files. All 35 accepted files passed byte-identical codec round trips; 325 source hashes and 35 graph-image hashes were checked. Independently transcribed CI fixtures are `tests/fixtures/graphical.json`.

## Screens, navigation and interaction

The heading is `GRAPHICAL LINEAR PROGRAMMING SOLUTION`. The left cyan model panel contains the title, objective, `subject to`, numbered constraints, `all xj >= 0` and `Click here to graph LP in one stroke`. Instruction text is `To graph the LP below, click constraints one at a time, then click objective function`. `Optimal Solution` appears below. A square cyan graph occupies the right side; controls above are `Zoom In`, `Zoom Out`, `Print Graph`. The footer is `View/Modify Input Data`, `MAIN Menu`, `Exit TORA`.

On the initial graph, the nonnegative quadrant is dotted and no result is shown. Clicking a constraint adds its red boundary and intersects the dotted region with that constraint. Repeating a constraint does not toggle it off. Clicking the objective before all constraints raises title `Graph Obj Function`, body `Must graph all constraints prior to objective function`. Once all are drawn, the objective displays the solution. Clicking the sign row resets the graph and result; one stroke draws all constraints and the objective. `graph-manual-stable/` captures every intermediate state. Earlier quick `graph-manual/` blank frames were timing artifacts, not accepted behavior.

Back navigation preserves the input document; editing/opening input invalidates the graph session. Shared output-format dispatch explicitly identifies Graphical or the requested simplex method. Mouse row selection follows the reference; Qt item activation also supports keyboard Return. No unobserved TORA-specific graphical shortcut is claimed.

## Result numbers, labels and terminal states

The result panel reads `Objective value = ...`, `x1 = ...`, `x2 = ...`. Decimal and scientific values use the approved common formatter. Scientific N=5/D=2 shows `1.00000e+01`, `2.00000e+00`, `2.00000e+00`; axis ticks remain ordinary integers. Variable aliases do not replace x1/x2 in this screen. Mixed-sign expressions retain a plus before a negative coefficient.

Normal gives z=10.00 at (2.00,2.00). Alternative optima case 05 selects (4.00,0.00), z=4.00. Equality case 06 gives (3.00,1.00), z=11.00, with no two-dimensional dotted area. Big-M/two-phase test problems are graph inputs, not selectable graph algorithms. The clone obtains the final point through the approved bound-aware solver; the 35 TORA result comparisons verify this choice for these inputs. TORA's internal graphical solver is not known.

Infeasible displays `Problem has no feasible solution` in the panel and graph. Unbounded displays `Optimum solution is unbounded` in the panel and `Optimum solution is UNBOUNDED` on the graph. Both keep the observed blue origin marker; neither draws a finite objective contour. The marker is a legacy status location, not a feasible/optimal point.

## Captured geometry and numerical profile

Constraint lines are red, axes/objective/markers/text blue, constraint tags yellow, graph background cyan and feasibility dots gray RGB(128,128,128). Original dots are about 2×3 pixels. Objective drawing precedes red constraints, so coincident constraint segments remain red. A matching blue status-text pixel must not be mistaken for an axis hidden by a red zero-boundary constraint.

The following observed display profile matches the tested intercepts 0, 0.2, 1, 2, 2.5, 2.75, 3.5, 4, 5, 10, 15, 19, 20, 25, 30, 40, 50, 60, 80, 100, 200 and 1000. It is an inferred compatible profile, not extracted original code. Let L be the largest positive finite RHS/coefficient intercept, converted to an integer with ties to even; a converted zero uses L=5. The margin unit U is max(1, round-half-away(L/10)). The minimum is -U and initial maximum H=max(L+1,floor(1.1L)). Dot spacing is U times 0.15 for L<20, 0.2 for L<30, 0.25 for L<40, 0.3 for L<60, 0.35 for L<80, 0.4 for L<100, else 0.5.

The label-capacity profile is U×max(1,ceil((H+U)/U×(digits(H)+2)/60)). Zoom In converts 0.8H with ties to even; Zoom Out converts 1.25H. H<3 and unchanged results are refused. Minimum and dot lattice remain unchanged. Examples: 5→4→5; 3→3→4; 2→2→2; 7→6→8; 27→22→28; 33→26→32; 110→88→110; 1100→880→1100. Earlier one-unit zoom speculation is superseded. Source line/axis positions, not native solver output, determine these expectations.

`tools/audit_tora_graphical_geometry.py` verifies source SHA-256 and annotates 755 actual color pixels across 35 captures. The coincident case-12 line has no isolated red sample and is explicitly recorded as an overlap. Native tests compare normalized positions with a small neighborhood for source quantization, stroke widths and font changes; these are geometric/color checks, not bit-identical cross-platform font rasterization.

## Native implementation, files and printing

`GraphicalSession` is Qt-free; `LpGraphicalController` handles menus, state and notices; `GraphPlot` draws through QPainter. Layouts and font metrics expand the screen and preserve a square plot at native DPI. At a small logical viewport the shell scrolls content rather than shrinking fonts. Tests assert actual requested sizes for 1366×768, 1920×1080 and 3840×2160 at 100/125/150/200% using a large offscreen test monitor. The monitor fixture is test-only; it imposes no product window resolution.

The existing TORA `.txt` codec and Open/Save/New flow are unchanged. Graph display/zoom does not mutate input or serialize a bitmap. `Print Graph` uses the native print dialog and renders the model, current plot and results with QPrinter, including PDF. Native PDF generation and unchanged graph state are tested. TORA graph printing raises VB runtime error 482 under the available Wine setup and exits after acknowledgment; successful original print layout is not verified. The clone does not reproduce that environment crash.

The numerical viewport safety limit is 1e12 and zoom limits the lattice span to 2048 steps; these are explicit native safety policies outside the captured display range. They are not claims about original TORA limits.

## Unknowns / things I could not verify

Successful original printer output, pagination and print-dialog defaults remain Q-08. Physical mixed-DPI monitor moves/hotplug still require hardware acceptance. Hidden graphical solver implementation, tie choices outside the captured cases, internal range/tick thresholds outside the sampled intercepts, extreme/ill-conditioned numerical cases and unobserved shortcuts are not claimed as verified. Final solution/sensitivity remain separate implementation gates. No asserted comparison is knowingly accepted with a mismatch.
