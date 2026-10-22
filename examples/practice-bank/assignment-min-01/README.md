# assignment-min-01

Methods: Assignment / min.

Open [input.assignment.json](input.assignment.json) in the matching module.

| Row | 1 | 2 | 3 |
|---|---|---|---|
| 1 | 9 | 2 | 7 |
| 2 | 6 | 4 | 3 |
| 3 | 5 | 8 | 1 |

Objective: min.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "objective": 9,
  "one_pairing_zero_based": [
    [
      0,
      1
    ],
    [
      1,
      0
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
