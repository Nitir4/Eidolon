# Eidolon playtest guide

Thanks for testing. Extract the whole ZIP into a normal writable folder first.
Keep the files together; do not run the application from inside the ZIP viewer.
The version and source commit are in `release-info.json`.
Read `build-validation.md` for the actual checks and remaining platform limits.

## 1. Start here

**Windows x64:** double-click `Start-Eidolon.cmd`, or open `app/Eidolon.exe`.
The portable copy needs no administrator account. Native CI builds may also
include an optional installer in `app/setup/`; local cross-builds use the portable copy.

**macOS 13.3+, Intel or Apple Silicon:** double-click `Start-Eidolon.command`.
This ZIP either contains `app/Eidolon.app` directly (copy it to Applications), or
`app/Eidolon.dmg` (open it and drag Eidolon into Applications). Launch that copy.
This playtest build is not notarized. If macOS blocks it, inspect the download
and use Apple's **System Settings > Privacy & Security > Open Anyway** procedure
for this app. Do not disable Gatekeeper globally. If a `.command` file opens as
text, open Terminal in the extracted folder and run `sh Start-Eidolon.command`.

Use your own original TORA installation for comparison. TORA is a Windows
program; macOS testers can compare through their established Windows VM/Wine
setup or coordinate with a Windows tester. A TORA executable is not included.
Failure to launch TORA through Wine is separate from an Eidolon solver failure.

## 2. Run the supplied checks

- Windows: double-click `Run-Checks.cmd`.
- macOS: double-click `Run-Checks.command`, or run `sh Run-Checks.command`.

These scripts verify the shipped files, start the welcome screen, MAIN and all
nine module entry screens, then save their results under `results/`. A screen
should open and immediately close. The scripts also create your editable
`results/comparison.csv` if it does not already exist; they never replace it.

The automatic checks establish integrity and startup only. They do not operate
TORA, enter every problem, certify a printed answer, or mark manual checks passed.
On macOS the check uses the bundled app directly, or mounts this bundle's DMG
read-only and detaches its own temporary mount afterward. Your installed app
and problem files are untouched. Check `release-info.json` for the build route
and actual validation: a Linux cross-build has not been executed on macOS.

## 3. Compare all 42 workflows

`problems/README.md` lists every workflow and at least five examples for each.
There are 126 distinct problems reused across methods, giving 210 workflow/case
combinations. `comparison-template.csv` expands these into individual numerical
metrics. Keys are independent mathematical answers, not a claim about what your
TORA run will display. Do not edit the template; record your work in the results
copy. The old "comparison pending" labels in the practice sheets mean a fresh
comparison session; they are not evidence that your test has been performed.

For **every workflow**, do all five listed cases:

1. Open its README and enter or open the input in the matching Eidolon module.
   Assignment uses `input.assignment.json`; most others use `input.txt`.
2. Set the same objective direction, bounds, integer flags, start method and
   precision in both applications. Use at least four decimal places.
3. Read each input back from the grid. A wrong model is not a solver mismatch.
4. Solve in Eidolon and TORA. Record the displayed numerical values in the CSV's
   `eidolon` and `tora` columns. Use a dot as decimal separator in that CSV.
5. Compare feasibility first, then the objective and other metrics below. Tied
   optimal variable values, assignments, routes and pivot order may differ.
6. Save a screenshot of the input and result from each app under `results/`.
   Record iteration choices and filenames in the `notes` column.

| Family | What to check beyond the final number |
|---|---|
| LP Graphical | Constraint lines, feasible region, objective line and optimum. |
| All-slack, Big-M, Two-phase, Bounded, Dual | Next/All, allowed manual pivot, ratios, basis, feasibility and final objective. Compare the same method, not two different pivot sequences. |
| LP Final/Sensitivity | Variable/slack values, objective, dual/reduced-cost interpretation and displayed validity ranges; retain a screenshot of every section. The CSV alone does not certify every sensitivity field. |
| Change Cj / Change RHS | Solve the baseline, enter the specified change, inspect retained-basis feasibility and the reoptimized answer. Confirm Analyze does not silently edit the input. Test Use changed model and Save As on a copy. |
| Dual Revised (RHS) | Inspect basis/dual trace and final changed-model answer. A matching final answer does not establish identical intermediate algorithms. |
| Transportation | Compare North-west, Least cost and Vogel **initial** allocations separately; then MODI iterations, manual loops, final shipments, row/column totals and optimum. |
| Assignment Problems | Every real worker/job is used at most once, selected costs sum correctly, and unmatched entries/dummy pairs are identified. Test both min and max. |
| Integer pure/guided/mixed | Integrality only on marked variables, branch bounds, incumbent objective, pending/unsolved nodes, and guided node/variable choice. |
| Equations / Matrix inverse | Residual Ax-b, rank/status, inverse product, determinant and row-operation trace. |
| Networks | Edge direction and weights, valid path/tree, connectivity, flow conservation/capacity, and flow=cut. Test every listed algorithm. |
| CPM / PERT | Duration, earliest/latest events, activity floats, all tied critical paths, and PERT means/variances. Confirm duration units. |
| Queuing | Correct finite/infinite model, capacity including service, population, state probabilities, utilization, admitted rate, L/Lq/W/Wq and unit time for transient cases. |
| Zero-sum games | Min/max roles, probability bounds and sums, value, saddle point or mixed equilibrium, and graphical envelope where available. |

