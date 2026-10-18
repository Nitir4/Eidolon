# Full TORA application scope — 16 October 2026

The latest explicit user request is to implement the entire original TORA
application in the clone while continuing REA. This supersedes the earlier
four-module scope and optional/deferred designations. Implementation mode persists.

Required original families: Linear Equations (solution/inversion), LP, Transportation,
Integer Programming, Networks (MST/shortest route/max flow), Project Planning
(CPM/PERT), Queuing (M/M/c with finite/infinite system/source limits), Zero-Sum Games.
Assignment and Cj/RHS/Dual Revised remain required class extensions already authorized.
Common functionality includes native editing, files, validation, calculations,
traces/results, formatting, graph operations where present, printing and hosting
as a native C++/Qt desktop application. Original-specific claims require evidence.

Current verified foundation: LP and Transportation observed workflows, native LP
extensions; mathematical Assignment/Games/Dual Revised backends. New full-scope
work is in progress: Assignment/Games/Equations native workflows; Queuing backend/
native integration; graph algorithms; Integer branch-and-bound; CPM/PERT inventory.
A module is not complete merely because its menu is enabled. Documentation/status
records implemented, tested, observed and still-unknown pieces separately.

GitHub Actions remain disabled. No indirect trigger, re-enable or execution is
authorized by the full-application scope. Local checks continue. All existing
uncommitted work is preserved. No new packages/installation are claimed yet.

---

# Earlier scope history (superseded by the full-application request)

# LPP project scope — 16 October 2026

Implementation mode. Backend calculation correctness comes before additional UI/UX work.
This replaces the former plan to recreate every original main-menu module.

| Requirement | Built and tested | Work left |
|---|---|---|
| Linear Programming | Graphical; all-slack/simplex, Big-M, Two-phase, bounded simplex, Dual simplex; final solution/static sensitivity; Cj/B calculation backend with old-basis diagnosis and fresh re-solve; Dual Revised standard-form/LP adapter and true RHS warm solve; native Cj/B/Dual Revised drafts/results/traces/apply/files/selected-step printing; native input/files/printing | Extension cross-platform packages/physical-printer acceptance; report |
| Transportation | North-west, Least-cost, Vogel, MODI iterations, manual-loop feedback, u/v anchors, editable files, structural Undo, summary and printing | Observed profile implemented; settings pass local checks. Cross-platform settings/package validation and broader numerical profiles remain |
| Assignment | Qt-free min/max Hungarian backend; rectangular models;600exhaustive examples and dual certificates; two actual TORA equivalent-model examples | Frontend/files/integration/package validation; broader numerical profiles |
| Zero-Sum Games | Pure/mixed LP-based and Graphical envelope backends; payoff certificates; row-player LP; two LP-based examples/52fields and three Graphical examples/30fields | UI/files/plotting/zoom/printing/integration/package; broader profiles |
| Linear Equations | Optional; no backend | Implement only after required modules if time/scope permits |
| Report | Historical study resources and evidence exist | LaTeX report: case-wise mathematical models, two examples per case, actual TORA verification and limitations |

Integer Programming, Network Models, Project Planning and Queuing are outside required completion.
Existing LP/Transportation compatibility is bounded by documented captured profiles, not all possible inputs.

## Current verification

Cj/B backend:100independent vertex-enumerated models;four original changes/44fields.
Games:200independent2x2formula checks,100general certificate/invariance cases;
two actual TORA LP-based examples/52fields.
Graphical:300seeded envelope cases; three readable original examples/30fields.
Column-player upper-envelope mode is a mathematical extension; original3x2Graphical is disabled.

Current integrated suites: 121/121 desktop CTests in 103.16s; 51/51 Qt-free in 5.24s.
Native LP extensions: Release 4/4 in 3.89s; actual Wayland at100%/200% passes;
24screen captures,4PDFs/38pages/8016words with no out-of-page text boxes.
Dual Revised: 300 independent standard-form models and 150 bounded RHS changes;
two new REA-captured original examples / 16 summary fields; focused Release 3/3 in 0.22s.
Dependent equality bootstrap requires a reduced model; numerical scaling limits remain explicit.
Focused Release8/8in0.46s includes Graphical backend/original-field checks.
Transportation settings alone earlier passed106/106and40/40.
Three authentic nondefault-format roundtrips and159initial/final display fields match.
Printing:14workflows,19jobs,34pages,356ordered rows,1444teaching tokens match.
Actual Wayland:31views at100% and200%, including scientific initialization/save/reopen/output.
Settings are local source, not an installed three-OS release.
Installed Linux release remainsdc853b4(printing); full Windows printing gate previously timed out.

GitHub Actions are disabled. Specific explicit approval is required before any execution,
re-enabling or indirect trigger. Local builds/tests need no additional permission.

## Backend completion method

1. Specify supported mathematical input and failure cases.
2. Write independent expected solutions and mathematical optimality/feasibility checks.
3. Implement Qt-free calculation with explicit numerical limits.
4. Compare examples with the original TORA where available; use equivalent LP/Transportation
   formulations for class extensions. Record evidence honestly.
5. Only then wire required native controls and improve presentation.

REA: reuse authenticated evidence and one live analysis session; use targeted strings/xrefs
and focused routines. Save a retained snapshot on completion. Keep raw original material private.
Latest session closed successfully. Games snapshot retains15Evidence records;
large handler decompilation failed at an unresolved constructor, with no identical retry.
Task-owned8GiB swap was disabled and its file removed; no persistent settings changed.
