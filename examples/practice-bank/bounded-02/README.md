# bounded-02

Methods: LP / Bounded variables.

Open [input.txt](input.txt) in the matching module.

Maximize `z = 5*x1 + 4*x2`.

Subject to:

- `6*x1 + 4*x2 <= 24`
- `1*x1 + 2*x2 <= 6`
- `-1*x1 + 1*x2 <= 1`

Bounds: 0 <= x1 <= 3, 0 <= x2 <= 3.0.

Expected answer (independent mathematics; original comparison pending):

```json
{
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
}
```



[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
