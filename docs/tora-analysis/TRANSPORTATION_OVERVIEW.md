# Transportation investigation and first native slice

The user authorized this extension after the captured LP delivery. TORA's
Windows 1.00 reference was operated under Wine at 800×600. REA 6.4.0 records
observer-process output and selected files; the existing Win32 GUI driver
performs the interactions. REA's Linux CLI does not provide macOS screen/AX
control or supply a solver. The independent X server lets the same TORA
instance serve multiple cases without occupying the user's desktop.

Only UI behavior, saved files and ordinary print interfaces are observed.
No binary analysis, resources, icons or help text are extracted. Raw material
is private and excluded from Git and release artifacts.

## Observed routes and corpus

MAIN Menu → Transportation model opens new/existing selection and notation
settings, then an editable input grid. SOLVE/MODIFY → Solve Problem offers
Final solution and Iterations. Iterations offers North-west corner start,
Least-cost start and Vogel's start. Each opens the tableau directly. Optional
multiplier initialization/manual loops and Write to Printer are present;
their complete behavior is not established.

Eighteen inputs were run through all three starts and Final solution. Seventeen
have complete independent numerical transcriptions (76 tableaus). The remaining
small-fraction probe is captured but has clipped long values, so it is excluded
from numerical acceptance. Existing completed captures were reused on resume.

| Case | North-west | Least-cost | Vogel | Optimal objective |
|---|---:|---:|---:|---:|
| 01-normal | 3 | 1 | 1 | 150.00 |
| 02-reverse-costs | 2 | 1 | 1 | 135.00 |
| 03-all-ties | 1 | 1 | 1 | 50.00 |
| 04-degenerate | 2 | 1 | 1 | 165.00 |
| 05-zero-source | 2 | 1 | 1 | 270.00 |
| 06-zero-destination | 2 | 1 | 1 | 150.00 |
| 07-zero-costs | 3 | 2 | 1 | 25.00 |
| 08-decimal-quantities | 3 | 1 | 1 | 20.56 |
| 09-negative-costs | 2 | 1 | 1 | -70.00 |
| 10-cost-ties | 2 | 2 | 1 | 64.00 |
| 11-larger-quantities | 2 | 1 | 1 | 1910.00 |
| 12-vogel-ties | 1 | 1 | 1 | 124.00 |
| 13-three-sources | 5 | 1 | 1 | 190.00 |
| 14-digits | 1 | 1 | 1 | 461.00 |
| 16-dyadic-fractions | 1 | 1 | 1 | 0.42 |
| 17-leaving-tie | 2 | 2 | 2 | 59.00 |
| 18-entering-tie | 4 | 1 | 1 | 48.00 |

Private provenance: `reference/tora-captures/transportation/corpus/<case>/`
contains the authentic `tora-saved.txt`, input, initial/terminal/scroll screens,
per-method number transcriptions with screenshot hashes, and capture markers.
Unique REA attempt evidence preserves failed attempts; child exit status is
checked separately from REA's capture-process success. The independent
`transcription-audit.json` checks hashes and arithmetic consistency. It does
not generate expected solver output.

## Implemented preview scope

The first native slice is read-only opening of default-format, balanced saved
Transportation models, example loading, and automatic North-west/MODI traces
with u1=0. Seventeen independent cases and 38 tableaus form the exact golden
contract. The existing module registry accepts this module without changing
the LP model or numerical implementation. Native layout uses Qt Widgets.

New/edit/save, Least-cost/Vogel algorithms, a separate final-summary screen,
manual loop choices, alternate multiplier initialization and Transportation
printing are not implemented. Unsupported quantity/format settings produce an
explicit error. This slice is not a complete Transportation clone.

## Unknowns / things I could not verify

Exact dimension/validation limits, unbalanced handling, prohibited routes,
other notation settings/code pages, all Least-cost/Vogel tie rules, manual
loops, alternate multipliers, cycling behavior and printer pagination.
Nonbinary small-fraction values are clipped in the original grid. The captured
fractional final-summary amount discrepancy is documented separately; no
uncaptured general rounding rule is asserted.

The native preview explicitly rejects non-ASCII legacy text until its code page
is verified, and caps a displayed trace at250,000 cells to bound widget allocation.
These are preview safeguards, not original TORA limits.
