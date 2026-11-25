# JSON and TXT model inputs

Open these files through the module's **File → Open** (Assignment also has an
Open button). Save As selects JSON for `.json` and TXT for `.txt`. Parsing is
transactional: invalid files leave the previous model and solution installed.
All modules limit files to 4 MiB and dimensions to 1–100.

JSON schemas are native Eidolon formats. Unknown fields, nonfinite numbers,
invalid types/counts/indices and unsupported versions are rejected. Numeric
arrays contain JSON numbers; infinity is permitted only in LP bounds, using
the strings `"infinity"` and `"-infinity"`. `numberFormat` is `[notation,N,D]`,
where notation 0 is decimal and 1 is scientific. It does not round imported data.

Existing TORA TXT readers remain in use: LP marker `3000`, Transportation `4000`,
Games `9000`. Their record framing and quoted-field rules are unchanged.
**The native TXT formats below are not claimed to be accepted by TORA.**

## Assignment JSON

Required fields: `format:"ToraCloneAssignment"`, `version:1`, `title`,
`sense:"min"|"max"`, `numberFormat`, `workers`, `jobs`, rectangular `costs`.
The historical format identifier is retained for compatibility.

Optional fields:

- `forbidden:[[worker,job],...]`: **one-based** indices of absent edges.
- `dummyWorkers:[...]`, `dummyJobs:[...]`: one-based indices identifying explicitly
  supplied dummy rows/columns. These do not overwrite their entered costs.

Without `forbidden`, every finite cost is an admissible edge, including 1000.
Titles and large numbers never imply restrictions. Enter **X** in the input grid
to forbid a pair. Enter a finite number to permit it again. Stored costs under
forbidden entries are retained placeholders and do not enter the optimization.

Rectangles are padded internally to a square with zero-cost dummy rows/columns.
Exactly the smaller dimension's number of supplied-matrix pairs are selected;
extra workers are unassigned or extra jobs unused. An admissible complete padded
matching must exist, or the result is **infeasible**. No dummy identities are
inferred from names or costs.

## Native Assignment TXT, version 1

UTF-8, whitespace-separated records, with a fixed field order:

```text
EIDOLON ASSIGNMENT 1
title "26-Q1"
sense min
numberFormat 0 5 2
workers 3
"W1"
"W2"
"W3"
jobs 3
"J1"
"J2"
"J3"
dummyWorkers 0
dummyJobs 0
forbidden 3
1 2
2 3
3 1
costs 3 3
4 0 3
2 1 0
0 3 2
end
```

Counts follow `workers`, `jobs`, `dummyWorkers`, `dummyJobs`, and `forbidden`.
Dummy indices follow their count on the same record (for example `dummyJobs 1 2`).
Each forbidden record contains worker and job indices. `costs` gives row and
column counts, followed by numeric rows. A zero beneath a forbidden entry is
only a stored placeholder. Quoted title/labels use backslash escaping for a
quote or backslash (`\"`, `\\`). Extra trailing tokens are rejected.

## Validating LP-equivalent Assignment TXT

`decodeAssignmentLp` reuses `compat::ToraLpCodec` and accepts only a complete,
validated **square row-major** assignment structure:

- `x1=x11, x2=x12, ..., x(n*n)=xnn` in the entered column order.
- First n constraints are ordered worker equations, each exactly the appropriate
  consecutive block of ones, RHS 1. Next n are ordered job equations, each the
  corresponding strided ones, RHS 1. Other coefficients are zero.
- Variables are nonnegative and not unrestricted, with upper bound 1 or infinity.
- Additional constraints must be individual `xk=0` equations. These become
  explicit forbidden pairs, regardless of the objective coefficient of xk.
- Direction and original cost coefficients are preserved. The input page reports
  the validated variable-to-worker/job mapping and constraint blocks.

An arbitrary LP, a reordered/ambiguous mapping, side constraints or a rectangular
LP that would require inferred dummy identities is rejected with a mapping
message. Use native Assignment JSON/TXT to supply an explicit model. Conversion
does not infer restrictions from cost 1000 or a title. Automatic reconstruction
of arbitrary renamed, permuted or redundant LP formulations is unsupported.

## Linear Programming JSON

Required fields: `format:"EidolonLP"`, `version:1`, `title`, `sense:"min"|"max"`,
`numberFormat`, `objective`, `variables`, `constraints`.

Each variable has `{ "name":..., "lower":..., "upper":..., "unrestricted":... }`.
Each constraint has `{ "coefficients":[...], "relation":"<="|">="|"=", "rhs":... }`.
All vectors must match dimensions. Use lower `"-infinity"` and upper `"infinity"`
for a free variable. Inconsistent combinations that the existing editor would
normalize are rejected. Exact numeric spellings are seeded through the existing
TXT parser so coefficients/bounds retain double precision independent of display
digits. LP text labels retain the existing Windows-1252 profile; unrepresentable
Unicode labels are rejected rather than replaced.

LP TXT is the existing TORA `3000` format, not JSON with another extension.

## Transportation JSON and extended native TXT

