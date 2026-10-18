# Original application inventory

Observed 2026-10-16 through REA capture-process against the isolated original application. This ledger records visible behavior; mathematical implementations must be validated separately. Private screenshots, saved input files and process evidence remain in `reference/tora-captures/full-inventory` and earlier `application-inventory` captures.

## Main hierarchy (observed)

- Linear Equations: Matrix Inversion; Linear Equations Solution.
- Linear Programming.
- Transportation model.
- Integer programming.
- Network models: Minimal Spanning Tree; Shortest Route; Maximal Flow.
- Project Planning: CPM - Critical Path Method; PERT - Program Evaluation & Review Technique.
- Queuing analysis.
- Zero-Sum Games.
- EXIT TORA.

REA evidence `ev_d830c8a20c7aa3c09d8c3114d11fd4308313c05700fefc90f7ce929fe36e8e4e` captures all three submenus. Assignment is not a separately observed MAIN item; its relationship to Transportation belongs to the existing assignment evidence.

## Linear equations (observed)

Linear Equations Solution enters the shared new/existing input mode and decimal/scientific formatting screen. Input has problem title and number of variables. A two-variable input creates two equation rows, X1, X2, and RHS. Save of a zero two-variable example has marker 2000, numeric fields 5,2,1, quoted empty title, dimension2, followed by six quoted zero strings. The interpretation of 5,2,1 is not yet fully established here. Matrix Inversion marker/dimensions remain unverified.

SOLVE/MODIFY includes Solve Problem, View/Modify Input Data, MAIN Menu, Exit TORA. Solve opens output decimal/scientific format selection. A singular zero system shows ORIGINAL EQUATIONS and `NO INVERSE: Determinant =0.00`, with Next Iteration/All Iterations disabled and Write to Printer enabled. Evidence `ev_7057157b3ba62de10d28c5e5532eb4eaceb92037bddfcb7f6664b08e4618bfb0`. No nonzero equation solution is claimed by this initial probe.

## Integer programming (observed)

Input has title, variables, constraints and LP-style table: variable names; Maximize (click to toggle Minimize); coefficients and inequality direction/RHS; lower bound default0; upper bound default infinity; unrestricted y/n default n; Integer y/n default y. Two-variable, two-constraint schema captured by `ev_fa604afe1424e6551c882a7f08584afba818f6371bdcb13b10ddf44ba606541f`.

Zero coefficient constraints block solving with Input Data Error: all left-hand coefficients of the numbered constraint are zero; reenter or delete constraint. Generic table instructions mention Delete, Insert, Copy, Paste row/column through EditGrid. Exact solve method hierarchy and file schema remain unverified.

## Queuing (retained observed evidence)

Original input has title and number of scenarios. One scenario creates `INPUT TABLE - M/M/c queues`: Lambda, Mu, Nbr. of Servers, System Limit, Source Limit. Defaults are0,0,1,infinity,infinity. Earlier private captures `application-inventory/actions-queuing-one-scenario/queuing-grid.png` and REA `ev_` in its adjacent evidence JSON establish this schema. Output, file marker, finite population semantics and limits remain unverified.

## Probe reliability and outstanding work

The dedicated runtime/display were active initially and reused. Screen coordinates need the detected window origin offset; helper SetWindowText alone did not reliably persist edited grid data. Actual keyboard typing with Ctrl+A and Tab did persist a coefficient. One unsuccessful entry attempt saved a zero equation file at private `dual-revised/rhs-min/1.txt`; it is not an oracle for a nonzero model. No product source was changed.

During Integer solve transition the reference process exited. Runtime service explicitly logged target exit after five absent probes; display remained active. Owned service recovery was attempted; no timeout-based display restart or unrelated process kill was performed. Networks/project input and all remaining solve/print/file details still need investigation.

## Follow-up observed evidence

The two early reference exits were caused by my clicking the automatically opened SOLVE/MODIFY popup's Exit item. They were normal exits, not unexplained crashes. Owned runtime recovery reused the active display. A later PERT Calculations probe did produce the actual VB runtime9 `Subscript out of range` dialog (one activity, nodes1→2); this is preserved separately. Final reference state is visually verified MAIN, REA `ev_5e7b5cfd406e76843683dcf4aec4759d4231b402cdeb0b9db6802b7c9b4629fb`.

