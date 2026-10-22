# assignment-min-05

Methods: Assignment / min.

Open [input.assignment.json](input.assignment.json) in the matching module.

| Row | 1 | 2 | 3 |
|---|---|---|---|
| 1 | 1 | 4 | 7 |
| 2 | 5 | 2 | 8 |
| 3 | 6 | 9 | 3 |

Objective: min.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "objective": 6,
  "one_pairing_zero_based": [
    [
      0,
      0
    ],
    [
      1,
      1
    ],
    [
      2,
      2
    ]
  ]
}
```

Original equivalent: Transportation with unit supply/demand; negate profits for max. Rectangular cases need a zero-cost dummy row/column.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
