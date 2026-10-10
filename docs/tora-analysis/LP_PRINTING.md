# LP printing compatibility

This specification comes from actual TORA UI operations and the resulting
PostScript jobs, not from the clone's PDF output. It covers 25 observations:
23 successful print jobs and two final-output states without a print control.
Raw evidence remains private under
`reference/tora-captures/probes/printing/<case>/`. The public independent
transcription is `tests/fixtures/printing.json`.

## Purpose and workflow

Print Graph prints the graphical solution. Write to Printer prints the final
summary (including sensitivity) or the recorded simplex tableaus. Each successful
job includes original model data before the output. TORA sends directly to its
default printer; no additional selection dialog appeared in the captured jobs.
Infeasible and unbounded final summaries expose no Write to Printer control.

The reference ran in Wine on an isolated 800×600 Xwayland display, using a
private Wine prefix and a user-owned CUPS server on loopback port 18631. The
OraclePS queue produces PostScript without modifying the host's printer queues.
The initial Wine error 482 was resolved by starting this queue and registering
it through the ordinary Windows EnumPrinters/GetDefaultPrinter API.
This proves successful reference printing for this driver, not every driver.

## Original data and final summary

The first page has `LINEAR PROGRAM -- ORIGINAL DATA`, Title, x1/x2 variable
columns, Maximize/Minimize, Subject to, constraint relations/RHS, Lower Bound,
Upper Bound and `Unrestr'd (y/n)?`. Printed variables retain the original x labels
even when input aliases are present. The clone uses its own branding/date.

Summary output has `LINEAR PROGRAMMING OUTPUT SUMMARY`, Title, Final Iteration
No., and a compact Objective Value. Variable values, objective coefficients and
contributions precede RHS/slack rows and `***Sensitivity Analysis***`. The
captured print format uses five logical columns, including shorter tables.
The interactive alternative-optimum notice is omitted from the printout.
Decimal, D=3 and scientific formats are represented.

Negative-lower-bound slack printing uses restored original variable values.
TORA's screen retains a shifted-value quirk for these same cases. The clone
preserves this observed distinction in printing only. Additional captured
positive-cost, finite-upper, combined-bound and negative-RHS cases verify the
printer calculation and suffix orientation without changing screen goldens.

## Simplex traces and graph

Trace headings are plural `SIMPLEX TABLEAUS -- (...)`. All-slack, M, Two-phase,
Bounded and Dual prints retain ordered phase/iteration labels, Basic labels,
columns and numbers. Wide tables repeat Basic in successive horizontal bands.
Same-input M/Two-phase probes establish method-specific field widths: Two-phase
has wider fields and fewer columns per band. Default captured M bands contain
six numeric columns; Two-phase bands contain four. Empty bound-row continuation
bands are omitted. Captured jobs use two or three A4 pages.

Graph printing uses `LINEAR PROGRAMMING -- GRAPHICAL SOLUTION`, Title, Summary
of Optimal Solution, objective/x1/x2, then a predominantly monochrome native
plot with yellow constraint/objective labels. Its plot is slightly taller than
wide. Screen colors remain unchanged. The existing graph geometry goldens
verify mathematical content; visual inspection verifies readable print placement.

## Implementation and verification

`src/desktop/lp_printing.cpp` centralizes page composition and printer selection.
It uses QPrinter/QPainter and installed fonts; no TORA assets are copied. A
missing default printer opens Qt's native printer/PDF selection dialog rather
than reproducing the original crash. Cancellation leaves the document unchanged.

`tests/desktop_printing.cpp` exports native PDFs and checks controller/model
non-mutation, terminal print availability and cancellation. The cancellation
branch uses an injected selector in offscreen CI: Qt's macOS print panel enters
a native Cocoa modal loop, which cannot be driven by a QWidget timer on that
platform. Interactive application calls still use the actual native dialog.
The original local Linux dialog cancellation was also observed before this
portable test boundary was introduced.
`tests/verify_printing_pdf.py` independently checks exact page counts and
content against the transcription. It normalizes whitespace and equivalent
font ligatures only, never numeric values. Original-data text and output/band
ordering match exactly. Output-page text is concatenated because installed
font metrics can move a row across a page boundary. Graph summaries match
exactly; plot labels are compared as a multiset because drawing-command order
does not represent visual placement.

`tools/run_tora_print_probes.py` launches or attaches to TORA before operating
its UI, refuses an unavailable queue, and skips completed capture markers.
It observes actual jobs and records their provenance without calculating goldens.
Private PostScript is converted with an isolated Ghostscript runtime. Public
release artifacts contain no original prints, screenshots, saved files or binaries.

## Unknowns / things I could not verify

Physical printer output, other Windows printer drivers/paper sizes, exact
row-by-row page breaks across installed fonts, and pixel-identical graph layout
remain unverified. Compact objective formatting outside sampled values and
extreme field widths/large-model printer pagination remain outside this profile.
The native renderer's font-driven large-model banding is an engineering policy
until corresponding TORA captures exist. No physical printer or second monitor
was connected during these checks.
