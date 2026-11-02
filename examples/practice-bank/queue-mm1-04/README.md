# queue-mm1-04

Methods: Queuing / M/M/1.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 2.0 |
| service | 6.0 |
| servers | 1 |
| K | None |
| N | None |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.6666666666666666,
  "L": 0.5,
  "Lq": 0.16666666666666666,
  "W": 0.25,
  "Wq": 0.08333333333333333,
  "lambda_effective": 2.0,
  "exact": {
    "P0": "2/3",
    "L": "1/2",
    "Lq": "1/6",
    "W": "1/4",
    "Wq": "1/12"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
