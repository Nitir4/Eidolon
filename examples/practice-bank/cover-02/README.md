# cover-02

Methods: LP / Big-M, LP / Two-phase, LP / Dual simplex.

Open [input.txt](input.txt) in the matching module.

Minimize `z = 3*x1 + 5*x2`.

Subject to:

- `1*x1 + 2*x2 >= 6`
- `3*x1 + 1*x2 >= 8`
- `1*x1 + 0*x2 <= 8`
- `0*x1 + 1*x2 <= 8`

Bounds: 0 <= x1 <= infinity, 0 <= x2 <= infinity.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "status": "optimal",
  "objective": 16.0,
  "objective_exact": "16",
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

For Big-M use M=1000. Dual simplex applies directly to this nonnegative-cost minimization.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
