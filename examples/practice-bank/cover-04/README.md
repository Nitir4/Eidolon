# cover-04

Methods: LP / Big-M, LP / Two-phase, LP / Dual simplex.

Open [input.txt](input.txt) in the matching module.

Minimize `z = 5*x1 + 9*x2`.

Subject to:

- `1*x1 + 2*x2 >= 8`
- `3*x1 + 1*x2 >= 10`
- `1*x1 + 0*x2 <= 8`
- `0*x1 + 1*x2 <= 8`

Bounds: 0 <= x1 <= infinity, 0 <= x2 <= infinity.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "status": "optimal",
  "objective": 37.2,
  "objective_exact": "186/5",
  "one_solution": [
    2.4,
    2.8
  ],
  "one_solution_exact": [
    "12/5",
    "14/5"
  ],
  "check": "Objective and feasibility are authoritative; tied solutions may differ."
}
```

For Big-M use M=1000. Dual simplex applies directly to this nonnegative-cost minimization.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