### Equivalent original workflows

- **Assignment:** open `tora-equivalents/<case>.txt` in TORA Transportation.
  Every supply/demand is one; rectangular problems are square-padded with zeros.
  For maximization, costs are negated. Record TORA's raw minimum cost in the CSV;
  it is the negative of Eidolon's maximum profit. Do not count dummy pairs as jobs.
- **Cj/RHS changes:** the baseline input is unchanged. TORA has no matching class
  extension screen; open the supplied changed-model equivalent and solve afresh.
- **Dual Revised:** compare the changed-model optimum to a TORA re-solve; inspect
  Eidolon's revised trace independently. Do not claim original revised-trace parity.
- **Bellman-Ford extension:** the baseline uses positive edges, so TORA's shortest
  route answer provides a common comparison. Negative-cycle tests are separate.

### Known queue differences: do not force equality

For finite source **and** finite capacity, original TORA divides L and Lq by
attempted arrivals, including blocked attempts. Eidolon uses admitted throughput
for an admitted customer's waiting times. Pure-death original Ls omits the initial
state contribution, truncates after a post-peak probability below 0.00001, and
limits active states to min(K,N,500). Eidolon reports the full requested mean.

Rows with these differences contain separate `expected_eidolon` and
`expected_tora` values and an explanation. Original-value predictions come from
the instruction-checked arithmetic and match the earlier captured baseline at
its printed precision. Your fresh observation is still required. For pure death
N=2, mu=1, t=1, the full mean is 3/e=1.1036383235; original Ls is 1/e=0.3678794412.

## 4. Check keyboard entry, resizing and persistence

On every module's input and solution screens:

- Single-click a cell; Enter moves down without opening an editor. Type to
  replace, F2/double-click to edit, Enter to commit/down, Shift+Enter up, Tab
  across and Escape to cancel. Buttons and menu items activate with Enter.
- Copy/paste a rectangular TSV block (Command on macOS, Ctrl on Windows).
  Protected cells, ragged blocks and out-of-range pastes must be handled safely.
  Some grids keep invalid draft text until Solve; correct the error and retry.
- Resize between a small window, half-screen tile and maximized. Reach the last
  table row/column and every action; confirm results/iteration history persist.
- If available, test 100%,125%,150%,200% scaling and move between monitors with
  different scaling. Record actual hardware/scale; skip unavailable combinations.
- Save, close, reopen, edit and Save As to a new path, including a path with
  spaces/non-ASCII characters. Confirm read-only originals stay untouched and
  cancelled dialogs/invalid opens preserve the current model.
- Export a populated solution to PDF. Check title, all rows/columns, page breaks,
  clipping and blank pages. Physical printer testing is optional and not required.

## 5. Run the edge-case checklist

Use `edge-cases.md`. Enter these in the UI to exercise input validation and status
messages; most are intentionally outside the small baseline bank. Mark each as
pass, fail, skipped or unsupported with actual observations. A safe refusal is
acceptable where documented; a crash, hang or stale answer is a defect.

## 6. Review and send the evidence

Optional, if Python 3 is installed:

```
python scripts/compare-results.py results/comparison.csv
```

On macOS use `python3` if `python` is unavailable. The checker evaluates only
entered scalar values against their own keys. Blank entries stay UNTESTED;
unsupported TORA workflows must be recorded, not silently counted as passes.
Different optimal solutions still need the feasibility checks in section 3.

Send the `results/` folder, source/version from `release-info.json`, OS/architecture,
display scale, case ID, exact action sequence, expected/actual result and input
file for each failure. Include screenshots/PDFs and say whether you reproduced
the issue after reopening the app. Review your evidence for personal problem
titles or local paths before sharing. Keep your original saved models separately.

Passing this finite checklist is bounded evidence, not proof for every possible
input. Record untested hardware, unsupported methods and unresolved differences.

References: [Apple's app-opening guidance](https://support.apple.com/en-us/102445),
[MIT finite-capacity waiting times](https://web.mit.edu/urban_or_book/www/book/chapter4/4.6.3.html).
