# Project Planning implementation

The clone implements activity-on-arrow CPM and PERT as an acyclic directed event network. Each activity carries a from node, to node, label and either a deterministic duration or estimates a,m,b. Original GUI observation establishes this input model; it does not establish recovered source or the original implementation algorithms.

## Original evidence and file compatibility

REA capture uses the user-selected original `.tora-installed/tora.exe`, the existing private Wine prefix and a dedicated Xvfb96 display. The reference runtime was absent after the host session restarted; only the owned display/runtime units were recreated. No unrelated Wine process was stopped.

- CPM two-activity input1→3 duration4,3→7 duration6 and original Save As: `ev_9c35ffa7486b139eba0c60f955cfeca13df93d7f4be2320dbb85d59aa5a07e51`. Private source record: `reference/tora-captures/full-inventory/actions-planning-cpm-two/tora-saved.txt`.
- PERT two-activity input1→3(a1,m3,b5),3→7(a2,m4,b6) and original Save As: `ev_b63ddb5c93c8ecc9406cd1d1a12e47b2ad01232b221a85fb488715cc74cac22f`. Private source record: `reference/tora-captures/full-inventory/actions-planning-pert-new/tora-saved.txt`.
- Earlier single-activity PERT original record: `ev_5ba526447e19bae6a91c1f0b41a2b1b8e4cb27e22f01e536ae62ff438b010b70`. Original Activity Mean/Var displayed3.00 and0.44 for(a1,m3,b5). PERT Calculations on that minimal model produced VB runtime9, Subscript out of range; the clone validates and computes the acyclic network instead of reproducing that fault.
- GUI released at visually verified MAIN: `ev_be0946109d1d255687545b17e244242c0779542afbd4cac19b0ed59a7ab26d77`.

Original records are line-oriented, with headers marker8001(CPM) or8002(PERT), N,D,notation1(decimal)/2(scientific), quoted title, maximum event ID, then grid row dimension including the heading. Each remaining row has4 quoted fields(CPM) or6(PERT): from,to,symbol,duration or from,to,symbol,a,m,b. The sparse event IDs1,3,7 establish that the first dimension is the maximum ID, rather than the count of distinct events. Saved two-activity grids have dimension4 and include a third blank row, encoded as from`0` and empty remaining fields. The earlier solved single-activity PERT record has dimension2 and no trailing blank row.

`src/desktop/planning_document.{hpp,cpp}` imports these original8001/8002 streams, ignores only complete blank sentinel rows in the computational model and preserves an unchanged import byte for byte, including numeric spellings and blank rows. Edited exports use the same original schema, full numeric precision, CRLF, and doubled quote escaping with literal backslashes. Parsing enforces dimensions, finite numbers, positive event IDs, maximum event ID, one-line strings and bounded model/file sizes. The controller continues to import earlier clone planning JSON files; current saves use the original record format. Synthetic export roundtrips are tested locally; both edited 8001 and 8002 clone exports were later loaded successfully by the original application; see the 18 October evidence below.

The inventory also confirms row header single-click insertion after the target, double-click insertion before it. This convention is implemented in native EditGrid actions.

## Numerical method

`src/planning/planning.cpp` sorts events topologically with Kahn's algorithm. Earliest event time is the longest incoming path duration. Latest times start at the project duration and propagate backwards by minimum successor latest time minus duration. Disconnected components share virtual start/finish times, so the project completes when the slowest component completes. Cycles, self loops, invalid endpoints, invalid estimates and nonfinite/overflow calculations are rejected.

For activity i→j of mean d:

- ES=E(i), EF=E(i)+d.
- LF=L(j), LS=L(j)−d.
- Total float=L(j)−E(i)−d.
- Free float=E(j)−E(i)−d.

Zero total float, within a numeric roundoff tolerance, identifies critical activities. Output lists every critical activity and one selected longest-mean critical path; it does not enumerate exponentially many paths. Equal-mean path ties select the greatest accumulated variance. Zero-duration dummy arcs are supported. Labels need not be unique; input activity order identifies individual arcs.

