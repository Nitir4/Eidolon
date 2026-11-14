# Active task — friends’ playtest delivery and source publication

20 October 2026, implementation mode. User requests windows.zip and macos.zip
with native Eidolon builds, launch/check scripts and testing.md, plus publication
of all source work. Generated ZIPs are ignored and must not enter source commits.
User explicitly chose pushing all now with new commit dates spread from
14 October through 15 November 2026, each below 500 added plus deleted lines.
Future timestamps do not schedule publication; preserve existing history.

Delivery tooling and guides are prepared: 126 problems, 42 workflows, 210 case
runs, 503 scalar comparison rows, 21 TORA-equivalent inputs. The local isolated
Qt6.10.3 Release gate passed 179/179 in37.34s; five delivery tests pass separately.
Native Windows/macOS binaries and friend scripts have not yet been validated
for this source snapshot. Unit fixture payloads are not delivery packages.

Actions remain disabled. Source push is authorized only while disabled; enabling
and ONE manual Build, test and package run still require specific approval.
Proposed inputs: platforms=windows-and-macos, playtest=true; two45minute jobs.
Pin the final source SHA in the external execution plan before asking approval,
then disable Actions after that execution. Do not reuse the October18 approval.
Keep the old tora-clone installed command removed; launch eidolon only.
Evidence: build/playtest-20261020/ and external docs/Eidolon/.

---

# Latest completion — Eidolon only, 20 October 2026

User requested removing TORA Clone and retaining only Eidolon. Removed old
~/.local/bin/tora-clone compatibility wrapper, old per-user icon and
~/.local/share/tora-clone release installation (43 files,503686238bytes), after
confirming known artifact-only contents and no running old process. Old desktop
entry already absent; Gio sees one Eidolon entry. Former-name search keywords
removed from live/future desktop metadata; README alias claim updated.

Eidolon MAIN+Assignment Wayland startups pass after removal; installed AppImage
hash5731888230a8 remains unchanged. Saved problems/preferences, history/source
paths and original TORA oracle preserved. No algorithm edit/full-suite rerun,
Actions or remote change. Launch eidolon only; do not recreate the old alias.
Evidence build/eidolon-only-20261020/ and docs/Eidolon/eidolon-only-20261020/.

---

# Preserved preceding handover

# Latest completed task — Eidolon rebrand, 20 October 2026

User selected Eidolon and requested GitHub repo rename. Implementation mode.
Remote renamed LIVE: https://github.com/Nitir4/Eidolon, sameID1403605185.
Origin updated. Actions remain disabled; run count/latest ID unchanged. No source
commit/push. Preserve all existing dirty work; do not infer publishing approval.

App/welcome/window/exit/PDF branding and package/install identities now Eidolon.
CMake projectEidolon, internal targetToraClone with OUTPUT_NAME Eidolon.
QSettings explicitly uses oldToraClone/ToraClone store; old serialized IDs intact.
71 numerical/compatibility files unchanged. Resource files now eidolon.desktop/svg.

Full isolated Qt6.10.3 Release178/178 in33.74s, portable packaging16 checks pass.
Native private-X11 packaged welcome/title+legacy900x650 geometry verified; PDF
headers and MAIN rendered/inspected. Windows/macOS delivery not run. No Actions.
Package SHA2565731888230a8ed1bd533c89c63d5c3ca026a00dd7f7c718f66acf9bebb5e0190.
281 payload hashes,92 ELF audits, installed Wayland MAIN+9 and old-alias smoke pass.
Launch ~/.local/bin/eidolon or wofi Eidolon; tora-clone remains an alias. New
release ~/.local/share/eidolon/releases/20261020-5731888230a8/. Old releases,
preferences and running windows preserved. Gio finds one visible eidolon entry.

Checkout path stays /home/nitir505/Ritin/TORA-clone for build/evidence continuity.
Docs /home/nitir505/Ritin/docs/Eidolon aliases existing docs/TORA-clone history.
Evidence build/eidolon-rename-20261020/ and docs/Eidolon/eidolon-rename-20261020/.

---

# Preserved preceding handover

# Latest completed task — UI cleanup, 20 October 2026

