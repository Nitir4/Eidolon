# mst-05

Methods: Networks / Minimum spanning tree.

Open [input.txt](input.txt) in the matching module.

| From (1-based) | To (1-based) | Weight |
|---|---|---|
| 1 | 2 | 3 |
| 1 | 3 | 4 |
| 2 | 3 | 5 |
| 2 | 4 | 1 |
| 3 | 4 | 2 |

Start/source: node A (1). Destination/sink: node D (4).

Expected answer (independent mathematics; original comparison pending):

```json
{
  "total_weight": 6
}
```

Route files use marker 0: select Shortest Route before opening. Source A, destination D. Positive undirected costs are valid for all listed methods.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