Every forward and backward update records its event, activity index, prior value, candidate and resulting max/min. These are actual solver updates. Tests independently replay them and compare the final event times.

PERT moments use mean=(a+4m+b)/6 and variance=((b−a)/6)^2 with0≤a≤m≤b. `approximateDeadlineProbability` applies the normal CDF to the selected path's mean and summed variance. This assumes independent activity durations and approximates the selected path only; competing paths, correlations and path switching mean it is not an exact project completion probability. A zero-variance path gives a deterministic step probability.

## Native workflow

`src/desktop/planning_screens.cpp` implements activity editing, automatic blank row append after a completed final field, add/delete activities, header-selected copy/paste/insert/delete and one-level Undo, CPM/PERT selection, decimal/scientific N/D formatting, validation, original file open/atomic Save As, read-only copy protection, dirty/cancellation guards, and wrapped paginated printer/PDF output.

Results include project duration, earliest/latest event times, activity mean/variance, ES/EF/LS/LF, total/free float, critical activities, selected critical path and the forward/backward calculation trace. PERT adds a deadline normal approximation with its assumptions visible beside the results. An activity network tab draws directed labeled arcs and colors critical activities red. Drawing is limited to200 events to keep the UI usable; numerical results support10000 events/activities. Sparse IDs and disconnected DAG components are supported. This graph is a clone presentation choice, not a claim of original screen or layout parity.

`planningModule()` registers input/result screens and keeps state in the per-window `PlanningController`.

## Observed verification

Strict backend compilation and execution passed:

```
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc/planning/include src/planning/planning.cpp tests/planning.cpp -o /tmp/tora-planning-test
/tmp/tora-planning-test
```

Tests independently enumerate every source-to-sink path in300 small randomized connected DAGs and verify durations, critical path witnesses, slack identities, analytic CPM/PERT examples, tie selection, cycles, overflow rejection and actual trace replay. An earlier zero-duration-tail endpoint bug was fixed by selecting a sink for the critical path endpoint.

Strict Qt6 syntax compilation passed for both native source files. The standalone codec probe compiled with strict warnings and passed byte-exact import/re-export for all three observed original planning saves. Derived clone model durations were10,7 and3; the first two are mathematical checks of the saved inputs, not claims of observed original result output. Evidence log: `build/planning-native/codec-probe.log`.

The first corrected native gate passed all four scale configurations after the Add Activity fix. Those tests cover original record streams, malformed dimensions, precise numeric/quoted/Unicode roundtrips, atomic failures, read-only files, active input validation, grid Undo, formats, cancellation, native graph captures and extracted PDF text. The later schedule chart regression is pending its own build and run.

Original CPM/PERT output step order, tie decisions, layout and deadline probability UI are not yet corroborated. The implementation uses documented mathematical methods and independent tests; it does not claim full original behavioral parity for those unknown details.

## Resumed validation, 18 October 2026

The first shared native gate exposed a programmatic Add Activity insertion triggering the automatic final-field append handler, yielding two extra rows instead of one. The insertion now suppresses automatic append while initializing its cells. Automatic append remains active for user edits. The existing cycle regression still requires one added reverse arc and verifies cycle rejection; rerun results are pending the next shared build.

The corrected native screen passed `c++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -fsyntax-only -Isrc/desktop -Isrc/planning/include -Isrc/core/include -Isrc/lp/include -Isrc/compat/include $(pkg-config --cflags Qt6Widgets Qt6PrintSupport) src/desktop/planning_screens.cpp` with exit0. This is a syntax check, not an execution result.

## Original solve and exchange oracles, 18 October 2026

