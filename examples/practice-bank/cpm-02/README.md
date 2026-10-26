# cpm-02

Methods: Planning / CPM, Planning / Schedule bar chart.

Open [input.txt](input.txt) in the matching module.

| From | To | Activity | Duration |
|---|---|---|---|
| 1 | 2 | A | 2 |
| 1 | 3 | B | 5 |
| 2 | 4 | C | 6 |
| 3 | 4 | D | 2 |
| 4 | 5 | E | 3 |

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
  ]
}
```

CPM TSV: from, to, symbol, duration. PERT TSV: from, to, symbol, optimistic, most likely, pessimistic; start at the first estimate cell for the last three fields.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
