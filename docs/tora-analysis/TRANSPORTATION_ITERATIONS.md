# Transportation iteration views

Status: seventeen complete numerical profiles and seventy-six observed tableaus
across three starts. The native North-west golden contract covers all thirty-eight
North-west tableaus; other starting algorithms remain observation only.

## Layout and navigation

Heading: TRANSPORTATION TABLEAU followed by the selected starting method.
Title includes “minimum cost”. Initialize u or v defaults to u1=0. Controls:
Next Iteration, All Iterations, Write to Printer, View/Modify Input Data,
MAIN Menu, Exit TORA. Initialization is disabled after the first improvement.
Next appends a tableau beneath the previous one; vertical scrollbar arrows
expose the stack. Mouse-wheel events did not scroll the observed legacy grid.

Table labels include Iter, ObjVal, Name, D1…Dn, Supply, S1…Sm, Demand,
u1…um and v1…vn. A source block shows costs, shipments, and reduced costs.
The default initialized multiplier is red. An improvement loop uses green
for entering, red for leaving and gray for other selected loop cells.
Costs, multipliers, reduced costs and objective have two decimals by default;
integer shipments show integer strings. Nonbasic shipment cells are blank.

At optimality the pilot shows an “Optimal Tableau” dialog:
“Iteration 3 is optimal”. Next/All Iterations then become disabled.
Least-cost/Vogel start optimal and already have disabled iteration controls.

## Reviewed North-west trace

Input: costs [[2,3,1],[5,4,8]], supply [20,30], demand [10,25,15]. Blank
entries below mean visibly empty shipment cells, not a displayed zero.

| Iter | Objective | Shipments S1 | Shipments S2 | u | v |
| --- | --- | --- | --- | --- | --- |
| 1 | 230.00 | 10,10,blank | blank,15,15 | 0.00,1.00 | 2.00,3.00,7.00 |
| 2 | 170.00 | 10,blank,10 | blank,25,5 | 0.00,7.00 | 2.00,-3.00,1.00 |
| 3 | 150.00 | 5,blank,15 | 5,25,blank | 0.00,3.00 | 2.00,1.00,1.00 |

Reduced-cost rows are respectively:
[[0,0,6],[-2,0,0]], [[0,-6,0],[4,0,0]], [[0,-2,0],[0,0,-4]],
each displayed with two decimals. These agree with u+v−cost in this case.
That agreement is an inference from observed numbers, not proof of every
algorithm/termination rule. Least-cost and Vogel show the third row's
shipments/multipliers/reduced costs as their Iter 1.

## Isolated automatic North-west rules

North-west allocates the smaller remaining supply/demand. Equal exhaustion
advances the source first, retaining a basic zero in the next source and same
destination. Explicit zero shipments and blank nonbasic cells differ.
The basis forms a source/destination tree. Starting with u1=0, basic cells
satisfy u+v=cost. Reduced costs are u+v−cost; the largest positive nonbasic
value enters. Tied entering and minimum-shipment leaving cells are chosen in
source-major, then destination order. The unique alternating tree loop is
updated by its minimum negative-position shipment. The observed terminal
condition is no positive reduced cost in the captured cases.

Case 17, costs [[7,0,8],[9,5,3],[9,0,5]], supply [2,3,8], demand [6,4,3],
discriminates row-major leaving ties from loop-path order. The original keeps
S3D3 as a zero basic cell at the terminal tableau. Case 18, costs
[[6,6,2],[1,5,5],[7,1,3]], supply [2,9,3], demand [1,8,5], discriminates
row-major entering ties from column-major scanning. Both discriminators are
included in the fixture. The independent prototype was used only to isolate
rules; expected fixture values come from screenshot transcription.

The product uses explicit native cycle/1000-iteration and size safeguards,
and a 1e-9 improvement threshold. These are engineering policies, not verified
TORA limits. The captured profile contains integer and dyadic quantities.
Non-exact single-precision supply/demand are rejected pending a complete
contract for the clipped long-fraction display. Costs/objective/multipliers
use shared decimal N=5,D=2 formatting; basic shipments use short strings.

## Native layout and controls

The output retains source blocks, cost/shipment/reduced-cost row order,
ObjVal =, Name, supply, demand, u/v, Iter labels and stacked iterations.
Next/All append tableaus; terminal controls are disabled and advancing to the
last tableau shows “Iteration N is optimal”. Automatic loop colors are applied
to earlier displayed tableaus. Readability colors/fonts follow the user's
approved LP accessibility repair rather than the original saturated background.
The layout expands and scrolls using native font metrics, without an 800×600
raster UI. Default u1=0 is displayed; changing initialization/manual loop
selection and printing are outside this initial preview.

## Unknowns / things I could not verify

Least-cost/Vogel penalty/tie policies; manual loop validation; other multiplier
initializations; imbalance; cycling; exact near-zero improvement thresholds;
nonbinary fraction strings; scientific settings; printer layout and
horizontal pagination. Passing this captured profile does not verify those.

A Windows-only event ordering in the first matrix run exposed a native column
resize bug on model reload. A deterministic viewport-event regression now
checks it on every OS; distribution is bounded by the columns currently
present. This is a presentation fix and does not alter numerical traces.
