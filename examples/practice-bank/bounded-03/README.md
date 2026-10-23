# bounded-03

Methods: LP / Bounded variables.

Open [input.txt](input.txt) in the matching module.

Maximize `z = 2*x1 + 3*x2`.

Subject to:

- `2*x1 + 1*x2 <= 8`
- `1*x1 + 2*x2 <= 8`
- `1*x1 + 0*x2 <= 3`

Bounds: 0 <= x1 <= 4, 0 <= x2 <= 3.5.

Expected answer (independent mathematics; original comparison pending):

```json
{
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
}
```



[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
