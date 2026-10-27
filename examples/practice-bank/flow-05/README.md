# flow-05

Methods: Networks / Maximal flow.

Open [input.txt](input.txt) in the matching module.

| From (1-based) | To (1-based) | Capacity |
|---|---|---|
| 1 | 2 | 8 |
| 1 | 3 | 9 |
| 2 | 3 | 10 |
| 2 | 4 | 6 |
| 3 | 4 | 7 |

Start/source: node A (1). Destination/sink: node D (4).

Expected answer (independent mathematics; original comparison pending):

```json
{
  "max_flow": 13,
  "min_cut": 13
}
```

Source A, sink D. Expected value independently enumerates all four source/sink cuts.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
