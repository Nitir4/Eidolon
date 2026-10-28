# game-05

Methods: Games / LP-based, Games / Graphical.

Open [input.txt](input.txt) in the matching module.

| Row | 1 | 2 |
|---|---|---|
| 1 | 5 | 0 |
| 2 | 1 | 4 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "value": 2.5,
  "row_probabilities": [
    0.375,
    0.625
  ],
  "column_probabilities": [
    0.5,
    0.5
  ],
  "exact": {
    "value": "5/2",
    "p": "3/8",
    "q": "1/2"
  }
}
```

Payoffs are to the maximizing row player. Graphical route requires two rows.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
