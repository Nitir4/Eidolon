# queue-mmc-03

Methods: Queuing / M/M/c.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 1.5 |
| service | 5.0 |
| servers | 2 |
| K | None |
| N | None |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.7391304347826086,
  "L": 0.3069053708439898,
  "Lq": 0.00690537084398977,
  "W": 0.20460358056265984,
  "Wq": 0.004603580562659846,
  "lambda_effective": 1.5,
  "exact": {
    "P0": "17/23",
    "L": "120/391",
    "Lq": "27/3910",
    "W": "80/391",
    "Wq": "9/1955"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
