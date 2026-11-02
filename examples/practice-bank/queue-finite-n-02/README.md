# queue-finite-n-02

Methods: Queuing / Finite source N.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 1.0 |
| service | 4.0 |
| servers | 2 |
| K | 7 |
| N | 7 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.18214816770661071,
  "L": 1.7322225155991606,
  "Lq": 0.4152781444989508,
  "W": 0.328833653420003,
  "Wq": 0.07883365342000295,
  "lambda_effective": 5.267777484400839,
  "exact": {
    "P0": "65536/359795",
    "L": "124649/71959",
    "Lq": "29883/71959",
    "W": "17807/54152",
    "Wq": "4269/54152"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