Integer original save is marker5000, format5,2,1,title,variables,constraints,sense then full grid row-major of (constraints+6) rows by (variables+2) columns. Rows: names, objective, constraints, lower, upper, unrestricted, integer. Quoted inequality is padded `        <=`. Exact zero model capture `actions-int-save/tora-saved.txt` (private) preserves lexical details.

Queue original marker6000, header5,2,1,title,scenario count, then quoted λ,μ,c,K,N per row. λ2 μ3 c1 K∞ N∞ gives effective λ2, Ls2, Lq1.33333, Ws1,Wq.66667, p0.33333. Finite K=N=3 gives probabilities [9,18,24,16]/67, effective arrival174/67, Ls114/67. This establishes per-idle-source arrival interpretation for finite sources. Displayed rho/c remains raw λ/(cμ), not busy utilization. K∞ N3 rejected because system limit exceeds source limit. Comparative Analysis table includes scenario,c,λ,μ,effectiveλ,p0,Ls,Lq,Ws,Wq. Per-scenario probabilities include cumulative values. Original output options and baseline/finite oracle screenshots are under `actions-queue-*`. Finite exact original saved file has marker6000; infinity spelling in Queue saved file not separately checked.

CPM is activity-on-arrow input From Node,To Node,Activity Symbol,Duration. PERT adds a,m,b instead of Duration. Title initializes one blank row; finishing duration appends another blank row. PERT original marker8002, header5,2,1,title,2,2 for a single edge1→2, then quoted1,2,A,1.00,3.00,5.00. Those two numeric fields need a second model to determine their meanings. Output options Activity Mean/Var and PERT Calculations. Activity A with a1,m3,b5 displays mean3.00 variance0.44. PERT Calculations faulted on this minimal model; no computed path oracle claimed.

Networks use title,N, then (N+1)x(N+1) table. First row Node Name plus custom names; first column node names; diagonal blank. MST offdiagonal infinity and symmetry checked. Shortest Route offdiagonal infinity and symmetry unchecked. Maximal Flow offdiagonal0.00 and symmetry unchecked. Saved markers observed7001 (MST), **0** (Shortest, unexpected and intentionally not corrected),7004 (Flow); full quoted grid then symmetry flag. Paths `actions-network-{mst,shortest,flow}-save/tora-saved.txt` preserve exact observations. Source/sink selection, mirror behavior, solve algorithms, limits and printer dialogs remain unknown.


### Planning follow-up, 18 October 2026

Original edited clone8001/8002 exports both loaded successfully. CPM two contiguous edges4+6 gives earliest0,4,10/latest10,4,0 (`ev_d5cba5fc31a1ccf05ecfaa080d9b68dc149582acc46c72de83a91f20bc302b7d`). Original CPM includes a Bar Chart with zoom/Print Graph and relative start-change experiments bounded by total float. C duration2 parallel to critical3+5 permits+3 and then−3…+3 (`ev_8cad4d8d1c61e1aea46e07f561e2578ca0c17af37ecf1dedb3217b936c2b18cb`). A successor D shifts when C exceeds free float, but the original warns later successors are not checked (`ev_4ba664b63827b05ffd229b79b7771c0a9a3a97d1588a502806a11381d4c72c21`). Native chart and full transitive precedence checking have been added; latest native test result is tracked in PROJECT_PLANNING.md.

Original PERT moments for contiguous edges are3/.44 and4/.44, but PERT Calculations still raises VB runtime9 after node2 mean3/std.67 (`ev_68c48b107bc069290ce62ab6e6e9522e72023b49c847f02142ab0c590dd74e5b`). Therefore no original total7 result is claimed. This original fault exits the target after OK; owned runtime recovery was required. GUI was finally released at verified clear MAIN to Integer.

## Integer solved/guided runtime follow-up (2026-10-18)

[INTEGER_REA_ORACLES.md](INTEGER_REA_ORACLES.md) records the observed Automated/User-guided B&B dropdown, original pure max6 and mixed max7, green-root branching interaction, x2-first children withbounds7.25/5.50 and active-bound fathoming toggle. These close the earlier bounded inventory gaps for these core behaviors; exact original print sections and exhaustive guided behavior remain open. The original GUI was released at MAIN.
