# queue-pure-death-03

Methods: Queuing / Pure death.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| rate | 3 |
| time | 1 |
| initial_population | 500 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "time": 1,
  "rate": 3,
  "P_no_events": 0.049787068367863944,
  "mean": 497,
  "initial_population": 500,
  "mean_note": "500-rate to displayed precision; exact mean has a positive Poisson tail correction below 1e-700."
}
```

Transient constant-rate unit-time profile. Pure death uses N=500 to match the captured original effective population; record probability of no deaths at state 500. Pure birth records state 0.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
