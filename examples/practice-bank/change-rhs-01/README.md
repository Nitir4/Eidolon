# change-rhs-01

Methods: LP / Change RHS, LP / Dual Revised (RHS).

Open [input.txt](input.txt) in the matching module.

Maximize `z = 3*x1 + 2*x2`.

Subject to:

- `1*x1 + 1*x2 <= 4`
- `1*x1 + 0*x2 <= 2`
- `0*x1 + 1*x2 <= 3`

Bounds: 0 <= x1 <= infinity, 0 <= x2 <= infinity.

Changed objective: `3*x1 + 2*x2`. Changed RHS: `[6, 2, 3]`.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "before": {
    "status": "optimal",
    "objective": 10.0,
    "objective_exact": "10",
    "one_solution": [
      2.0,
      2.0
    ],
    "one_solution_exact": [
      "2",
      "2"
    ],
    "check": "Objective and feasibility are authoritative; tied solutions may differ."
  },
  "after": {
    "status": "optimal",
    "objective": 12.0,
    "objective_exact": "12",
    "one_solution": [
      2.0,
      3.0
    ],
    "one_solution_exact": [
      "2",
      "3"
    ],
    "check": "Objective and feasibility are authoritative; tied solutions may differ."
  }
}
```

Open baseline input, then Class extensions. Enter changed objective or RHS from answer.json. Compare before/after. Original comparison requires manually changing and re-solving a copy.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