Implementation mode confirmed. Assignment displayed as Assignment Problems in
MAIN, page descriptors, input heading and PDF heading. Stable assignment routes
and mathematical code unchanged. Queue comparison removed hard-coded captured
values/stale unresolved-discrepancy note; verified omission/cutoff now explained.
Independent min(N,500) comparison explicitly distinguished from original min(K,N,500).

Only assignment_screens.cpp and queuing_controller.cpp changed among130 source
files, plus existing navigation/presentation test expectations. Preserve dirty work.
Isolated Ubuntu24.04/Qt6.10.3 build passes; focused12/12 at four scales. Initial
four Assignment failures were capture filename collisions; isolated rerun4/4 passes.
Rendered MAIN/compact Queue and PDF text inspected. No full-suite/Windows/macOS run.

Linux package installed and launcher updated; SHA256
12bf00648c8583da54d3f58ec2aa351f1a3c1486d8dd872cb81e9f617dcaf7e8.
281 payload hashes,92 ELF audits,10 installed Wayland startups pass. Installed
~/.local/share/tora-clone/releases/20261020-12bf00648c85/. Existing windows/releases
preserved. Reopen launcher for update. Branding stays TORA Clone pending choice;
proposed names ORbit (preferred), Pivot, OptiLab. No Actions/remote writes.
Evidence build/ui-cleanup-20261020/ and external docs/ui-cleanup-20261020/.
Host cached build references removed Qt6.11.2 library; use isolated baseline build.

---

# Preserved preceding handover

# Latest completed continuation — queue binary audit, 19 October 2026

User requested decompilation and mathematical adjudication of disputed TORA queues.
Implementation mode remains confirmed. Full findings and address-linked artifacts:
/home/nitir505/Ritin/docs/TORA-clone/queue-binary-20261019/README.md
Workspace: build/queue-binary-20261019/.

REA/Ghidra boundaries/assembly/callees and helper pseudocode recovered. Full C
rendering of queue function 0x4977d0 timed out at 30s in normal/normalize modes;
readable arithmetic is manually decompiled from complete native instructions.
Reconstruction matches 17 controlled outputs and 10 original baseline differences.
Fresh native decoder/solver probe: 17 scenarios, exact match to retained results;
independent Fraction/70-digit Decimal standard checks pass. No production edit.

Finite-source rate 0x4992ce..0x4992f0 = lambda*(N-L), includes blocked attempts;
used for waits at 0x499a0a..0x499a4f. Shared c=1/c>1 finite-source block confirmed,
including c=2 baseline. Standard admitted throughput remains the correct default.
Pure death: global 0x50a328=500; min(K,N,500) active bound; Exp(-mu) seed;
mean initialized 0 and loop k=1..M-1 omits M*pM. Cutoff 1e-5 encoded 497aae/ab8,
exit 49a502..49a520 after peak. This exactly explains prior large-N residuals,
rather than unexplained Exp rounding. Entered N can differ from effective bound.

All 130 production hashes and original EXE SHA unchanged. Current comparison UI
still has independently calculated partial means and two captured notes saying
residual unresolved (queuing_controller.cpp 58..69); presentation cleanup is a
possible follow-up, not performed in this analysis request. Baseline remains
200 agreements/10 differences. Prior 178/178 CTest not rerun. No Actions/remote writes.
Private REA client closed, service inactive, cleanup.json saved. User sessions untouched.

---

# Preserved preceding handover

# Latest continuation completed — 19 October 2026

User authorized mathematical foundations -> edge tests -> controlled original queue
experiments, with REA where needed. Implementation mode remains confirmed.
All stages complete locally. Current docs:
/home/nitir505/Ritin/docs/TORA-clone/mathematical-foundations.md
/home/nitir505/Ritin/docs/TORA-clone/edge-case-matrix.md
/home/nitir505/Ritin/docs/TORA-clone/math-edge-20261018/README.md
Evidence UI: math-edge-20261018/oracle/index.html under that external docs root.
Working helpers/logs: build/math-edge-20261018/.

Ten new CTest suites, 238 analytic/transformation assertions. Host Release178/178
passed in73.71s; fresh Qt-free new suites10/10 in0.04s. Production sources untouched;
only new tests/mathematical_contracts.cpp plus append-only tests/CMakeLists.txt
registration. Preserve all existing dirty work. The 210-row baseline comparison
and installed87f04786df10 AppImage remain unchanged.

