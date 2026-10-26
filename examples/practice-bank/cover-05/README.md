# cover-05

Methods: LP / Big-M, LP / Two-phase, LP / Dual simplex.

Open [input.txt](input.txt) in the matching module.

Minimize `z = 6*x1 + 11*x2`.

Subject to:

- `1*x1 + 2*x2 >= 9`
- `3*x1 + 1*x2 >= 11`
- `1*x1 + 0*x2 <= 8`
- `0*x1 + 1*x2 <= 8`

Bounds: 0 <= x1 <= infinity, 0 <= x2 <= infinity.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "status": "optimal",
  "objective": 50.8,
  "objective_exact": "254/5",
  "one_solution": [
    2.6,
    3.2
  ],
  "one_solution_exact": [
    "13/5",
    "16/5"
  ],
  "check": "Objective and feasibility are authoritative; tied solutions may differ."
}
```

For Big-M use M=1000. Dual simplex applies directly to this nonnegative-cost minimization.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
