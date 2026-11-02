# queue-mmc-04

Methods: Queuing / M/M/c.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 2.0 |
| service | 6.0 |
| servers | 2 |
| K | None |
| N | None |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.7142857142857143,
  "L": 0.34285714285714286,
  "Lq": 0.009523809523809525,
  "W": 0.17142857142857143,
  "Wq": 0.004761904761904762,
  "lambda_effective": 2.0,
  "exact": {
    "P0": "5/7",
    "L": "12/35",
    "Lq": "1/105",
    "W": "6/35",
    "Wq": "1/210"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
