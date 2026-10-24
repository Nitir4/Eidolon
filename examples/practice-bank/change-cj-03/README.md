# change-cj-03

Methods: LP / Change Cj.

Open [input.txt](input.txt) in the matching module.

Maximize `z = 2*x1 + 3*x2`.

Subject to:

- `2*x1 + 1*x2 <= 8`
- `1*x1 + 2*x2 <= 8`
- `1*x1 + 0*x2 <= 3`

Bounds: 0 <= x1 <= infinity, 0 <= x2 <= infinity.

Changed objective: `5*x1 + 2*x2`. Changed RHS: `[8, 8, 3]`.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "before": {
    "status": "optimal",
    "objective": 13.333333333333334,
    "objective_exact": "40/3",
    "one_solution": [
      2.6666666666666665,
      2.6666666666666665
    ],
    "one_solution_exact": [
      "8/3",
      "8/3"
    ],
    "check": "Objective and feasibility are authoritative; tied solutions may differ."
  },
  "after": {
    "status": "optimal",
    "objective": 19.0,
    "objective_exact": "19",
    "one_solution": [
      3.0,
      2.0
    ],
    "one_solution_exact": [
      "3",
      "2"
    ],
    "check": "Objective and feasibility are authoritative; tied solutions may differ."
  }
}
```

Open baseline input, then Class extensions. Enter changed objective or RHS from answer.json. Compare before/after. Original comparison requires manually changing and re-solving a copy.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