Seventeen controlled original scenarios in13 successful REA captures resolve the
queue statistic distinction observationally: finite-K/N waits fit attempted rate;
K=N removes the difference. Nine small-N pure-death Ls fit the moment omitting N.
At mu=2,N500 and501 share Ls430.33169 and capped probability profile; N499 differs.
Three large-N partial-mean residuals~0.000664..0.000668 remain unexplained arithmetic;
printed probabilities/CDFs match all12 profiles at5decimals. No decompilation was
needed for model validation or performed; exact arithmetic reproduction remains
optional legacy-parity work, never a reason to replace correct default mathematics.

QueuingDocument requires finite K<=N when N is finite, while its backend is more
permissive; record this domain distinction. Initial rejected input/navigation
calibrations and a wrapper signal143 are retained/excluded; resumed captures passed.
No Actions execution/enabling or remote writes. Personal AGENTS instructions active.
Native Windows/macOS confirmation remains separate; physical printer acceptance is
out of scope. All owned private runtime resources are stopped; user app
sessions are untouched (build/math-edge-20261018/cleanup.json). Consult the final cleanup receipt before restarting any
old task runtime. Do not automatically revive stale workflow candidates.

---

# Preserved earlier handover

# Latest completed task: five examples per workflow — 18 October 2026

Implementation mode. Numerical comparison is complete: 126 distinct inputs,
42 workflows, 210 rows, **200 agreements and 10 explained queue-definition
differences; no pending rows**. Evidence and interactive comparison:
`/home/nitir505/Ritin/docs/TORA-clone/comparison-20261018/index.html`.
Workspace helpers/receipts: `build/comparison-20261018/`.

All 210 native production-decoder/solver executions pass 1,030 independent checks;
practice-bank import passes 1/1 CTest (126 inputs, 129 QtTest slots). There are 161
successful referenced original REA captures and 212 original PDFs. The only bank
correction sets K=N=500 in the five pure-death input files and grids; their answer
keys already describe that finite population. Product solver/UI sources and the
installed 87f04786df10 AppImage are unchanged by this comparison task. Preserve the
existing dirty checkout. Do not change correct queue mathematics merely to force
original printed statistic equality. See queue-discrepancies.md for actual values,
formulas, rounding limits and primary references.

This corpus is representative, not exhaustive edge coverage. Actual named native
method dispatch and evidence links were audited. Equivalent original calculations
are explicit for Assignment, Cj/RHS, Dual Revised and Bellman-Ford; unavailable
original traces are not claimed. No native Windows/macOS confirmation was added.
The user excludes physical printer acceptance; software print capture was used.

The user's personal AGENTS.md was supplied again and is active. Earlier historical
claims below that it was rescinded do not apply to this task. No Actions were
triggered, enabled, dispatched or rerun; no remote writes occurred. The previous
follow-up candidate remains stale after UX work and requires fresh review plus
specific approval before any workflow execution.

---

# Preserved earlier handover

# TORA-clone handover — 18 October 2026

Implementation mode is confirmed. All original families are implemented:
Equations, LP, Transportation, Integer, Networks, Project Planning, Queuing and
Games, plus Assignment and the Cj/RHS/Dual Revised extensions. Do not recreate
completed modules or repeat unchanged failed original probes.

## Preserve the working state

The original checkout remains dirty on future-scope at
b4bd3acb4cde96e07abd17dc569b41a63394cb4c. Preserve existing and new work.
The installed Linux launcher now opens the verified UX AppImage87f04786df10.
Its per-user desktop entry appears as TORA Clone in wofi; the older dc853b4
release remains on disk. Installation receipt: build/wofi-install-20261018/installation.json.
The user rescinded the previously supplied AGENTS instructions. Implementation
mode and the concrete agreement for one native workflow execution remain facts.

That approved execution,39106336418, ran on isolated validation branch commit
b2889f496c88a6de2ed0dbec45fa34c87e7f5a63. Ubuntu passed163/163 plus packaging
and protocol checks. Windows failed compiling a test; macOS built and passed
154/163. Repository Actions were disabled afterward and independently verified.
No PR, merge, tag, release or real installation was performed.

