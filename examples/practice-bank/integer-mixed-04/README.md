# integer-mixed-04

Methods: Integer / Mixed.

Open [input.txt](input.txt) in the matching module.

Maximize `z = 4*x1 + 7*x2`.

Subject to:

- `1*x1 + 2*x2 <= 10`
- `3*x1 + 1*x2 <= 12`
- `0*x1 + 1*x2 <= 4`

Bounds: 0 <= x1 <= infinity, 0 <= x2 <= infinity.

Integer variables: x1. Other variables are continuous.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "status": "optimal",
  "objective": 36.0,
  "objective_exact": "36",
  "one_solution": [
    2.0,
    4.0
  ],
  "one_solution_exact": [
    "2",
    "4"
  ],
  "check": "Objective and feasibility are authoritative; tied solutions may differ."
}
```

Integer enumeration bounds 0..30 cover these small bounded feasible regions.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
