# assignment-max-03

Methods: Assignment / max, Assignment / Rectangular.

Open [input.assignment.json](input.assignment.json) in the matching module.

| Row | 1 | 2 | 3 |
|---|---|---|---|
| 1 | 5 | 1 | 4 |
| 2 | 2 | 6 | 3 |

Objective: max.

Expected answer (independent mathematics; original comparison pending):

```json
{
  "objective": 11,
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
