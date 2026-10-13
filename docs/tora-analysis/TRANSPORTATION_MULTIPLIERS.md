# Transportation multiplier initialization

Status: implemented locally; verification and delivery are tracked in STATUS.json.
EditGrid remains the installed release until this cycle completes its own gate.

## Observed behavior

The original iteration screen offers one zero anchor for every source and
destination: u1=0 through um=0, then v1=0 through vn=0. It offers fixed zero
choices, not an arbitrary numeric editor. Default solving starts with u1=0.

Choosing an anchor recomputes multipliers without changing shipments, objective,
reduced-cost strings or the observed automatic improvement route. The selected
multiplier is highlighted. Refreshing the first tableau changes its multiplier
strings from two decimals to short numeric strings, even when reselecting u1=0.
Later tableaus use normal two-decimal strings and keep the same zero anchor.
Once an improvement is shown, initialization is disabled. An already-optimal
initial tableau still permits initialization, despite disabled Next/All controls.

The pilot starts with costs [[2,3,1],[5,4,8]], supplies [20,30] and demands
[10,25,15]. Its first objective remains 230.00 for every anchor:

| Anchor | u strings after initialization | v strings after initialization |
| --- | --- | --- |
| u1=0 reselected | u1=0, u2=1 | v1=2, v2=3, v3=7 |
| u2=0 | u1=-1, u2=0 | v1=3, v2=4, v3=8 |
| v1=0 | u1=2, u2=3 | v1=0, v2=1, v3=5 |
| v2=0 | u1=3, u2=4 | v1=-1, v2=0, v3=4 |
| v3=0 | u1=7, u2=8 | v1=-5, v2=-4, v3=0 |

With v3=0, Iteration 2 shows u1=1.00, u2=8.00 and
v1=1.00, v2=-4.00, v3=0.00; objective 170.00. Iteration 3 shows
u1=1.00, u2=4.00 and v1=1.00, v2=0.00, v3=0.00; objective 150.00.
The initializer stays disabled after Next. Previously refreshed first-tableau
strings remain short within the stacked history.

The already-optimal Least-cost pilot permits u2=0 and shows u1=-3, u2=0,
v1=5, v2=4, v3=4. Its objective remains 150.00 and Next/All remain disabled.
The fractional-cost case 08 with v1=0 shows u1=2.25, u2=3, v1=0, v2=1.25,
v3=5.5 and objective 29.19. Case 17's three-source North-west start with
u3=0 shows u1=-2, u2=0, u3=0, v1=9, v2=0, v3=5; objective 65.00.

## Evidence

Private original images and REA process records are under
reference/tora-captures/transportation/multipliers/. Every accepted capture
below reports child exit zero. Public tests contain independent transcriptions,
screenshot hashes and evidence IDs; no original screenshots are shipped.

- Initial dropdown: ev_026e0a950889992324936e7f22454179eff3cc11a5323c9f52dc319cec832985.
  Its first screenshot has incomplete repaint; the expanded dropdown is complete.
- Four nondefault pilot anchors:
  ev_5e35ab218f77a57c411b2154d991becd3b4e034ff5fe55c1ba53df113cd66e74.
- u1 reselection, v3 Next and complete Iteration 2:
  ev_21ac9390564bcb99ce17af2ccb8eed73ca99c7dbd42b6adeed100c53f2ae39d5.
- v3 terminal/disabled control:
  ev_1d8dc3aae3513b904e44e2e5a41f4bdd1b975500858f78674d490750dd0573cc.
- Already-optimal dropdown:
  ev_4d3ad0b1a721de0abeadd34fa7fa0fc8a5b51d2b1f93d81cf9673713223e96a3.
- Optimal u2 and fractional v1:
  ev_0df011568d976d78f25ab2e71f32f25e587ce29c66fbd09b02fe3ba1b244b10a.
- Three-source dropdown/u3:
  ev_8c06a3d62754c97cd1250576ede89c68a7180df67d0eeeb259c52ec121a98031.

## Design and execution flow

The Qt-free numerical API defines MultiplierAxis and MultiplierAnchor.
initializeMultipliers returns a new tableau after validating the selected
index and the complete basic tree. It traverses every nonblank shipment edge,
including zero basic shipments, starting at the selected zero node. Across
a basic edge, the unknown multiplier is cost minus the known multiplier.
All nodes must be visited and the basis must have m+n-1 edges. Invalid axes,
indices, dimensions, disconnected/cyclic bases or nonfinite arithmetic fail
before replacing any state.

This calculation changes only u/v. Shipment amounts, objective, reduced-cost
records and the selected pivot remain the accepted existing solution trace.
For the observed bases, new u/v sums reproduce the same basic equations and
reduced values. This is a presentation initialization of a solved trace;
it does not broaden the solver's accepted numerical profile.

TransportationController validates a copied whole trace, then commits the
anchor/refreshed flag and emits multipliersChanged. Input bytes and dirty state
stay unchanged. New/open/reinitialize/input edits and successful fresh solves
reset the default anchor. Failed initialization preserves the old trace/anchor.
The view locks initialization when Next or All reveals an improvement, keeping
the same selected anchor throughout that trace.

The native dropdown lists all sources followed by destinations. User activation
also handles same-choice reselection. The first initialized tableau uses short
strings; subsequent tableaus use existing decimal formatting. Dark red text
marks the selected anchor against the approved readable palette. Native Qt
layouts, font metrics and scrolling handle the existing scale/resolution range.

## Verification and limits

Core and native tests first failed on absent APIs. The independent probe golden
also first failed on its missing --anchor option. Private logs are in
build/transportation-multipliers/.

Eight independent scenarios compare ten observed tableaus and 311 exact fields.
They cover all pilot anchors, reselection, later/terminal persistence,
immediate optimality, fractional costs and a three-source model. Additional
core safety tests check every anchor, malformed bases, input immutability and
zero basic edges. Native tests check real keyboard activation, dropdown order,
first/later strings, selected-only highlighting, lock/reset, failed changes,
input/dirty preservation and twelve renders across three sizes/four scales.

The focused local gate passes six invocations, including all four native scales.
Complete local/native/three-OS/package/install results belong to STATUS.json.
Arbitrary nonzero anchors are not an observed feature. Broader short-number
precision, settings, extreme cancellation and legacy encoding remain separate
profile investigations. Physical mixed-DPI handoff and printer hardware remain
unverified. Manual loops, final summary, printing and remaining modules are
still separate feature cycles.

Local complete gate: 91/91 desktop CTests in 157.82 seconds; 35/35 Qt-free CTests in 3.39 seconds. Actual Wayland acceptance passed 23 views with LP preserved. Selected anchor/Next views and small 200% render were visually inspected. Three-OS/package/install gate remains pending.
