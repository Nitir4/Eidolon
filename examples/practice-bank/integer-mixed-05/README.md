# integer-mixed-05

Methods: Integer / Mixed.

Open [input.txt](input.txt) in the matching module.

Maximize `z = 7*x1 + 5*x2`.

Subject to:

- `2*x1 + 3*x2 <= 18`
- `3*x1 + 1*x2 <= 15`
- `1*x1 + 0*x2 <= 4`

Bounds: 0 <= x1 <= infinity, 0 <= x2 <= infinity.

Integer variables: x1. Other variables are continuous.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "status": "optimal",
  "objective": 43.0,
  "objective_exact": "43",
  "one_solution": [
    4.0,
    3.0
  ],
  "one_solution_exact": [
    "4",
    "3"
  ],
  "check": "Objective and feasibility are authoritative; tied solutions may differ."
}
```

Integer enumeration bounds 0..30 cover these small bounded feasible regions.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