- Actual `PlanningDocument` edited export `build/planning-native/cpm-export.txt.clone-export` loaded by original CPM: `ev_f5139a0bc6dd71763675a8158a1c1d700340653b8dfcabbd7fc819dbe58a4470`. Its title and contiguous activities 1→2 A4, 2→3 B6 appeared correctly. The initial generic Transportation loader rejected it because the selected module was wrong; that attempt is not evidence of an incompatible planning file.
- CPM All Steps and Bar Chart: `ev_d5cba5fc31a1ccf05ecfaa080d9b68dc149582acc46c72de83a91f20bc302b7d`. Forward event times0,4,10 and backward10,4,0 corroborate duration10. Both activities are critical with zero total/free float. Output offers CPM Calculations and CPM Bar Chart, Next Step/All Steps, printing and graph zoom.
- Actual edited 8002 clone export loaded by original PERT: `ev_4e44d7e89c8d130a77adaadf250430865a8649a5d98994d2363f279d7718b2f2`. Moments and calculation fault: `ev_68c48b107bc069290ce62ab6e6e9522e72023b49c847f02142ab0c590dd74e5b`. A(a1,m3,b5) displays mean3.00,variance0.44; B(a2,m4,b6) mean4.00,variance0.44. PERT Calculations shows node2 path1-2 mean3.00,std0.67, then VB runtime9 before node3. Total7 remains independently computed by the clone, not observed from the original. This second fault occurs on two contiguous edges, so sparse labels and a one-edge graph alone do not explain it.
- Acknowledging that original fault exited the actual target: `ev_106dc116b02fda617a10aea7840b6859c35352c055d90669c59e48eb10f016f8`. Recovery restarted only the owned runtime unit after verifying no `tora.exe` remained. One owned Wine device resisted SIGTERM; only the dedicated unit received KILL to finish its stop. No unrelated Wine process was touched.
- CPM direct-slack schedule experiment A1→2 duration3, B2→3 duration5, C1→3 duration2: `ev_8cad4d8d1c61e1aea46e07f561e2578ca0c17af37ecf1dedb3217b936c2b18cb`. C has total/free float6. Relative shift+3 moves C from[0,2] to[3,5]; subsequent relative choices range−3…+3. The original prints start3, delay from earliest3 and original free float6.
- CPM successor experiment A1→2 duration3, B2→4 duration5, C1→3 duration2, D3→4 duration2: `ev_4ba664b63827b05ffd229b79b7771c0a9a3a97d1588a502806a11381d4c72c21`. C total float4/free float0 permits+3; original moves C start3 and D start5. The warning says following activities beyond D are not checked. The clone intentionally checks every downstream successor. Clear MAIN was visually verified in this capture and handed directly to the Integer oracle worker.

## CPM schedule implementation

`planning::schedule(problem, requestedDelays)` keeps each request between zero and that activity's original total float, processes all events topologically, and starts each activity no earlier than both its requested earliest-relative start and every predecessor finish. It rejects invalid/nonfinite requests and any start beyond the original latest start/project finish. This validates collective precedence, rather than merely checking separate slider limits. The controller stores only transient experiment requests, so saved model durations and solved CPM bounds remain unchanged. Moving an activity earlier than a delayed predecessor is rejected with an explanation; Reset Schedule restores earliest feasible starts.

The native CPM Bar Chart uses actual scheduled starts/finishes, gray original ES→LF windows, red critical activities and blue noncritical activities, numeric time ticks, activity tooltips, relative shift and reset controls. It shows100 activities per selectable page with scrolling and zoom. Print Graph uses vector output and20 activities per page; calculation printing also includes current schedule start, finish and delay values. Transitive downstream propagation and a numeric shift control are explicit clone behavior improvements; the original uses integer relative choices and warns it does not recursively check following activities.

Strict backend compilation and execution passed with the existing300 independently enumerated DAG checks plus300 delayed schedules checked for every predecessor relation, every ES/LS bound, preserved durations and unchanged completion, as well as direct and transitive successor examples and invalid requests. Native regression now covers the original C+3→D start5 example, relative bounds, impossible predecessor start rejection, unchanged input, Reset, zoom and extracted graph PDF text. Its execution result is pending the next shared build.

The schedule output page always creates its CPM chart tab, then toggles it and the PERT deadline labels when the method changes. This respects the shell's cached-page lifecycle. A PERT→CPM→PERT regression also checks activity names refresh for a same-sized replacement model. Strict native syntax compilation passed again after these changes. The complete native execution gate is pending root's serialized build.
