# queue-mm1-03

Methods: Queuing / M/M/1.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 1.5 |
| service | 5.0 |
| servers | 1 |
| K | None |
| N | None |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.7,
  "L": 0.42857142857142855,
  "Lq": 0.12857142857142856,
  "W": 0.2857142857142857,
  "Wq": 0.08571428571428572,
  "lambda_effective": 1.5,
  "exact": {
    "P0": "7/10",
    "L": "3/7",
    "Lq": "9/70",
    "W": "2/7",
    "Wq": "3/35"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
