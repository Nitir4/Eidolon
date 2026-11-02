# queue-finite-n-04

Methods: Queuing / Finite source N.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 2.0 |
| service | 6.0 |
| servers | 2 |
| K | 9 |
| N | 9 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.03903928026347498,
  "L": 3.5855892039521247,
  "Lq": 1.7807856052694995,
  "W": 0.33111536407334874,
  "Wq": 0.1644486974066821,
  "lambda_effective": 10.82882159209575,
  "exact": {
    "P0": "486/12449",
    "L": "44637/12449",
    "Lq": "22169/12449",
    "W": "14879/44936",
    "Wq": "22169/134808"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
