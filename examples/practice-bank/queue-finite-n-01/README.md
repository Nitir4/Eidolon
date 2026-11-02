# queue-finite-n-01

Methods: Queuing / Finite source N.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 0.5 |
| service | 3.0 |
| servers | 2 |
| K | 6 |
| N | 6 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.3851268526429182,
  "L": 0.9322833475725271,
  "Lq": 0.08766390550128153,
  "W": 0.3679303368809922,
  "Wq": 0.034597003547658836,
  "lambda_effective": 2.5338583262137364,
  "exact": {
    "P0": "10368/26921",
    "L": "25098/26921",
    "Lq": "2360/26921",
    "W": "4183/11369",
    "Wq": "1180/34107"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
