# game-04

Methods: Games / LP-based, Games / Graphical.

Open [input.txt](input.txt) in the matching module.

| Row | 1 | 2 |
|---|---|---|
| 1 | -1 | -4 |
| 2 | -5 | -2 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "value": -3.0,
  "row_probabilities": [
    0.5,
    0.5
  ],
  "column_probabilities": [
    0.3333333333333333,
    0.6666666666666666
  ],
  "exact": {
    "value": "-3",
    "p": "1/2",
    "q": "1/3"
  }
}
```

Payoffs are to the maximizing row player. Graphical route requires two rows.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
