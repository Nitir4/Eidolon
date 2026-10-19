# Integer original-runtime oracles (2026-10-18)

This bounded REA pass executed the installed original `.tora-installed/tora.exe` in the existing owned Wine prefix and X display `:96`. REA `capture-process` retained each driver invocation, stdout and screenshot digests. Private screenshots and manifests are under `reference/tora-captures/integer-oracles/`; helper drivers and input candidates are under `build/integer-native/rea/`. No native analysis session was opened, no Actions were triggered, and no original process recovery was required during this pass.

## Answered questions

| Question | Observation | REA evidence |
|---|---|---|
| Does original5000 accept the clone's model? | Integer input visibly has max3x1+2x2, rows2x1+x2<=4.5 andx1+2x2<=4.5, lower0/upperinfinity, unrestrictedn/n and integery/y. | `ev_2fa9856de085fa5024e3131cd398a0570f3cbaa1cf4810f4e637e1c1368c70ba` |
| Which methods does the output dropdown expose? | `Automated B&B` and `User-guided B&B` (plus initial `?`). This observed menu does not expose a cutting-plane method. | `ev_c4924e3ac600f64641956ae2c6988043130b2e15f9954dcfee6aa984fc496e5e` |
| Pure integer result? | Automated max6 at(2,0), found iteration9 and verified iteration11. Improved feasible list:5 at(1,1), then6 at(2,0). | same evidence as method dropdown |
| Guided interaction? | Choosing user-guided mode reveals color-coded search tree and active-bound fathoming checkbox. Clicking green root opens a branching-variable grid: relaxation7.50 at(1.50,1.50), integer flagsy/y, upperbound7.50 and lowerbound-infinity. | `ev_e95fa1b73a7af1e7d16e840b7c7d5afe6a6c381b8a24bf219d0e8fbcdc5888ba`; `ev_6d96c0c0d6fd536dc3c13076209c872f51aa2724c4fd22d0aa4bddd0c688873a` |
| Can the user choose x2 first? | Clickingx2 creates childrenx2<=1 withLPbound7.25 andx2>=2 withLPbound5.50. Both are green and await investigation. The active-bounds checkbox can be unchecked. | `ev_e3970e06de3e0036d56daccec5e0610fcff76f3e04f527f0a689eb403b473107` |
| Mixed result? | Original accepted the same5000 model with integer flagsy/n. Automated max7 at(2,0.50), found and verified iteration3; improved feasible list6.50 at(1,1.75), then7. | input:`ev_96b4ac4ce2edbbc6083700b59177f6f252ec356619beefa2341f7b1e9a053238`; result:`ev_3bf06d8636fe6e8ae1476a81268a058ba02dd568cdb9f13cf5553d29eca4e461` |

Original coordinates and native layouts differ. Original iteration numbering, green/red/gray/magenta rendering, branch search order and exact print layout are observations, not compatibility guarantees of the clone. The native solver's proof and trace follow its independently checked LP relaxation engine and its own explicit node/tolerance limits. Guided mode implements the observed choice of pending subproblem, fractional integer variable, bound fathoming and inspection of the branch tree/path. Its dropdown/table interaction is a native Qt design choice.

## Remaining questions and limitations

- Complete original user-guided run, all tree color transitions, exhaustive branch-order behavior and its10-branches-per-level/32000-subproblem limits were not exercised. The corresponding limits appeared in a bounded static printable-string inspection only.
- The original Write to Printer button is visible. Original print-section selection and exact original tree/summary page layout were not observed in this pass. The clone currently prints the model, incumbent and branch trace with wrapped text.
- This pass did not exhaust all models, infeasibility/unbounded semantics or all integer variable/bound combinations in original TORA. Those clone cases have independent mathematical regression coverage rather than claimed original parity.
- An initial generic Transportation loader correctly rejected marker5000 as a module mismatch. The prompt was declined, then Integer was explicitly selected. That attempt does not constitute Integer import failure or a process crash.

The original GUI was returned to MAIN after the mixed result. Its popup was then dismissed and the clear MAIN screen visually checked (`release-clear-main/main.png`). Parent closeout owns final full-suite and rendered evidence, external documentation and the fresh handover. No additional application module is started by this task.
