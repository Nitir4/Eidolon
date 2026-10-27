# flow-02

Methods: Networks / Maximal flow.

Open [input.txt](input.txt) in the matching module.

| From (1-based) | To (1-based) | Capacity |
|---|---|---|
| 1 | 2 | 6 |
| 1 | 3 | 4 |
| 2 | 3 | 8 |
| 2 | 4 | 5 |
| 3 | 4 | 3 |

Start/source: node A (1). Destination/sink: node D (4).

Expected answer (independent mathematics; original comparison pending):

```json
{
  "max_flow": 8,
  "min_cut": 8
}
```

Source A, sink D. Expected value independently enumerates all four source/sink cuts.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
