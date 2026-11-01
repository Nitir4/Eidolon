# queue-finite-kn-04

Methods: Queuing / Finite K and N.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 2.0 |
| service | 6.0 |
| servers | 2 |
| K | 8 |
| N | 9 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.039149347510874816,
  "L": 3.570323827936201,
  "Lq": 1.766070565490575,
  "W": 0.32980623269934817,
  "Wq": 0.16313956603268148,
  "lambda_effective": 10.825519574673756,
  "exact": {
    "P0": "81/2069",
    "L": "7387/2069",
    "Lq": "3654/2069",
    "W": "7387/22398",
    "Wq": "609/3733"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