Required fields: `format:"EidolonTransportation"`, `version:1`, `title`,
`sense:"min"|"max"`, `numberFormat`, `sources`, `destinations`, `supply`, `demand`,
`costs`. Optional `dummySources` and `dummyDestinations` contain one-based indices.
Supply/demand are nonnegative and all costs finite. Explicit dummy indices label
supplied model entries; no automatic balancing or zero-cost conversion occurs.
The existing solver's numerical profile and balanced-model requirements remain.

Minimization without dummy metadata writes the existing `4000` TXT format.
Maximization or explicit dummy metadata writes **EIDOLON TRANSPORTATION 1**:
the same seven header records (with that new marker in place of `4000`), then
three extra unquoted records: `min` or `max`, the dummy-source count and indices,
and the dummy-destination count and indices. The original quoted grid follows.
CRLF framing and ASCII text restrictions remain those of the existing parser.

Maximization solves the negated-cost minimization problem and maps objective,
potentials and reduced costs back to the original direction. Supplied coefficients,
names, supplies/demands and dummy metadata remain unchanged. Insert/delete shifts
dummy indices; deleting a declared dummy removes its metadata.

## Games JSON and labelled native TXT

Required fields: `format:"EidolonGame"`, `version:1`, `title`, `numberFormat`,
rectangular `payoff` to maximizing row player A. Column player B minimizes.
Optional `rowNames` and `columnNames` must be supplied together and match sizes.
Without them, A1… and B1… are used. Finite negative/zero values are permitted.

Default-label TXT retains `9000`. Custom labels write **EIDOLON GAME 1**:
the existing seven Game header records with the new marker, followed by one
quoted row label per strategy, one quoted column label per strategy, then the
existing row-major quoted payoff records. Doubled quotes escape embedded quotes.
Native Game text is UTF-8; existing `9000` decoding/roundtrip remains unchanged.

## Paired examples and original hard restrictions

The [examples/teaching directory](../examples/teaching) contains paired files:

| Model | JSON | TXT | Independently expected result |
|---|---|---|---|
| 26-Q1 | [JSON](../examples/teaching/26-Q1.assignment.json) | [TXT](../examples/teaching/26-Q1.assignment.txt) | W1→J1, W2→J2, W3→J3; minimum 7 |
| 26-Q2 | [JSON](../examples/teaching/26-Q2.assignment.json) | [TXT](../examples/teaching/26-Q2.assignment.txt) | W1→J2, W2→J3, W3→J1; minimum 5 |
| 28-Q1 | [JSON](../examples/teaching/28-Q1.assignment.json) | [TXT](../examples/teaching/28-Q1.assignment.txt) | Infeasible: W1 and W2 both require J1 |
| 28-Q2 | [JSON](../examples/teaching/28-Q2.assignment.json) | [TXT](../examples/teaching/28-Q2.assignment.txt) | Infeasible: W1 and W2 both require J2 |
| Rectangular negative profit | [JSON](../examples/teaching/rectangular-max-negative.assignment.json) | [TXT](../examples/teaching/rectangular-max-negative.assignment.txt) | Maximum -5 |
| Explicit dummy | [JSON](../examples/teaching/explicit-dummy.assignment.json) | [TXT](../examples/teaching/explicit-dummy.assignment.txt) | Minimum 3 |
| Finite 1000 costs | [JSON](../examples/teaching/finite-penalty-legacy.assignment.json) | [TXT](../examples/teaching/finite-penalty-legacy.assignment.txt) | Minimum 2000, not infeasible |
| LP | [JSON](../examples/teaching/lp.json) | [TXT](../examples/teaching/lp.txt) | (-1,0), minimum -1 |
| Transportation | [JSON](../examples/teaching/transportation.json) | [TXT](../examples/teaching/transportation.txt) | Minimum 18 |
| Transportation maximum / dummy | [JSON](../examples/teaching/transportation-max-dummy.json) | [TXT](../examples/teaching/transportation-max-dummy.txt) | Maximum 20 |
| Labelled matching pennies | [JSON](../examples/teaching/game.json) | [TXT](../examples/teaching/game.txt) | Both strategies (1/2,1/2), value 0 |

The original restriction matrices above came directly from the user's university
questions. No restrictions were guessed from legacy finite penalties. Other old
files are not silently migrated.

## Original question-bank dummy mappings

Eight verified native TXT alternatives are in `examples/teaching`:

- `17-Q1.transportation.txt` and `17-Q2.transportation.txt`: destination 3 is dummy.
- `18-Q1.transportation.txt` and `18-Q2.transportation.txt`: source 3 is dummy.
- `25-Q1.assignment.txt`: W3 is dummy; `25-Q2.assignment.txt`: J3 is dummy.
- `29-Q1.assignment.txt` and `29-Q2.assignment.txt`: J3 is dummy.

The original Assignment LP files contain padded matrices without dummy flags;
Transportation 17/18 retain DUMMY labels but lack typed dummy indices. These native
alternatives preserve the original matrices and add the verified role metadata.
Use LP for cases 21/22: case 21 requires forbidden-route zero-fixing constraints;
case 22's maximum-profit LP files are not automatically converted to Transportation.
The local original question bank and generated handoff ZIP are excluded from Git.
