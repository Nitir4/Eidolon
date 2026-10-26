# cpm-04

Methods: Planning / CPM, Planning / Schedule bar chart.

Open [input.txt](input.txt) in the matching module.

| From | To | Activity | Duration |
|---|---|---|---|
| 1 | 2 | A | 5 |
| 1 | 3 | B | 2 |
| 2 | 4 | C | 3 |
| 3 | 4 | D | 4 |
| 4 | 5 | E | 2 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "project_duration": 10,
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
  ]
}
```

CPM TSV: from, to, symbol, duration. PERT TSV: from, to, symbol, optimistic, most likely, pessimistic; start at the first estimate cell for the last three fields.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
