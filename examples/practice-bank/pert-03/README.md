# pert-03

Methods: Planning / PERT.

Open [input.txt](input.txt) in the matching module.

| From | To | Activity | a | m | b |
|---|---|---|---|---|---|
| 1 | 2 | A | 3 | 4 | 5 |
| 1 | 3 | B | 2 | 3 | 4 |
| 2 | 4 | C | 1 | 2 | 3 |
| 3 | 4 | D | 4 | 5 | 6 |
| 4 | 5 | E | 0 | 1 | 2 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "project_duration": 9,
  "total_float": [
    2,
    0,
    2,
    0,
    0
  ],
  "critical_activities": [
    "B",
    "D",
    "E"
  ],
  "activity_variances_exact": [
    "1/9",
    "1/9",
    "1/9",
    "1/9",
    "1/9"
  ],
  "project_variance_exact": "1/3",
  "note": "Do not sum variance over both branches; compare selected critical path and tie behaviour separately."
}
```

CPM TSV: from, to, symbol, duration. PERT TSV: from, to, symbol, optimistic, most likely, pessimistic; start at the first estimate cell for the last three fields.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
