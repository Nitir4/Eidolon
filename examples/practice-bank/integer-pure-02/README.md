# integer-pure-02

Methods: Integer / Pure automated, Integer / Guided branch-and-bound.

Open [input.txt](input.txt) in the matching module.

Maximize `z = 5*x1 + 4*x2`.

Subject to:

- `6*x1 + 4*x2 <= 24`
- `1*x1 + 2*x2 <= 6`
- `-1*x1 + 1*x2 <= 1`

Bounds: 0 <= x1 <= infinity, 0 <= x2 <= infinity.

Integer variables: x1, x2. Other variables are continuous.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "status": "optimal",
  "objective": 20.0,
  "objective_exact": "20",
  "one_solution": [
    4.0,
    0.0
  ],
  "one_solution_exact": [
    "4",
    "0"
  ],
  "check": "Objective and feasibility are authoritative; tied solutions may differ."
}
```

Integer enumeration bounds 0..30 cover these small bounded feasible regions.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
