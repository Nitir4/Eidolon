# Unverified observations and module-gate obligations

The original 37-pair repair and later 17-pair repair are finished and empty.
Do not rerun them. Evidence coverage passes 79 full traces plus two expected
warnings, 273 tables, 7,711 numeric cells; 15 graphical and 15 final-summary
studies; 19 byte-identical TORA Save As/reopen round trips. Additional probes
cover finite bounds, manual pivots/alternative optima, notation, M acceptance,
row editing, zoom and specific malformed files.

These unknowns are part of the Gate 0 review package. They are not permission
requests and must not be silently promoted to verified compatibility. Gate 0
approval accepts the analysis/design and its explicit limits; it does not
waive the later requirement to resolve any ambiguity affecting a module's
implementation or comparisons.

| ID | Unverified behavior / observed limit | Must resolve before |
| --- | --- | --- |
| Q-01 | Hidden arithmetic precision, zero/near-tie thresholds, rounding at halfway values, negative-zero creation, cycling safeguard/iteration cap | Foundation 2 decisions and affected numerical module gates |
| Q-02 | N/D allowed extrema, invalid notation-field messages, large-width/exponent overflow, scientific formatting beyond N=5/D=2 | Formatter/input/output module gates |
| Q-03 | Undo enablement/restoration; column insert/delete/copy/paste; all advertised shortcuts, objective-cell editing, invalid relation/name/cell messages | Data-entry module gate |
| Q-04 | Read-only editing/saving, suffix defaults, overwrite/cancel, quote/comma/non-ASCII encoding, other malformed counts/records/cells | Foundation 4 / file-I/O gate |
| Q-05 | Graph eligibility for one-variable, finite-bound/unrestricted inputs; individual-constraint click states; general range/tick/dot/zoom rules | Graphical module gate |
| Q-06 | Unrestricted expansion, inconsistent-bound validation, reverse bound complements, redundant equality/artificial-basic cleanup | Model/preprocessing and affected solver gates |
| Q-07 | Direct applicable Dual infeasibility with no eligible entering column | Dual-simplex module gate |
| Q-08 | Successful table/graph printing, pagination and print dialog defaults | Printing/output module gate |
| Q-09 | Automatic Final solution's internal method/iteration accounting; bound/unrestricted sensitivity transformations | Final/sensitivity module gates |
| Q-10 | Complete independently reviewed golden labels/messages and every transcribed table, beyond automated coverage and reviewed glyph seeds | Tests-first step of each numerical/UI module |

Q-08 has an environment blocker: Write to Printer produced Run-time error
'482': Printer error under Wine, and acknowledging the dialog ended TORA.
A working printer configuration or Windows reference is required to observe
successful printing. The requested fallback is to generate a precise test
packet for the user to run if that reference remains unavailable. No successful
print behavior has been guessed.

Specific file errors are now observed: wrong marker 9999 produces the exact
module-format Retry warning, 15-record truncation leaves a blank grid, an
extra quoted record is ignored, LF-only framing leaves an uninitialized form.
These results do not establish all malformed-file behavior. M 0/-1/0.5/1e2/
bogus each reached an initial tableau; bogus visibly behaves like zero there.
This does not establish arbitrary text parsing or all terminal M behavior.

Revised simplex and Parametric analysis are absent from the complete visible
LP hierarchy; neither is included as an invented module. An additional path
would be studied if direct evidence reveals it. The M=100 infeasible case's
optimal notice with positive Rx5 is recorded legacy behavior, not an unresolved
transcription mismatch. The clone's implementation must preserve observed
compatibility explicitly rather than silently correcting that trace.

Foundation 2 narrows Q-01/Q-02: 65 reviewed display strings verify halfway
rounding, tiny-negative display, exponent padding, D=0/D=10 and scientific
N=0/1/2/5/10. Decimal N=0 triggers runtime error 5; 1e-100/1e100 triggers
runtime error 6. Exact policies, sources and remaining limits are recorded in
`tora-analysis/NUMERIC_FOUNDATION.md`. The Qt-free pivot primitive matches the
normal initial/next two tables. Solver thresholds/tie policies and iteration
caps are still required at solver gates; the primitive never selects a pivot.
