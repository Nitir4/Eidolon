# Platform and original-print followup, 18 October 2026

Implementation mode remains confirmed. All eight original MAIN families and the
Assignment/Cj-RHS/Dual Revised extensions are implemented. This followup closes
specific portability and printing gaps; it does not claim exhaustive original
numeric, search-order, color or page-layout equivalence.

## Applied changes

Queuing mean system time now uses `W = Wq + 1/mu`. Computing `Ls/throughput`
lost the representable service time for lambda=1e-300 and mu=1e300 when occupancy
underflowed. MSVC represents `long double` as binary64, unlike the host Linux
compiler. A Linux translation-unit approximation narrowed to binary64 reproduced
the failure before the fix and passed afterwards. This is not native MSVC evidence.
Regression coverage includes 1, 2 and 10000 servers. Relevant files are
`src/queuing/queuing.cpp` and `tests/queuing.cpp`.

Guided Integer printing now includes pending subproblems, their parent/depth and
complete defining branch paths, explicitly labeled as unsolved LP relaxations.
Printing does not solve them or invent objective bounds. The regression failed
before the fix and now covers initial, split, nested and completed states.
Relevant files are `src/desktop/integer_screens.cpp` and `tests/desktop_integer.cpp`.
Four scale checks passed 4/4 in 4.34 seconds (11 QtTest slots each); 20 PDFs,
28 pages and 3508 words passed blank-page and page-bound checks. The nested case
fits one page at every tested scale. Receipt: `build/platform-followup/parity/verification.json`.
Host programming-font ligatures can omit inequality operators from extracted PDF
text even when visually correct; the new semantic test explicitly uses sans-serif.
Product font selection is unchanged.

## Original observations through REA

An owned private CUPS server and file-backed PostScript printer were used on a
separate Wine display. No physical jobs or host printer settings were changed.
Raw captures, original PostScript and converted pages stay private under ignored
`reference/` and `build/`. These successful jobs supersede the earlier blanket
printing limitation: the prior error 482 remains evidence of the old environment.

| Scenario | Observed result | REA evidence |
| --- | --- | --- |
| Equations, 2x+y=5 and x-y=1 | One printed page: solution (2,1), determinant -3, inverse rounded to .33/.33/.33/-.67 | `ev_91951f33d1b9dc08624d7c9cf4151b5d5bb60aff3e5985af289b5c5c087ecc88` |
| Guided Integer root | Model plus tree page; N10 bound 7.50 | `ev_14158edaab77414224e775a0ab88e73737a8d1abcf72568a2c8513129cecb720` |
| Guided Integer x2 split | Printed children N20 x2<=1 bound 7.25 and N21 x2>=2 bound 5.50 | `ev_a864a487cc875d85697a0fd728672fb028b260138092526fb2588d5ac77f4c55` |
| Integer after switching to automated | Screen shows optimum 6 at (2,0), found at iteration 9 and verified at 11; emitted job says no feasible solution and has an empty tree | `ev_d81fd974df9c65151a5da7a6258b125f53efac29bf4b998515a9ef2b4650afb7` |
| PERT three complete contiguous activity rows | Printed mean/variance rows: (3,.44), (4,.44), (1,.00); calculations show intermediate event means 3 and 7, then VB9 at final event 4 | `ev_795c47cb3893762de24702bfe84bd0ad5a97c9b0ef19967eeb3aa230a1866be1` |

The guided original prints also say no feasible solution before an incumbent
exists. That statement does not establish model infeasibility and is not copied
into native output. Original children already have solved LP bounds; native
frontier children are deliberately incremental and honestly labeled unsolved.
The automated screen/print discrepancy is observed; its internal cause is unknown.

The changed PERT input has exactly three complete activity records and no blank
sentinel. The mathematical project duration is 8, but original final duration 8
was not observed. The fault at the terminal event supports a terminal-handling
hypothesis, not recovery of the original implementation. Dismissal caused the
owned target to exit (`ev_407e06324d6ea117dca267227fb7e41a0832a476c51c4419d2333c67d9129691`).
No unchanged retry is warranted.

## Platform acceptance boundaries

The user confirmed that no physical devices are available and requested continued
software checks. Qt detects one eDP-1 panel and zero configured printers. Physical
printing and movement between differently scaled monitors remain unverified.

An isolated Ubuntu 24.04/glibc 2.39 environment with an official Qt 6.10.3 SDK is
being used for a full Release build, tests, packaging and contained copy/launch
checks. The first suite passed 152/161: eight failures were missing pdftotext;
the ninth assumed a single horizontal LP table band. DejaVu Sans legitimately
moves the Solution column into a second band without losing it. Rendered review
corrected the initial clipping hypothesis. Portable Python/pypdf extraction and
band-aware verification are being validated, without changing product fonts or
valid table layout. A packaging preflight also exposed SDK discovery hidden by
system Qt on the host: deployment now prepends qmake's QT_INSTALL_LIBS directory
to its loader path. Deployed startup checks still clear SDK loader variables. Results must be read from its final receipts; a running build is not a pass.
The older host-built preview requires glibc 2.43 and remains historical evidence.

No native Windows/macOS SDK is available locally. The existing three-OS workflow
is prepared in `build/platform-followup/cross-platform-plan.md`, with a sanitized
source export and path/hash audit. The user specifically approved this one workflow execution and temporary
Actions enablement, followed by disabling Actions again. Actions remain disabled
until final snapshot review; no commit, push, PR, tag, release or workflow execution
has occurred yet. The approval does not authorize rerunning a failed gate.

## References

- Microsoft floating-point representations: https://learn.microsoft.com/en-us/cpp/c-language/values?view=msvc-170
- Qt 6.10 Linux requirements: https://doc.qt.io/qt-6.10/linux.html
- Qt 6.10 supported platforms: https://doc.qt.io/qt-6.10/supported-platforms.html