All observed failures are corrected locally. A six-file patch is staged in
build/platform-followup/remote/publish, but has not been committed or pushed.
The user has been asked to approve one follow-up run or finish with local results;
the answer is pending. Do not infer an answer. The exact plan, patch and hashes
are remote/followup-run-plan.md, followup-candidate.patch and followup-candidate.json.
The first published snapshot and its manifest are preserved separately.

## Applied changes and current verification

Queue mean system time now preserves the service contribution when occupancy
underflows. Guided Integer printing includes pending nodes with complete ancestry
and honest unsolved status, without advancing the solver. Linux deployment finds
the selected SDK during staging and clears SDK variables for artifact startup.
PDF tests use configured Python/pypdf and compare exact table content across valid
font bands. DejaVu's Solution column was intact in another band; product LP layout
was unchanged. Semantic PDF tests scope a generic font and restore the theme.

Native feedback led to three further corrections: the Networks test directly
includes<string>; Assignment/Integer tests use Qt's Return edit key on macOS and
F2 elsewhere; Networks conservation checks normalize incident flows so valid
1e308 flow is not rejected by an overflowing diagnostic magnitude. The flow
algorithm, tolerance, capacity/cut/total guards and API remain unchanged. Huge,
tiny and isolated-node regressions pass, as does a binary64 approximation that
reproduced the original failure. Native confirmation of these fixes is pending.

The corrected full gates pass: host Release163/163 in171.46 seconds, isolated
Ubuntu24.04/glibc2.39/Qt6.10.3 Release163/163 in159.98 seconds, and fresh headless
Release58/58 in6.27 seconds. Platform edit-key checks pass8 scales and4 Wayland
runs; semantic-font checks pass8 scales and4 Wayland runs. Actual Wayland at100%
and200% passes Assignment6, Integer11, Networks6, Planning8 and Queue12 QtTest
slots per run, including setup/cleanup. Integer pending-print and both LP font
corpora pass PDF content, blank-page and page-bound audits.

Current Linux package:
build/platform-followup/linux-baseline/packages-after-native-fixes/TORA-Clone-0.1.0-linux-x86_64.AppImage
SHA256:2916982c39e517318ca52181457fb078340006c7eae3ff5087785596ef89c925.
Offline MAIN+9 module startups,281 payload hashes and92 ELF checks pass; the
maximum required GLIBC version is2.38. Live Wayland, dedicated X11 and contained
Unicode-path launch/removal pass, preserving an unrelated-file sentinel.
The shared host kernel, physical hardware and host desktop-menu integration are
outside this isolated userspace check. Receipt: build/platform-followup/verification.json.

The report compiles to71 pages/13051 words, with38 unchanged evidence blocks
and clean page bounds. Its reviewed source is
build/continuation-report/full-application-report.tex; avoid stale generators.
External documentation and study resource are under
/home/nitir505/Ritin/docs/TORA-clone/. Update native outcomes after any approved run.

## Original evidence and remaining boundaries

REA captured5 original software PostScript jobs,9 converted pages and476 words,
with clean page bounds. Equations confirms(2,1), determinant-3; guided Integer
prints root7.50 and child bounds7.25/5.50. Premature original infeasibility headers
are not proofs. A completed automated screen and empty/infeasible print differ
after mode switching; the cause is unknown. Printer482 was specific to the old
setup, not a universal printing failure.

Changed PERT input with3 complete edges prints means3/4/1 and variances.44/.44/0;
calculations show intermediate means3/7, then VB9 at the terminal event. Final8
was not observed. Newly owned private printing services were stopped; the
original X96 MAIN services remain active. No new deep-analysis session was opened.

The user has no physical devices and requests software checks only. Physical
printing, mixed-DPI handoff and broader original numeric/tie/search/color/layout
profiles remain unverified. Windows/macOS native confirmation awaits the
follow-up-run choice. Keep implementation, runtime and hardware claims distinct.


## Desktop launcher update

User requested wofi integration. The latest verified AppImage is installed under
/home/nitir505/.local/share/tora-clone/releases/20261018-2916982c39e5/.
The stable ~/.local/bin/tora-clone launcher and ~/.local/share/applications/tora-clone.desktop
were updated, retaining backups and the old release. Desktop validation, Gio
visibility, live wofi search and all10 Wayland startup checks pass. Wofi settings
and existing search history were preserved. No workflow execution was involved.
Physical-printer acceptance is outside the user's requested scope; PDF saving is
already tested. The proposed Windows/macOS follow-up run still awaits an answer.


