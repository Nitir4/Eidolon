# Transportation manual-loop feedback

Status: implemented locally; focused verification is in progress. The installed
release still comes from the verified multiplier source e8e2c23.

## Observed interaction

The original tells the learner to click cells defining the computed
change-of-basis loop in any order, then press Next. Only shipment rows provide
feedback. Cost and reduced-cost rows do not color a shipment. Correct entering
cells turn green, the chosen leaving cell turns red, and other computed-loop
cells turn gray. Incorrect cells stay unchanged without an error dialog.
Repeated clicks do not toggle a correct cell off.

Correct basic cells can be clicked before the entering cell. Completion is
optional: Next after only one accepted cell still gives the normal computed
next tableau. Automatic Next/All already work with no manual clicks. Feedback
does not choose a new pivot or change the numerical allocation.

The entering-tie discriminator is important. In case18, S1D3 and S2D1 both
have positive reduced value4.00. Clicking alternateS2D1 gives no feedback;
the computedS1D3 loop receives the expected colors. The leaving-tie case17
marks chosenS2D1 red, while equally small minus allocationS3D3 stays gray.
In case05, the zero basic allocationS1D1 is still the chosen leaving cell
and turns red. Blank nonbasic shipments and basic zero shipments differ.

Multiplier initialization preserves accepted feedback colors. A terminal
shipment click gives no new loop colors. Fresh solution starts show an
unmarked active tableau. On progression, the previous computed loop has full
automatic coloring and the newly active tableau accepts its own attempt.

## Evidence

Private captures are under reference/tora-captures/transportation/manual/.
The reviewed ledger build/transportation-manual/reviewed-feedback.json contains
independent rules, screenshot paths/hashes and successful REA identifiers.

| Question | Evidence, child exit zero | Observation |
| --- | --- | --- |
| Entering/wrong/repeated click | ev_d9239f060f726d91751eb492945e938a04d4a7b9264b05b91c78406f511daa4c | Correct entering green; wrong shipment unchanged |
| Nonpath-order complete loop | ev_3a251e7326f634cafe77340ab6993ce5acf46ebb52baccc1bccf325ed48ede31 | Other gray/leaving red before completion; Next reaches170.00 |
| Partial/repeated correct click | ev_f51643f307a28168badffe674d466f461ebee8e8391875d41cc8e483096e4d07 | No toggle; one-cell attempt still advances |
| Equal improving alternative | ev_add41a582a8cc135ed7c174d409ea74b3f23d88cfc638a8832ab700c0c509e2a | Alternate tied entering ignored |
| Event row/leaving tie | ev_e6dcaabe8e65be3830f14d6cc91612edfc4894543d5dbc3fe3ae757bae0f88be | Shipment only; chosen leaving red, other tied minus gray |
| Zero basic leaving | ev_b5d67f270384018273977d8cea9a7b25b627c2818d263fc5b97f745bf5afa412 | Zero allocation red and correct loop marked |
| Anchor refresh | ev_f830a2d6f847a5f238ebb3a5446f29e5d44e6f88590e8c18369e9681a2e9b45f | Feedback colors retained |
| Actual terminal shipment | ev_6e2752bc5c8e1cc1bedc8040b5547c5083ab015b64f53b57be27ff186513d313 | No new feedback |

The earlier terminal-cell-click image in the anchor probe targeted a cost row;
it does not establish terminal shipment behavior. The later explicit shipment
capture above resolves that gap. Raw original images remain private and are
not shipped.

## Native design

The Qt-free loopRole API classifies a cell against the accepted computed
nextPivot: Entering, Leaving, Other or None. It is read-only and returns None
for absent pivots and out-of-grid coordinates. Existing solver allocation,
tie policy, objective and improvement history are unchanged.

The native output page keeps a set of accepted source/destination cells for
the active displayed tableau. Shipment-row clicks are translated to that
tableau's logical coordinates. Accepted clicks mark only the clicked item,
preserving scroll position; repeated insertion into the set is idempotent.
Wrong rows/cells receive no new role color. Earlier displayed tableaus retain
their automatic loop colors.

