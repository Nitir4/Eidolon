# transport-02

Methods: Transportation / North-west, Transportation / Least cost, Transportation / Vogel, Transportation / MODI improvement, Transportation / Final solution.

Open [input.txt](input.txt) in the matching module.

| Row | 1 | 2 | 3 |
|---|---|---|---|
| 1 | 4 | 1 | 7 |
| 2 | 2 | 6 | 3 |

Supply: `[3, 4]`. Demand: `[2, 2, 3]`.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "optimal_cost": 17,
  "one_optimal_flow": [
    [
      1,
      2,
      0
    ],
    [
      1,
      0,
      3
    ]
  ]
}
```

Final optimum key. Initial heuristic costs can differ. Compare each initial allocation separately, then improve to the stated optimum. Paste numeric costs starting at the first cost cell, or open input.txt.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
