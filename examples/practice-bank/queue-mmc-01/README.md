# queue-mmc-01

Methods: Queuing / M/M/c.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 0.5 |
| service | 3.0 |
| servers | 2 |
| K | None |
| N | None |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.8461538461538461,
  "L": 0.16783216783216784,
  "Lq": 0.0011655011655011655,
  "W": 0.3356643356643357,
  "Wq": 0.002331002331002331,
  "lambda_effective": 0.5,
  "exact": {
    "P0": "11/13",
    "L": "24/143",
    "Lq": "1/858",
    "W": "48/143",
    "Wq": "1/429"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
