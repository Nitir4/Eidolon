# change-rhs-04

Methods: LP / Change RHS, LP / Dual Revised (RHS).

Open [input.txt](input.txt) in the matching module.

Maximize `z = 4*x1 + 7*x2`.

Subject to:

- `1*x1 + 2*x2 <= 10`
- `3*x1 + 1*x2 <= 12`
- `0*x1 + 1*x2 <= 4`

Bounds: 0 <= x1 <= infinity, 0 <= x2 <= infinity.

Changed objective: `4*x1 + 7*x2`. Changed RHS: `[12, 12, 4]`.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "before": {
    "status": "optimal",
    "objective": 36.4,
    "objective_exact": "182/5",
    "one_solution": [
      2.8,
      3.6
    ],
    "one_solution_exact": [
      "14/5",
      "18/5"
    ],
    "check": "Objective and feasibility are authoritative; tied solutions may differ."
  },
  "after": {
    "status": "optimal",
    "objective": 38.666666666666664,
    "objective_exact": "116/3",
    "one_solution": [
      2.6666666666666665,
      4.0
    ],
    "one_solution_exact": [
      "8/3",
      "4"
    ],
    "check": "Objective and feasibility are authoritative; tied solutions may differ."
  }
}
```

Open baseline input, then Class extensions. Enter changed objective or RHS from answer.json. Compare before/after. Original comparison requires manually changing and re-solving a copy.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
