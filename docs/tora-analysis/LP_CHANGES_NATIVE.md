# Native LP class-extension integration

Implemented and locally verified workflow, 16 October 2026. Class-extension UI is new design; it does not
claim an original TORA menu. Reuses existing LP input, file codec and Qt shell.

From SOLVE Menu choose Class extensions -> Change Cj / RHS or Dual Revised (RHS).
Snapshot the validated current document. Edit replacement values in draft tables;
full binary64 precision is preserved. Revised mode makes objective cells read-only.
Analyze via analyzeChanges or reoptimizeDualRevisedRhs respectively. Fresh Cj/B
shows before/changed solutions and retained-basis feasibility diagnosis. Revised
shows original-variable results and a navigable canonical basis/dual/reduced-cost
trace. Terminal infeasible/unbounded/limit outcomes have no fabricated optimum.

Analyzing does not alter input. Use changed model explicitly applies Cj/RHS to
the existing input document, preserves its path/read-only flag and enables its
normal dirty/save flow. Navigation Back/Input/Main can leave a draft unapplied.
Input changes invalidate the baseline/results. Failed calculations clear stale
results so old output cannot be applied/printed as if current.

Screen output format controls select decimal/scientific N/D without changing
calculations. Printing includes baseline/changed data, diagnosis, restored
solution and the selected trace information; must handle wide/long tables.

Validation performed: keyboard edits, precision/save/reopen, cancel and stale-result
invalidation, min/max original-equivalent examples, terminal/error cases, real
retained-basis traces, printing text/pages, four scales and small/large windows.

## Implemented source and execution flow

- `lp_screens.cpp`: adds a Class extensions sibling beside Solve Problem and
  registers lp/changes and lp/changes-result. Original method submenu is retained.
- `lp_changes_controller.hpp/.cpp`: snapshots the validated document after successful
  navigation, analyzes independent replacements, invalidates stale results and
  applies changes only on explicit user action. Failed navigation preserves state.
- `lp_changes_screens.cpp`: strict finite-number draft tables; decimal/scientific
  N/D controls; baseline/changed values, contributions and constraint gaps;
  selected trace, pivot feedback and revised infeasibility-witness weights.
- `lp_input_controller.cpp`: transactional applyCoefficientChanges copies the
  document, changes only Cj/RHS, validates through the codec, then refreshes input.
  Source/read-only/saved checkpoints and unrelated mathematical data are preserved.
- `desktop_lp_changes.cpp`: keyboard workflow, exact precision/save/reopen,
  cancellation, stale/error/terminal outcomes, min/max equivalent original fields,
  100-variable trace/printing and layout tests at four scales.

Revised baseline results are recovered using the retained initial basis on the
unchanged canonical model, with a zero-pivot revised check. This does not perform
a fresh changed-model primal solve. Fresh Cj/B explicitly uses its existing
analyzeChanges backend. In both cases the document is unchanged during analysis.

Trace costs/prices belong to canonical minimization; shifted/free variables are
labeled accordingly. The overall Changed model status refers to the terminal
outcome even when the selected trace step has negative basic values. Trace spinbox
is one-based. The final result footer uses a two-row grid for narrow high-DPI tiles.
Small windows can use the existing shell scroll area; wide traces scroll internally.

Printing uses native QTextDocument/QPrinter, escapes title text, wraps long values
and prints the selected canonical step. Wide traces label each numeric value by
its column so cross-page headers are unnecessary. Printing does not advance or
mutate the calculation. Physical printer/device failures and other OSes require
separate acceptance; a generated PDF is not that broader claim.

## Evidence and limitations

Two previously REA-captured equivalent LPs supply 16 directly reviewed display
fields: max value12 at(2,3), min value14 at(7,0), including contributions/gaps.
Full REA IDs and authenticated file hashes are in tests/fixtures/dual-revised.json
and LP_DUAL_REVISED.md. Native tests compare these mathematical fields; neither
native class-extension design nor its iteration count is claimed as an original
TORA screen. No new original runtime or binary analysis was needed for this slice.

Observed initial native gate:121/121desktop in100.46s; headless51/51in5.24s.
Subsequent presentation refinements and narrow-footer fix have their own final
logs in build/lp-changes/accepted-*.log. Current final counts are recorded in
STATUS.json/handover after those checks finish. Earlier screenshots/PDFs remain
preserved separately from final accepted runs.

Backend numerical limits and dependent-equality bootstrap restrictions continue
to apply. An invalid/unbounded baseline produces a notice and no stale result.
Drafts are kept only in the live workflow; saving an unapplied draft/trace as a
separate analysis session is not implemented. Apply changes plus normal Save As
persists the changed LP. New native controls are local source, not an installed
three-OS release. Assignment/Games native integration and the report remain.


## Final accepted local gate

- Desktop CTest:121/121in103.16s; headless:51/51in5.24s.
- Local Release native four-scale suite:4/4in3.89s.
- Actual Wayland:7/7at100%in4.233s and7/7at200%in4.475s (Qt setup/cleanup included).
- Final captures:24PNGs and4PDFs in build/lp-changes/native-accepted-{1,2}/.
  Representative draft/result/initial+final trace/compact views reviewed; first and
  last PDF pages reviewed. PDF machine audit:38pages,8016words, no blank pages or
  word boxes beyond page bounds. Not every screenshot/page is claimed as manually reviewed.
-100-variable wide print is17pages; selected trace numbers retain column labels.
- Final source build emits no new compiler warnings. An earlier full rebuild exposed
  two pre-existing missing aggregate beforeLeave initializers in desktop_shell test
  fixtures; they were recorded, not silently treated as extension defects.
- Final git diff --check and receipt/documentation validation recorded at closeout.

Receipt/logs:build/lp-changes/verification.json,accepted-dev-{build,tests}.log,
accepted-release-{build,tests}.log,native-accepted-{1,2}.log,pdf-audit.json.
No commit,push,GitHub Actions,three-OSpackage,physical-printer or installed-build claim.