## User comparison windows —18October2026

The user requested the original TORA for comparison. Both are open side by side
on workspace2: native clone on the left, original on the right. Original MAIN
was visually confirmed. It runs in an800x600 Wine virtual desktop through
`tora-user-comparison-desktop-20261018.service`, prefix`.tora-oracle-wine`,
host DISPLAY=:1. Leave this user-controlled session running; do not reuse or kill
its prefix for REA work. Private original reference services on :96 are untouched.
A direct initial launch exited after its screen-setting prompt; the separate
virtual desktop resolved startup without changing host monitor settings.
Receipt: build/user-comparison-20261018/comparison-ready.json.


## Current UX milestone —18October2026

The user requested Enter activation, Excel-style table entry, resize verification,
and at least3–5 examples for each workflow, then original TORA comparisons later.
Explicit preference: Enter on a selected input cell moves down without editing.
This is implemented centrally in keyboard_navigation.{hpp,cpp}; Enter commits/down
inside the editor, Shift+Enter up, Tab/Shift+Tab across, typing replaces, F2 edits
in place, Escape cancels. Buttons/checks/dropdowns activate on Enter. Input views
skip protected and hidden cells. Rectangular TSV copy/paste includes bounds and
protected-cell preflight. Some QTableWidget modules keep invalid draft text until
solve validation; do not claim atomic semantic validation for every model.

LP input tables no longer reserve18 blank rows. LP tableau/summary use full width
and remaining height; iteration instructions collapse. Existing Basic-column
pinning and internal scrolling remain. New tests verify full table area, last-row
reachability and unchanged solver history after resizing. The old Cj/RHS test
helper picked a closed deferred editor; it now uses the active editor, preserving
all stale-result assertions.

Current gates: hostQt6.11.2 Release168/168 in85.70s; isolated Ubuntu24.04/glibc2.39,
Qt6.10.3 Release168/168 in67.06s. Keyboard/layout checks pass on actual Wayland at
100/200 on both Qt runtimes. Native checks first ran in an inactive special
workspace and could not focus a new editor; the active-workspace rerun passes.
A popup-grab warning reflects synthetic QtTest input without a native seat serial;
no physical keyboard/hardware certification is claimed. Offscreen checks include
clipboard validation/rollback at four scales; model rejection is distinct from a
controller-retained invalid draft.

126 practice cases cover42 workflows, minimum5 each, with210 comparison rows.
Import validation passes all126 generated input files. Readable exercises,
structured independent answer keys, native inputs and TSV blocks are under
examples/practice-bank/. Original comparisons are explicitly pending. Do not
invent full original traces, tie rules or sensitivity ranges from these keys.
Guide: docs/KEYBOARD_AND_LAYOUT.md. The generator uses rational vertex/elimination,
exhaustive small enumeration, queue balance/tails and2x2 game formulas.

The packaged UX AppImage SHA256 is
87f04786df10cb7aa4acafe2158bf26f4e41c80d1b5b33d49429a70fb253c614.
Artifact: build/platform-followup/linux-baseline/packages-ux-20261018/.
281 payload hashes and92 ELF audits pass; maximum required GLIBC2.38. Offline
MAIN+9 module starts, installed Wayland MAIN+9 starts and private Xvfb X11 start
pass. Installed release ~/.local/share/tora-clone/releases/20261018-87f04786df10/;
stable ~/.local/bin/tora-clone updated, desktop entry unchanged/validated.
Old release and launcher backup preserved. User's existing app stays on its old
binary until reopened; do not close it or the user's Wine sessions. Wofi config
and history are unchanged. Evidence: build/ux-keyboard/verification.json.

No workflow was dispatched, enabled, committed or pushed for this UX task.
The older proposed native six-file candidate is now stale and must be refreshed
before any further current-source native run; the prior choice is still pending.
Original comparison and broader normal/edge exercises are the next user session,
not claimed complete. Physical printing is outside scope per user; retain PDF
checks. The71page report predates UX; current guide/practice bank supplement it.
