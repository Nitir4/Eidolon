# TORA LP text format observed in direct round trips

Nineteen files have now been opened, saved with TORA's `File → Save As`, read
as bytes, and reopened by TORA. All nineteen saved byte sequences are identical
to their staged inputs. Evidence is under `reference/tora-captures/roundtrips/`:
the fifteen normal/abnormal LP corpus fixtures plus four targeted probes.
`observations.json` in each directory records byte count, record differences,
and reopening. These are direct compatibility observations, not an inference
from the executable. A `tora-saved.txt` copy is also present in each corpus and
method directory for the fifteen fixtures.

## Framing and header

Observed files are ASCII-compatible text with CRLF after every record,
including the last record. Counts/settings are unquoted; title, objective sense,
and grid-cell values are quoted. Numeric grid cells are serialized as strings.

| One-based record | Observed content | Interpretation/status |
| --- | --- | --- |
| 1 | `3000` | LP-file marker; substituting 9999 produces the captured module-format Retry warning |
| 2 | `5` or `4` | Input-format N setting, corresponding to the observed notation controls |
| 3 | `2` or `3` | Input-format D setting |
| 4 | `1` or `2` | Decimal = 1, scientific = 2; independently verified by creating and saving both modes through the UI |
| 5 | Quoted title | Problem title |
| 6 | Integer | Number of decision variables n |
| 7 | Integer | Number of constraints m |
| 8 | `"Maximize"` or `"Minimize"` | Objective sense |

The saved smoke problem contains 281 bytes. The saved normal fixture contains
271 bytes; its different title accounts for the difference. Do not assume
that every file has the smoke file's byte count or precision.

## Grid record order

Following the eight header records are `(m + 5) × (n + 2)` quoted cells in
row-major order. Within a row, the n variable cells precede the relation cell
and RHS cell. The screen's row-description column is not serialized here.

| Serialized row | n variable cells | Relation/RHS cells |
| --- | --- | --- |
| 0 | Variable names; blank means default `x1`, `x2`, … | Empty in observed files |
| 1 | Objective coefficients | Empty in observed files |
| 2 through m+1 | Constraint coefficients | Padded relation (`        <=`, `        >=`, or `        =`) and RHS string |
| m+2 | Lower bounds | Empty in observed files |
| m+3 | Upper bounds, including literal `infinity` | Empty in observed files |
| m+4 | Unrestricted flags, observed `n` and `y` | Empty in observed files |

The 2-variable/3-constraint files have 40 records. The directly saved 1-variable/
1-constraint probe has 26 records, and the 3-variable/2-constraint probe has
43 records, matching this ordering. The one-variable probe shows `decision`
in the `Var. Name` row while its column header remains `x1`; that establishes
the previously unidentified first grid row.

## Lexical preservation

TORA preserved `1.234567`, `1.5`, finite lower/upper bounds `1`/`2.5`, a named
variable, negative coefficients and RHS values, an unrestricted `y` flag, and
scientific strings such as `8.000e0` through direct Save As/reopen. It also
preserved empty quoted cells, relation padding, and open upper bounds. Thus
opening/saving an existing file does not automatically round every numeric
string to D decimal places. The codec design must retain these lexemes and
reserved cells while exposing parsed numeric values to the model.

Two further models were created through the UI and saved by TORA. With N=5,
D=2, decimal mode writes record 4 as `1` and zero cells as `0.00`;
scientific mode writes record 4 as `2` and zero cells as `0.00000e+00`.
Evidence: `reference/tora-captures/probes/targeted-20261004/new-decimal/`
and `new-scientific/`, including selected radio controls, initialized grids,
save dialogs and original saved bytes. Scientific input therefore uses N
mantissa decimal places for these zero cells; D is the exponent field setting.

## Required parser/writer behavior and unresolved edges

Implement no product codec before its Foundation 4 gate. Its eventual reader
must preserve ordered records and produce explicit diagnostics for malformed
counts/cells. Its writer must produce approved TORA-compatible CRLF/quoted
records and preserve unknown reserved fields during unchanged round trips.
Do not infer arbitrary quote escaping, encoding, whitespace tolerance, record
limits, or malformed-file messages from these successful files.

Direct error/framing probes: marker 9999 triggers the module-mismatch Retry
warning; a 15-record file leaves its grid empty; an extra quoted record is
ignored on open; LF-only framing leaves title/count fields blank. See the
original attempted bytes/screenshots in `probes/file-errors-20261004/`.
These are observations for specific inputs, not a general malformed-input grammar.

Still unverified: omitted filename suffix behavior, titles/names containing
commas or quotes, non-ASCII encoding, other extra/truncated lengths,
invalid bounds/sign flags, read-only behavior, and Save versus Save As edge
cases. Actual messages and recovery paths belong in `tora-analysis/FILE_BEHAVIOR.md`.
