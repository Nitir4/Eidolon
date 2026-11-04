# transport-05

Methods: Transportation / North-west, Transportation / Least cost, Transportation / Vogel, Transportation / MODI improvement, Transportation / Final solution.

Open [input.txt](input.txt) in the matching module.

| Row | 1 | 2 | 3 |
|---|---|---|---|
| 1 | 3 | 8 | 5 |
| 2 | 4 | 1 | 7 |

Supply: `[3, 4]`. Demand: `[2, 2, 3]`.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "optimal_cost": 25,
  "one_optimal_flow": [
    [
      0,
      0,
      3
    ],
    [
      2,
      2,
      0
    ]
  ]
}
```

Final optimum key. Initial heuristic costs can differ. Compare each initial allocation separately, then improve to the stated optimum. Paste numeric costs starting at the first cost cell, or open input.txt.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
