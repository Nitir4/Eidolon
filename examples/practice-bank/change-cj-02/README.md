# change-cj-02

Methods: LP / Change Cj.

Open [input.txt](input.txt) in the matching module.

Maximize `z = 5*x1 + 4*x2`.

Subject to:

- `6*x1 + 4*x2 <= 24`
- `1*x1 + 2*x2 <= 6`
- `-1*x1 + 1*x2 <= 1`

Bounds: 0 <= x1 <= infinity, 0 <= x2 <= infinity.

Changed objective: `8*x1 + 3*x2`. Changed RHS: `[24, 6, 1]`.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "before": {
    "status": "optimal",
    "objective": 21.0,
    "objective_exact": "21",
    "one_solution": [
      3.0,
      1.5
    ],
    "one_solution_exact": [
      "3",
      "3/2"
    ],
    "check": "Objective and feasibility are authoritative; tied solutions may differ."
  },
  "after": {
    "status": "optimal",
    "objective": 32.0,
    "objective_exact": "32",
    "one_solution": [
      4.0,
      0.0
    ],
    "one_solution_exact": [
      "4",
      "0"
    ],
    "check": "Objective and feasibility are authoritative; tied solutions may differ."
  }
}
```

Open baseline input, then Class extensions. Enter changed objective or RHS from answer.json. Compare before/after. Original comparison requires manually changing and re-solving a copy.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
