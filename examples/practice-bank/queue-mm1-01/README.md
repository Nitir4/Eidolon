# queue-mm1-01

Methods: Queuing / M/M/1.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 0.5 |
| service | 3.0 |
| servers | 1 |
| K | None |
| N | None |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.8333333333333334,
  "L": 0.2,
  "Lq": 0.03333333333333333,
  "W": 0.4,
  "Wq": 0.06666666666666667,
  "lambda_effective": 0.5,
  "exact": {
    "P0": "5/6",
    "L": "1/5",
    "Lq": "1/30",
    "W": "2/5",
    "Wq": "1/15"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
