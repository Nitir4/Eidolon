# Whole-application recreation plan

The user selected implementation mode and authorized continuous recreation on
12 October 2026, using REA and the installed TORA reference. This is an
implementation checklist, not a claim that the whole application is already
reverse engineered. The current feature cycle is Transportation EditGrid.

This inventory uses existing source, specifications, delivery records, and
saved screenshots. It did not operate the shared live TORA window or inspect
executable code/resources. The live-observation owner supplies new private
captures separately. No algorithm is added merely because a textbook associates
it with a module name.

## How to read the states

- **Delivered:** implemented and independently verified for the documented
  captured profile, with exact-source package evidence. This does not establish
  all possible input behavior.
- **Observed:** visible in saved reference evidence; implementation may be absent.
- **Partially observed:** entry/control exists, but its complete state transitions
  or results need focused probes.
- **Unknown:** the inspected evidence does not answer the question. A main-menu
  label or submenu arrow does not identify the submenu's algorithms.

Historical specifications contain earlier pending/read-only descriptions.
The nested `future_scope.input_editing` record in `docs/STATUS.json` and its
source-pinned audit files establish the newer delivery. Preserve those earlier
records as history rather than silently interpreting them as current failures.

## Current delivered baseline

| Area | Delivered captured profile | Source and evidence |
|---|---|---|
| Native shell | Qt Widgets navigation, readable fonts/colors, scalable layout, remembered geometry and display-change recovery | `src/desktop/application_shell.cpp`; `docs/UI_SPEC.md`; `docs/HARDWARE_ACCEPTANCE.md` |
| LP input/files | New/edit/open/save, native data-entry and structural editing; bounded legacy codec and approved quoted-field recovery | `src/compat/lp_codec.cpp`; `src/desktop/lp_input_controller.cpp`; `docs/tora-analysis/LP_DATA_ENTRY_MODULE.md`; `tests/fixtures/tora-input-edits.tsv` |
| LP numerical/output | All-slack, M-method, Two-phase, Bounded simplex, Dual simplex; Graphical; final summary and sensitivity | `src/lp/`; independent fixtures `all-slack.json`, `m-method.json`, `two-phase.json`, `bounded.json`, `dual.json`, `graphical.json`, `final-solution.json`, `sensitivity.json` under `tests/fixtures/` |
| LP printing | Original input, five tableau methods, graph, final/sensitivity summary and captured direct/default-printer behavior | `src/desktop/lp_printing.cpp`; `docs/tora-analysis/LP_PRINTING.md`; `tests/fixtures/printing.json` |
| Transportation automatic solve | North-west, Least-cost and Vogel starting bases with shared automatic MODI progression and native tableaus | `src/transportation/north_west.cpp`; three `transportation-*.json` numerical fixtures; `docs/tora-analysis/TRANSPORTATION_ITERATIONS.md` and method specs |
| Transportation input/files | New grid, title/aliases/costs/quantities, Open/read-only copy, atomic Save/Save As, dirty-navigation cancellation and byte-preserving default-format writer | `src/compat/transportation_document.cpp`; `src/desktop/transportation_controller.cpp`; `docs/tora-analysis/TRANSPORTATION_EDITING.md`; `tests/fixtures/transportation-input.json` |
| Deployment | Windows portable/installer, Linux AppImage, universal macOS DMG, matching runtime notices and startup checks | `.github/workflows/build.yml`; `tools/package_desktop.py`; `docs/STATUS.json` |

The latest input-editing product source is
`edc34b07a740041b86a719d26b0406fc2f5ece46`, verified in CI run `38286295916`:
88 CTest tests per OS, 264 overall, with zero failures/skips. Its local record
also has 32 Qt-free tests. The exact-source release/install records are
`build/transportation-input-final/{ci-verified,packages-verified,linux-payload-verified,installation,installed-window}.json`.
The installed AppImage SHA-256 is
`ab4060d7f1f1761c4dfb63d87341bfd3f8e5f6d73d0072653f3a9dc4d489c7e8`.
These are a baseline; future feature deliveries require their own source pins.

Only LP and Transportation are registered product modules in
`src/desktop/main.cpp`. LP routes are `input-mode`, `input`, `solve-menu`,
`output-format`, `final`, `graphical`, `simplex`, and `m-value` under `lp/`.
Transportation routes are `input-mode`, `input`, `solve-menu`, `northwest`,
`leastcost`, and `vogel` under `transportation/`. Menu labels for the other
modules do not establish their implementation.

## Original MAIN-menu inventory

