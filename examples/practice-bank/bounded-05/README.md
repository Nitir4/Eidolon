# bounded-05

Methods: LP / Bounded variables.

Open [input.txt](input.txt) in the matching module.

Maximize `z = 7*x1 + 5*x2`.

Subject to:

- `2*x1 + 3*x2 <= 18`
- `3*x1 + 1*x2 <= 15`
- `1*x1 + 0*x2 <= 4`

Bounds: 0 <= x1 <= 6, 0 <= x2 <= 4.5.

Expected answer (independent mathematics; original comparison pending):

```json
{
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
}
```



[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
