# lp-01

Methods: LP / Graphical, LP / All-slack, LP / Final solution, LP / Sensitivity.

Open [input.txt](input.txt) in the matching module.

Maximize `z = 3*x1 + 2*x2`.

Subject to:

- `1*x1 + 1*x2 <= 4`
- `1*x1 + 0*x2 <= 2`
- `0*x1 + 1*x2 <= 3`

Bounds: 0 <= x1 <= infinity, 0 <= x2 <= infinity.

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

LP TSV contains grid columns x1, x2, relation, RHS; paste names/objective and constraint rows separately to avoid protected blank cells. Bounds remain as shown.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
