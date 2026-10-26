# cpm-03

Methods: Planning / CPM, Planning / Schedule bar chart.

Open [input.txt](input.txt) in the matching module.

| From | To | Activity | Duration |
|---|---|---|---|
| 1 | 2 | A | 4 |
| 1 | 3 | B | 3 |
| 2 | 4 | C | 2 |
| 3 | 4 | D | 5 |
| 4 | 5 | E | 1 |

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
  ]
}
```

CPM TSV: from, to, symbol, duration. PERT TSV: from, to, symbol, optimistic, most likely, pessimistic; start at the first estimate cell for the last three fields.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