Saved original image
`reference/tora-captures/smoke/basic-lp-800/05-installed-main-menu.png`
was visually inspected. The same hierarchy is documented in
`docs/tora-analysis/APPLICATION_SHELL.md` and corroborated by
`reference/tora-captures/methods/lp/corpus/main-menu.png`.

| Original entry, in visible order | Current implementation | Observed detail and next evidence |
|---|---|---|
| Linear Equations | Absent | Label and submenu arrow observed. Enumerate the submenu before naming methods or model formats. |
| Linear Programming | Delivered captured profile | Complete captured solve hierarchy has Graphical/Algebraic and five iteration methods. No Revised simplex or Parametric analysis item is present in that observed hierarchy; do not invent either. |
| Transportation model | Partially delivered | Three automatic starts and editable files delivered. Structural edits, multiplier selection, manual loops, final summary and print behavior remain. |
| Integer programming | Absent | Label observed; entry route, algorithms, input schema, files and output unknown. |
| Network models | Absent | Label and submenu arrow observed; all submenu names and behavior unknown. |
| Project Planning | Absent | Label and submenu arrow observed; all submenu names and behavior unknown. |
| Queuing analysis | Absent | Label observed; model families, parameters and output unknown. |
| Zero-Sum Games | Absent | Label observed; matrix limits, methods, mixed/pure solution behavior and output unknown. |
| EXIT TORA | Native exit/dirty guard exists | Original startup keyboard focus/accelerators and every exit/cancel state are not exhaustively established. |

A bounded search of the current private capture paths found focused LP and
Transportation collections, but no dedicated collections for the other six
modules. This is a search boundary, not evidence about what the original can do.

## Ordered feature cycles

Each numbered cycle follows observed entry → state changes → result, then
independent expected values, tests-first implementation, local/native review,
and exact-source three-OS packaging verification. Reuse completed evidence;
do not rerun old broad LP sweeps merely to fill time. Keep a single owner of the
shared live reference window.

1. **Transportation EditGrid — active.** Establish row/column target selection,
   Copy/Paste/Delete/Insert, insertion placement, clipboard orientation,
   enabled/disabled commands, Undo behavior, dimension/count updates, data and
   lexeme movement, save/reopen, and trace invalidation. New actual menu images
   are under `reference/tora-captures/transportation/editgrid/actions-menu-inventory/`
   as `menu-no-heading.png`, `menu-source-heading.png`, and
   `menu-destination-heading.png`. The reviewed source-heading menu visibly
   lists Undo Last Insert, Paste, Or Delete (`Ctrl+U`, gray in this state), Copy
   Column or Row (`Ctrl+C`), Paste on Column or Row (`Ctrl+P`, gray), Delete
   Column or Row (`Ctrl+D`), and Insert column or Row (`Ctrl+I`).
   `actions-delete-via-menu/after-delete.png` visibly changes the original
   destination count from three to two, removes D1 data, and relabels remaining
   destinations; REA evidence
   `ev_2c7ba94a249c28ca2d3690860b38bfd989df9ac7ee7601dc4ffa513f1f323f69`.
   Full operation semantics remain under investigation. The older filename
   `input-editing/actions-editgrid-start/editgrid-menu.png` shows no expanded
   menu and must not alone establish command availability.
2. **Transportation multiplier initialization.** Observe how `Initialize u or v`
   selects any u/v and value, when it becomes disabled, whether changes alter
   displays or basis/iteration history, and invalid/default commitment. Existing
   evidence: `corpus/01-normal/northwest/initial.png` and the iteration spec.
   Default u1=0 is delivered; alternate initialization is not.
3. **Transportation manual improvement loops.** Observe entering-cell selection,
   loop construction/validation, plus/minus state, cancellation/reset, ties,
   basic-zero behavior, Next/All interaction, notices and completion. Existing
   automatic colored loops do not establish manual click semantics. Begin with
   known multi-iteration `01-normal`, then discriminating `17-leaving-tie` and
   `18-entering-tie`; add probes only where they isolate a missing rule.
4. **Transportation final summary.** Reuse seventeen existing
   `corpus/<case>/final/{initial.png,numbers-draft.json}` records and the contract
   `docs/tora-analysis/TRANSPORTATION_FINAL.md`. Their original JSON retains its historical pending-review status. A later
   independent visual review confirms all 355 populated cells and 85 following
   blank cells; see `TRANSPORTATION_FINAL_REVIEW.md` and the private
   `build/transportation-final-review/` ledger. Resolve general start/basis choice
   with `14-digits` and
   fractional amount formatting with `08-decimal-quantities` and
   `16-dyadic-fractions`. Original final summaries can display rounded shipment
   amounts alongside fractional contributions. Do not silently correct them or
   assume Vogel's optimum basis is the final route's basis.
