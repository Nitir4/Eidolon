# Transportation printing

This contract describes observed default-format output from ordinary TORA
Write to Printer commands. Original jobs remain private under
reference/tora-captures/transportation/printing/. No original binaries, pages,
branding or fonts are included in the application.

## Observed workflows

One iteration command submits one job: original input first, followed by one
page for each **shown** tableau. Printing the initial North-west view produces
two pages even though its solver has computed three tableaus. After All,
the same model produces four pages. Least-cost and Vogel examples each produce
three pages. Only the first tableau page carries the method heading and title;
later pages start with their iteration number and objective.

One final-summary command submits **two separate jobs**, in order: original
input and final summary. Four final examples each have one page per job.
The observed input File menu has no separate print command.

The independent fixture contains eleven workflows, fifteen jobs and twenty-six
pages. Each record carries the original PS hash, derived PDF hash, accepted
REA evidence id and authentic input identity:
[transportation-printing.json](../../tests/fixtures/transportation-printing.json).
It covers initial/all North-west, refreshed destination anchor, Least-cost,
Vogel, normal/fractional/alias/wide final output, and alias/wide North-west.

Representative evidence includes
ev_8957e6f6c8bc265ddeabb3d6c021fc8eeec29805cea0d14c85956e69338f1ca3
(final job boundary),
ev_997d06d513fde97ac4a2eacca8db2280836c560065cd84a9c8ce063577248f48
(shown North-west history), and
ev_6e6ddef84d283d15a852a67621b64a6bdba3a43a415963bd834cdc8676309f3c
(Vogel). All accepted producer children exited zero; failed probes are retained
separately and do not establish behavior.

## Content and presentation

Printed input uses lowercase s/d ids, lowercase aliases, fixed two-decimal
costs and quantities, and Supply/Demand labels. Input aliases are not
lowercased in the model or saved file. In the observed 2×9 case, input uses
D1–D5 then D6–D9 bands on one page; Name appears in the first band and Supply
only in the last. Source ids and Demand repeat.

Tableau labels and aliases preserve capitalization. Each source has cost,
shipment and reduced-value rows; basic zero shipments print explicitly while
nonbasic shipments remain blank. Supply/demand and shipments use the accepted
quantity strings. The first explicitly refreshed anchor prints short u/v
strings; unrefreshed/later multipliers use fixed decimals. Manual-loop and
selected-anchor colors are not printed.

The wide tableau uses D1–D4 in its first band and D5–D9 in its continuation.
Name, u multipliers and Demand label appear only in the first band. Source ids
repeat and Supply appears in the last band. Both bands fit one observed page.

Final output has the five From/To/Amt Shipped/Obj Coeff/Obj Contrib columns,
basic zero rows, terminal iteration and objective. It follows
[the final-summary contract](TRANSPORTATION_FINAL.md): sub-unit amount display
does not replace actual quantities in cost contributions. Alias capitalization
is preserved with the documented native one-space separator policy.

Original branding/date are excluded from teaching-content expectations. The
input printer page uses the clone’s own name/date and an installed sans-serif
face. Physical geometry and separator-space counts are not pixel-equivalence
claims.

## Native execution flow

`src/desktop/transportation_printing.{hpp,cpp}` draws immutable model/display
data with QPainter/QPrinter. It does not read widgets to reconstruct numerical
results or modify solvers. `printTransportationTrace` takes the page’s revealed
count, not the full precomputed history. Final printing snapshots both model
and summary before entering native selection and submits two independent jobs.

The Write to Printer controls in `transportation_screens.cpp` are enabled only
when their output exists. The existing `requestLpPrint` boundary writes to
the default printer or offers native selection/PDF output when none exists.
Cancellation is preserved; cancelling the second final selection cannot retract
the first completed job. Error state or unavailable output returns failure.
This boundary reports rendering/submission state, not physical completion.

## Verification and diagnostic history

Tests were added before implementation. The initial new target failed on the
missing transportation_printing.hpp API. After implementation, eleven native
exports, job-boundary/cancellation, unavailable output and enabled commands pass.
The final focused suite passes 2/2 in 1.45 seconds, with seventeen Qt checks.
Its PDF verifier checks exact page counts, teaching-token multiplicity and
278 geometrically ordered rows, detecting row/cell swaps despite PDF traversal
order differences. Two original extraction artifacts were corrected only after
visual review: split 10.0 0 and Vogel ’s.

Local full desktop regression passed 100/100 in 124.59 seconds before the final
printer Error-state guard/error-test extension; final focused printer checks
passed afterward. Qt-free regression passed 38/38 in 3.83 seconds. Final-source
three-OS/package/install verification is still pending.

Actual Wayland acceptance covered twenty-six views with the LP model preserved.
The dedicated CUPS server initially rejected Qt PDF because its OraclePS sink
accepted original PostScript only. Server diagnostics show explicit
document-format rejection; failed zero-byte jobs were retained. The private
file sink was then configured with PDF and PostScript passthrough using the
[primary CUPS PPD filter specification](https://openprinting.github.io/cups/doc/spec-ppd.html#cupsFilter).
Native jobs48–50 delivered nonempty PDFs with 2/1/1 pages for initial trace,
final input and final summary. Host printer settings were untouched.

All twenty-six native PDF pages rendered without warnings in Poppler and were
reviewed, including full-size alias summary, anchored trace and wide pages.
The private Ghostscript10.08 path reported stream warnings and missing glyphs
for these Qt PDFs; that failed derivative is retained, with cause unresolved.
It is not counted as visual acceptance. Original PostScript conversions used
that runtime successfully. Physical printers, other drivers and universal PDF
viewer compatibility remain unverified.

Large/long-alias models, nondefault formats and printer-specific page geometry
outside this corpus are not established original behavior. Native continuation
pagination beyond observed sizes is an engineering policy; it is not evidence
of every legacy page break. Private evidence: build/transportation-printing/.


## Pre-policy CI and Actions shutdown

Source dc853b45fa57ea3a06a890c599948dbc7cd2ce3a ran in CI38454278600 before the new Actions approval restriction. Linux/macOS passed100/100. Windows passed99/100; desktop.graphical.scale-1timed out. All new printer export/PDF checks passed on all three OSes, verifying17Qtchecks and26pages/278orderedrows/1143tokens perOS. Five available artifacts passed GitHub digest/ZIP checks; Windows package was withheld. This is a partial release gate, not full three-OS approval.

Actions are disabled for the repository under the user’s2026-10-14shutdown authorization. No rerun or enabling is allowed without specific explicit approval. Existing artifacts remain readable. Linux payload281hashes and actual Wayland/xcb startup passed. Partial evidence is build/transportation-printing-dc853b4/{printer-ci-verified,partial-delivery-audit,linux-payload-verified}.json. Later local-only settings work must not be described as cross-platform CI verified.

The reviewed Linux package dc853b4is installed via /home/nitir505/.local/bin/tora-clone, SHA256f0c07b3611f101cd7fed4b5d0f078e77bba83b6dc1f2b0c45bef4a3389cecfb8. Its exact native Wayland window was verified; prior releases/windows preserved. Installation is Linux-only and does not close the failed full Windows gate.
