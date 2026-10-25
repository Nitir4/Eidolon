# cover-01

Methods: LP / Big-M, LP / Two-phase, LP / Dual simplex.

Open [input.txt](input.txt) in the matching module.

Minimize `z = 2*x1 + 3*x2`.

Subject to:

- `1*x1 + 2*x2 >= 5`
- `3*x1 + 1*x2 >= 7`
- `1*x1 + 0*x2 <= 8`
- `0*x1 + 1*x2 <= 8`

Bounds: 0 <= x1 <= infinity, 0 <= x2 <= infinity.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "status": "optimal",
  "objective": 8.4,
  "objective_exact": "42/5",
  "one_solution": [
    1.8,
    1.6
  ],
  "one_solution_exact": [
    "9/5",
    "8/5"
  ],
  "check": "Objective and feasibility are authoritative; tied solutions may differ."
}
```

For Big-M use M=1000. Dual simplex applies directly to this nonnegative-cost minimization.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
