# assignment-max-02

Methods: Assignment / max.

Open [input.assignment.json](input.assignment.json) in the matching module.

| Row | 1 | 2 | 3 |
|---|---|---|---|
| 1 | 4 | 8 | 6 |
| 2 | 7 | 3 | 5 |
| 3 | 9 | 2 | 1 |

Objective: max.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "objective": 22,
  "one_pairing_zero_based": [
    [
      0,
      1
    ],
    [
      1,
      2
    ],
    [
      2,
      0
    ]
  ]
}
```

Original equivalent: Transportation with unit supply/demand; negate profits for max. Rectangular cases need a zero-cost dummy row/column.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
