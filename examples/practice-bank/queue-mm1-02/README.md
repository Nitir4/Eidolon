# queue-mm1-02

Methods: Queuing / M/M/1.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 1.0 |
| service | 4.0 |
| servers | 1 |
| K | None |
| N | None |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.75,
  "L": 0.3333333333333333,
  "Lq": 0.08333333333333333,
  "W": 0.3333333333333333,
  "Wq": 0.08333333333333333,
  "lambda_effective": 1.0,
  "exact": {
    "P0": "3/4",
    "L": "1/3",
    "Lq": "1/12",
    "W": "1/3",
    "Wq": "1/12"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