A multiplier-only refresh renders the stored attempt again. Fresh solution
changes clear the attempt and restart the shown count. Next/All discard the
active attempt and reveal the next existing solution state, with full automatic
colors on earlier computed loops. Next is never blocked by an incomplete
attempt. The interface adds a concise explanation of this interaction.

The approved native accessible palette continues to replace the saturated
original colors: entering pale green, leaving pale red and other pale header
color, with readable dark text. Default surface cells are already white; tests
compare that actual baseline instead of assuming an absent brush.

## Verification

Core tests first fail on the missing role API. New native tests were added
before implementation; their initial baseline-brush assumption was incorrect
and is retained as a failed test attempt, not valid regression evidence.
Corrected tests use the established white surface. An isolated copy of
pre-feature source94de057 with the corrected native tests is being checked to
prove missing feedback separately from that mistaken assumption.

Core checks use independently read pilot/tie/zero/terminal roles. Native checks
cover real mouse clicks, row filtering, repeated/wrong/out-of-order cells,
anchor preservation, partial Next, later-tableau feedback, fresh reset and
terminal behavior. Twelve manual renders cover three sizes/four scales.
Actual Wayland acceptance adds entering/anchor-preserved views while retaining
LP input and the existing structural/anchor checks. Observed results and
delivery identity belong to STATUS.json.

Other manual pivot choices, loop editing/cancellation beyond the observed
feedback interaction, settings and larger numerical precision profiles remain
unverified. The original evidence supports computed-loop teaching feedback,
not arbitrary user-selected entering/leaving pivots. Final summary, printing
and the remaining OR modules stay separate cycles.


## Current local gate

The corrected regression tests on isolated pre-feature source94de057 fail exactly on absent accepted-cell feedback (73 passed, three new checks failed). Current source passes the core plus four native scale invocations: five passes in30.70seconds, then a final five-pass run in25.05seconds with explicit Least-cost/Vogel assertions. The first invalid baseline-brush failures remain retained as test-development history.

The production-source full desktop suite passes92/92in187.86seconds; Qt-free full suite passes36/36in4.39seconds. Final cross-method test extensions were focused-checked after that full suite; production code did not change. Actual Wayland acceptance passes25views with LP input preserved. The three-OS/package/install gate is still pending.

Nonoptimal Least-cost case10 shows enteringS2D1green and leavingS2D2red under REAev_e1a8ca246454c74de40e763b8e99d3b6cb1480f8f4612164fed6173965605b2f. Vogel case17 shows enteringS1D1green, leavingS1D2red and nonloopbasiczeroS3D3unchanged under REAev_4096e6f8a6cb95b0235565cb292ef613dea551e84afed1dc3054004b3148e871. Both child exits are zero. These observations support the shared native feedback path across all three starts.


## Delivery gate attempts

Source22ce1fc passed92/92CTest on each OS in the initial run38351356443. Windows/macOS finished successfully. Linux packaging passed, then headless Wayland deployed welcome startup timed out after45seconds with an empty application log. The Linux package upload was skipped. Its five available archives and failed startup/server logs are retained; no initial artifact was installed. The cause is not established.

A fresh full matrix at unchanged source is running as38353276843, with separate evidence build/transportation-manual-final/ and intended packages dist/transportation/manual-final-verified/. This tests independent runner state without a guessed product fix or deleting failed-run evidence. Until the complete audit/install succeeds, the installed release remains e8e2c23.


## Final delivery

The fresh exact-source [gate](https://github.com/Nitir4/TORA-Clone/actions/runs/38353276843) passed all three OS jobs,92CTests each/276total,76Transportationchecks per scale and all existing comparisons. All36manual renders were reviewed, including small200%images on eachOS. Package/legal/installer/PDF audits and281Linuxpayloadhashes plus actualWayland/xcbstartup passed. Six artifact archives passedGitHubdigest/integrity checks.

Source22ce1fc is installed through tora-clone and its exact AppImage identity was verified in a nativeWaylandwindow. SHA256 1d683908734f73d2107d5398637559e4312eb2f49acdc0c928eafdc2fa17983e. Prior releases/windows and firstfailedgate artifacts/logs are preserved. The initialLinuxheadlessstartup timeout did not recur on new runners; its rootcause remains unknown. Newrun evidence is build/transportation-manual-final/; packages dist/transportation/manual-final-verified/.
