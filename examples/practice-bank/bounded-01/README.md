# bounded-01

Methods: LP / Bounded variables.

Open [input.txt](input.txt) in the matching module.

Maximize `z = 3*x1 + 2*x2`.

Subject to:

- `1*x1 + 1*x2 <= 4`
- `1*x1 + 0*x2 <= 2`
- `0*x1 + 1*x2 <= 3`

Bounds: 0 <= x1 <= 2, 0 <= x2 <= 2.5.

Expected answer (independent mathematics; original comparison pending):

```json
{
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
}
```



[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
