# assignment-max-05

Methods: Assignment / max.

Open [input.assignment.json](input.assignment.json) in the matching module.

| Row | 1 | 2 | 3 |
|---|---|---|---|
| 1 | 1 | 4 | 7 |
| 2 | 5 | 2 | 8 |
| 3 | 6 | 9 | 3 |

Objective: max.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "objective": 21,
  "one_pairing_zero_based": [
    [
      0,
      2
    ],
    [
      1,
      0
    ],
    [
      2,
      1
    ]
  ]
}
```

Original equivalent: Transportation with unit supply/demand; negate profits for max. Rectangular cases need a zero-cost dummy row/column.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
