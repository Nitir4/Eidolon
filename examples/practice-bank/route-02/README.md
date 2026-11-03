# route-02

Methods: Networks / Shortest route, Networks / Dijkstra, Networks / Floyd, Networks / Bellman-Ford extension.

Open [input.txt](input.txt) in the matching module.

| From (1-based) | To (1-based) | Weight |
|---|---|---|
| 1 | 2 | 4 |
| 1 | 3 | 2 |
| 2 | 3 | 6 |
| 2 | 4 | 3 |
| 3 | 4 | 1 |

Start/source: node A (1). Destination/sink: node D (4).

Expected answer (independent mathematics; original comparison pending):

```json
{
  "distance": 3
}
```

Route files use marker 0: select Shortest Route before opening. Source A, destination D. Positive undirected costs are valid for all listed methods.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
