# change-cj-05

Methods: LP / Change Cj.

Open [input.txt](input.txt) in the matching module.

Maximize `z = 7*x1 + 5*x2`.

Subject to:

- `2*x1 + 3*x2 <= 18`
- `3*x1 + 1*x2 <= 15`
- `1*x1 + 0*x2 <= 4`

Bounds: 0 <= x1 <= infinity, 0 <= x2 <= infinity.

Changed objective: `10*x1 + 4*x2`. Changed RHS: `[18, 15, 4]`.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "before": {
    "status": "optimal",
    "objective": 44.142857142857146,
    "objective_exact": "309/7",
    "one_solution": [
      3.857142857142857,
      3.4285714285714284
    ],
    "one_solution_exact": [
      "27/7",
      "24/7"
    ],
    "check": "Objective and feasibility are authoritative; tied solutions may differ."
  },
  "after": {
    "status": "optimal",
    "objective": 52.285714285714285,
    "objective_exact": "366/7",
    "one_solution": [
      3.857142857142857,
      3.4285714285714284
    ],
    "one_solution_exact": [
      "27/7",
      "24/7"
    ],
    "check": "Objective and feasibility are authoritative; tied solutions may differ."
  }
}
```

Open baseline input, then Class extensions. Enter changed objective or RHS from answer.json. Compare before/after. Original comparison requires manually changing and re-solving a copy.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
