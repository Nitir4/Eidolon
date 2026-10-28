# game-02

Methods: Games / LP-based, Games / Graphical.

Open [input.txt](input.txt) in the matching module.

| Row | 1 | 2 |
|---|---|---|
| 1 | 4 | 1 |
| 2 | 0 | 3 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "value": 2.0,
  "row_probabilities": [
    0.5,
    0.5
  ],
  "column_probabilities": [
    0.3333333333333333,
    0.6666666666666666
  ],
  "exact": {
    "value": "2",
    "p": "1/2",
    "q": "1/3"
  }
}
```

Payoffs are to the maximizing row player. Graphical route requires two rows.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
