# lp-03

Methods: LP / Graphical, LP / All-slack, LP / Final solution, LP / Sensitivity.

Open [input.txt](input.txt) in the matching module.

Maximize `z = 2*x1 + 3*x2`.

Subject to:

- `2*x1 + 1*x2 <= 8`
- `1*x1 + 2*x2 <= 8`
- `1*x1 + 0*x2 <= 3`

Bounds: 0 <= x1 <= infinity, 0 <= x2 <= infinity.

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

LP TSV contains grid columns x1, x2, relation, RHS; paste names/objective and constraint rows separately to avoid protected blank cells. Bounds remain as shown.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