5. **Transportation printing.** Capture actual printer output for original data,
   each applicable start/trace and final summary; cover aliases, multi-page/wide
   tables, fractions and terminal controls. Existing Write to Printer buttons
   prove availability only. Reuse the isolated printer setup and independent PDF
   acceptance methodology already verified for LP; do not reuse LP expected
   content as Transportation evidence.
6. **Transportation settings and validation profile.** Resolve observable input/
   output notation controls, N/D meaning and allowed extrema, imbalance/zero
   totals, validation messages, dimensions, nonbinary fractional quantities,
   text encoding/quoting, malformed-file outcomes and active-editor boundaries.
   Current engineering limits are 1..100 dimensions, ASCII/default 4000/5/2/1
   codec, 4 MiB, exact single-precision-representable quantities for solving,
   balanced solve inputs, and a 250,000 displayed-cell safeguard. These are
   explicit clone policies, not proved original limits. Do not generalize dummy
   source/destination handling from textbook Transportation conventions.
7. **Complete remaining MAIN hierarchy inventory.** Open each as-yet unobserved
   MAIN entry and capture all visible submenus and input/output choices. Create
   one module ledger per actual observed entry, with file discriminators only
   after authentic saves. This inventory may proceed between delivery waits
   without changing the active implementation cycle or competing for the live
   window. Record unknowns instead of assigning hypothetical variants.
8. **Linear Equations module.** Implement one actually observed submenu method
   per cycle, including model entry, files, numerical output/steps, failure
   messages, navigation and printing. Method names and decomposition details
   remain unknown until cycle 7 establishes them.
9. **Integer programming module.** Follow its actual entry hierarchy and split
   its observed choices into separate input/file, numerical, manual-output and
   print cycles. Reuse LP infrastructure only where the original contract agrees;
   no branch-and-bound/cutting-plane claim is established by the label alone.
10. **Network models module.** Create cycles from its actual submenu inventory.
    Graph editing, route/flow/tree types, balances and tie rules are unknown;
    none is a required algorithm until directly observed.
11. **Project Planning module.** Establish observed submenu methods, network/data
    entry, calculation steps, graphical/tabular output and files. Do not assume
    CPM/PERT variants from the menu title.
12. **Queuing analysis module.** Establish actually offered model families,
    parameter validation, stability behavior, notation, results and printing.
    Do not assume M/M/1 or other distributions from the title.
13. **Zero-Sum Games module.** Establish its actual matrix-entry and solution
    choices, dimensions, degeneracy/terminal handling, step output, files and
    printing. Do not assume dominance reduction or LP routing without evidence.
14. **Whole-app integration and remaining captured-profile gaps.** Check cross-
    module file errors, dirty-state ownership, reopen and trace invalidation,
    shortcuts/focus, route exits, screen scaling, printing and deployment after
    all observed modules are implemented. Address material open questions in
    `docs/OPEN_QUESTIONS.md` with focused discriminators, especially formatting
    extremes, encoding, near ties and safeguard behavior. Physical printers,
    actual mixed-monitor hardware and trusted signing require their real
    environments; retain untested labels when unavailable.

## Evidence and acceptance discipline

Private raw material stays under ignored `reference/`; transient drivers,
process reports and downloaded CI reports stay under ignored `build/`; release
packages stay under ignored `dist/`. Commit independent transcriptions,
requirements, source and meaningful tests, never original executables, help,
icons or raw reference screenshots/files.

For each feature, record target/version, inputs and actions, observed state,
REA Evidence IDs, screenshot/saved-file SHA-256, incomplete coverage and the
implementation decision separately. A successful REA observer capture does not
alone prove the child workflow succeeded; check its exit/result and selected
artifacts. A runtime-control screenshot does not prove a printer produced output.
A numerical agreement does not prove the original's internal arithmetic.

Keep model/solver code Qt-free, file compatibility separate, and native Qt
controllers/presenters consistent with existing source structure. New modules
must register actual routes rather than enabling labels that lead nowhere.
Use observed strings/order/format for golden comparisons; broaden tolerances
only with new independent evidence, never to conceal a mismatch.

Whole-application completion requires an explicitly reviewed reachable-menu
inventory, implemented and verified behavior for every identified entry, working
new/open/edit/save/solve/manual/print/navigation paths where offered, and a final
exact-source three-OS release gate. Passing current LP/Transportation tests is
not evidence that the other modules are complete. Unknown or unavailable states
must remain visible in the ledger rather than being renamed done.
