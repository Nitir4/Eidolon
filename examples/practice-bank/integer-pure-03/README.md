# integer-pure-03

Methods: Integer / Pure automated, Integer / Guided branch-and-bound.

Open [input.txt](input.txt) in the matching module.

Maximize `z = 2*x1 + 3*x2`.

Subject to:

- `2*x1 + 1*x2 <= 8`
- `1*x1 + 2*x2 <= 8`
- `1*x1 + 0*x2 <= 3`

Bounds: 0 <= x1 <= infinity, 0 <= x2 <= infinity.

Integer variables: x1, x2. Other variables are continuous.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "status": "optimal",
  "objective": 13.0,
  "objective_exact": "13",
  "one_solution": [
    2.0,
    3.0
  ],
  "one_solution_exact": [
    "2",
    "3"
  ],
  "check": "Objective and feasibility are authoritative; tied solutions may differ."
}
```

Integer enumeration bounds 0..30 cover these small bounded feasible regions.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
