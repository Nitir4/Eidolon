# queue-finite-kn-01

Methods: Queuing / Finite K and N.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 0.5 |
| service | 3.0 |
| servers | 2 |
| K | 5 |
| N | 6 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.38519839500668746,
  "L": 0.9313419527418636,
  "Lq": 0.08693713776192599,
  "W": 0.3676522351284759,
  "Wq": 0.034318901795142555,
  "lambda_effective": 2.5332144449398126,
  "exact": {
    "P0": "864/2243",
    "L": "2089/2243",
    "Lq": "195/2243",
    "W": "2089/5682",
    "Wq": "65/1894"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
