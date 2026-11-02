# queue-finite-n-05

Methods: Queuing / Finite source N.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 2.5 |
| service | 7.0 |
| servers | 2 |
| K | 10 |
| N | 10 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.016159693439536573,
  "L": 4.65209121765677,
  "Lq": 2.7421237953913313,
  "W": 0.3479559137595028,
  "Wq": 0.20509877090235992,
  "lambda_effective": 13.369771955858074,
  "exact": {
    "P0": "82644187136/5114217509461",
    "L": "23791806360950/5114217509461",
    "Lq": "14023817527500/5114217509461",
    "W": "475836127219/1367518436683",
    "Wq": "280476350550/1367518436683"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
