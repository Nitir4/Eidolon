# queue-pure-birth-04

Methods: Queuing / Pure birth.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| rate | 4 |
| time | 1 |
| initial_population | 0 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "time": 1,
  "rate": 4,
  "P_no_events": 0.01831563888873418,
  "mean": 4
}
```

Transient constant-rate unit-time profile. Pure death uses N=500 to match the captured original effective population; record probability of no deaths at state 500. Pure birth records state 0.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
