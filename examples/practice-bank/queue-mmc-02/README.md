# queue-mmc-02

Methods: Queuing / M/M/c.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 1.0 |
| service | 4.0 |
| servers | 2 |
| K | None |
| N | None |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.7777777777777778,
  "L": 0.25396825396825395,
  "Lq": 0.003968253968253968,
  "W": 0.25396825396825395,
  "Wq": 0.003968253968253968,
  "lambda_effective": 1.0,
  "exact": {
    "P0": "7/9",
    "L": "16/63",
    "Lq": "1/252",
    "W": "16/63",
    "Wq": "1/252"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
