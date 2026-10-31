# pert-02

Methods: Planning / PERT.

Open [input.txt](input.txt) in the matching module.

| From | To | Activity | a | m | b |
|---|---|---|---|---|---|
| 1 | 2 | A | 1 | 2 | 3 |
| 1 | 3 | B | 4 | 5 | 6 |
| 2 | 4 | C | 5 | 6 | 7 |
| 3 | 4 | D | 1 | 2 | 3 |
| 4 | 5 | E | 2 | 3 | 4 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "project_duration": 11,
  "total_float": [
    0,
    1,
    0,
    1,
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
