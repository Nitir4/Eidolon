# bounded-04

Methods: LP / Bounded variables.

Open [input.txt](input.txt) in the matching module.

Maximize `z = 4*x1 + 7*x2`.

Subject to:

- `1*x1 + 2*x2 <= 10`
- `3*x1 + 1*x2 <= 12`
- `0*x1 + 1*x2 <= 4`

Bounds: 0 <= x1 <= 5, 0 <= x2 <= 4.0.

Expected answer (independent mathematics; original comparison pending):

```json
{
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
}
```



[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
