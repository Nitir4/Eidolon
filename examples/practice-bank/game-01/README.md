# game-01

Methods: Games / LP-based, Games / Graphical.

Open [input.txt](input.txt) in the matching module.

| Row | 1 | 2 |
|---|---|---|
| 1 | 1 | -1 |
| 2 | -1 | 1 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "value": 0.0,
  "row_probabilities": [
    0.5,
    0.5
  ],
  "column_probabilities": [
    0.5,
    0.5
  ],
  "exact": {
    "value": "0",
    "p": "1/2",
    "q": "1/2"
  }
}
```

Payoffs are to the maximizing row player. Graphical route requires two rows.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
