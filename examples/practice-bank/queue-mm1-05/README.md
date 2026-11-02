# queue-mm1-05

Methods: Queuing / M/M/1.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 2.5 |
| service | 7.0 |
| servers | 1 |
| K | None |
| N | None |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.6428571428571429,
  "L": 0.5555555555555556,
  "Lq": 0.1984126984126984,
  "W": 0.2222222222222222,
  "Wq": 0.07936507936507936,
  "lambda_effective": 2.5,
  "exact": {
    "P0": "9/14",
    "L": "5/9",
    "Lq": "25/126",
    "W": "2/9",
    "Wq": "5/63"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
