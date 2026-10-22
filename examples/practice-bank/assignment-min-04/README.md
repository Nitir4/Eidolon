# assignment-min-04

Methods: Assignment / min, Assignment / Rectangular.

Open [input.assignment.json](input.assignment.json) in the matching module.

| Row | 1 | 2 |
|---|---|---|
| 1 | 3 | 8 |
| 2 | 7 | 2 |
| 3 | 4 | 6 |

Objective: min.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "objective": 5,
  "one_pairing_zero_based": [
    [
      0,
      0
    ],
    [
      1,
      1
    ]
  ]
}
```

Original equivalent: Transportation with unit supply/demand; negate profits for max. Rectangular cases need a zero-cost dummy row/column.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
