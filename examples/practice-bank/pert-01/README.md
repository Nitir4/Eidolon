# pert-01

Methods: Planning / PERT.

Open [input.txt](input.txt) in the matching module.

| From | To | Activity | a | m | b |
|---|---|---|---|---|---|
| 1 | 2 | A | 2 | 3 | 4 |
| 1 | 3 | B | 1 | 2 | 3 |
| 2 | 4 | C | 3 | 4 | 5 |
| 3 | 4 | D | 2 | 3 | 4 |
| 4 | 5 | E | 1 | 2 | 3 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "project_duration": 9,
  "total_float": [
    0,
    2,
    0,
    2,
    0
  ],
  "critical_activities": [
    "A",
    "C",
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
