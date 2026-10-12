# Transportation final-summary visual review

On 12 October 2026, all seventeen retained Transportation final-summary
screenshots were visually compared with their existing glyph transcriptions.
The review confirms 355 populated table cells and 85 cells in the first blank
row following each table, with zero draft mismatches and no unresolved visible
table regions. Heading, title, final iteration number, objective, column labels
and visible control states were also inspected.

This completes the pending visual review for these exact saved screens. It is
not an implementation or a universal final-summary algorithm contract. No live
reference actions, clone solver calls, regenerated expected numerical values,
product/test edits or print jobs were used in this review.

## Provenance and scope

Each case's original screenshot and unchanged draft are under
`reference/tora-captures/transportation/corpus/<case>/final/initial.png` and
`numbers-draft.json`. The private reviewed ledger is
`build/transportation-final-review/<case>.json`, indexed by
`build/transportation-final-review/review-summary.json`. Records contain exact
screenshot/draft SHA-256, manually read summary fields and rows, inspected-field
counts, comparisons, mismatches, unresolved regions and limitations.

All seventeen final images match their existing `capture-attempt.json` hashes
and the `files_after` image hash of a retained REA process capture. Each matched
capture reports child exit code zero. The ledger records its full Evidence ID
and path; the review did not create replacement original captures or modify the
old pending-status draft files.

| Case | Visible final iteration | Visible objective | Shipment rows |
|---|---:|---:|---:|
| 01-normal | 1 | 150.00 | 4 |
| 02-reverse-costs | 1 | 135.00 | 4 |
| 03-all-ties | 1 | 50.00 | 4 |
| 04-degenerate | 1 | 165.00 | 4 |
| 05-zero-source | 1 | 270.00 | 4 |
| 06-zero-destination | 1 | 150.00 | 4 |
| 07-zero-costs | 2 | 25.00 | 4 |
| 08-decimal-quantities | 1 | 20.56 | 4 |
| 09-negative-costs | 1 | -70.00 | 4 |
| 10-cost-ties | 2 | 64.00 | 4 |
| 11-larger-quantities | 1 | 1910.00 | 4 |
| 12-vogel-ties | 1 | 124.00 | 4 |
| 13-three-sources | 1 | 190.00 | 5 |
| 14-digits | 1 | 461.00 | 4 |
| 16-dyadic-fractions | 1 | 0.42 | 4 |
| 17-leaving-tie | 2 | 59.00 | 5 |
| 18-entering-tie | 1 | 48.00 | 5 |

## Confirmed presentation

The visible heading is `TRANSPORTATION MODEL OUTPUT SUMMARY`. Each screen
shows the entered title, `Final Iteration No.`, and
`Objective Value (minimum cost)`. The table columns are `From`, `To`,
`Amt Shipped`, `Obj Coeff`, and `Obj Contrib`, with source-major then
destination-major ordering in this corpus.

Every reviewed screen visibly has disabled Next Iteration and All Iterations,
an enabled Write to Printer button, View/Modify Input Data, MAIN Menu, Exit TORA,
and a horizontal scroll control. These observations establish visible states,
not button execution, hidden scroll content or successful printing.

The statement in [the earlier final specification](TRANSPORTATION_FINAL.md)
that the captured final route always reports iteration 1 is too broad. Cases
07, 10 and 17 visibly report iteration 2. Their retained REA capture IDs are,
respectively:

- `ev_2acf3f7dbcdeb5ff64bb63c6a9a7aa84d0da116cda72ce7b1f34667556c756b6`
- `ev_1b8f9c5b4a79172f1a88179fd160ae14d74924d87d48d10998d84137e94f5a1a`
- `ev_88d2b9312ef95d5a9f46ebd1fc3ba3f58faaa7a3dd8a619d063ccb5ab693c4c9`

The pilot's positive shipment rows cannot establish positive-only filtering.
Case 05 visibly includes S1→D3 amount `0`; case 06 includes S1→D1 amount `0`;
case 17 includes S3→D3 amount `0`. These explicit zero rows agree with the
nonblank zero basic shipments in their captured terminal tableaus.

## Bounded evidence for final-route start choice

A comparison between these reviewed final rows and the independently captured
`leastcost/numbers-draft.json` terminal tables finds agreement in all seventeen
cases for:

- final iteration number versus the observed Least-cost tableau count;
- objective string;
- ordered source/destination rows versus nonblank terminal shipment cells;
- cost strings in those rows.

For fourteen integer-quantity profiles, displayed shipment amounts also agree.
Cases 08 and 16 retain the distinct final-summary amount display described
below. The comparison reads existing screen transcriptions only and performs
no optimization or contribution calculation.

Together with the different Vogel optimum basis in case 14, these observations
support a Least-cost-start hypothesis for the final route in this corpus.
They do not prove general start selection or identify original internal code.
A later implementation must preserve the reviewed outputs and resolve any
newly discriminating case with a focused original-app probe.

## Fractional display discrepancies remain observable behavior

Case 08 shows final amounts `0`, `2`, `0`, `3` for S1→D1, S1→D3, S2→D1 and
S2→D2. Its captured Least-cost terminal shipments are `0.75`, `1.75`, `0.75`,
`2.75`, while final contributions visibly remain `1.69`, `3.06`, `4.13`,
`11.69` and the objective remains `20.56`. Retained capture:
`ev_63f26bdb8d3535066765d4a74434c466f3b5afeb89974428e7b00392d42df88e`.

Case 16 shows all four final amounts as `0`, while captured terminal shipments
are `0.25`, `0.125`, `0.125`, `0.5`. Final costs are `0.13`, `0.25`, `0.38`,
`0.63`, contributions are `0.03`, `0.03`, `0.05`, `0.31`, and objective is
`0.42`. Retained capture:
`ev_bb697e726c74fff4977ddb482e67e47a3155d0f2d1ae9f0b35253b527b7edc80`.

The screenshots establish these strings. They do not establish a general
rounding/conversion rule. Do not calculate contributions from the displayed
integer amount or silently change the original discrepancy.

## Remaining evidence before broader acceptance

General final-start selection, fractional format thresholds, aliases/code pages,
scientific/N/D variants, imbalanced/invalid models, alternate-optimum notices,
large table scrolling and actual printing/pagination remain unverified here.
The existing small-fraction/clipped profile excluded from the seventeen-case
corpus is not promoted to reviewed acceptance by this pass. Date/copyright text,
font metrics and frame identity are outside this numerical/presentation review.

The reviewed table cells are suitable as the independent starting corpus for
a later tests-first final-summary cycle. They do not make that feature delivered
or authorize treating unrelated profile gaps as resolved.
