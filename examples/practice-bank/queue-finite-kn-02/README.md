# queue-finite-kn-02

Methods: Queuing / Finite K and N.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 1.0 |
| service | 4.0 |
| servers | 2 |
| K | 6 |
| N | 7 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.18230777790141314,
  "L": 1.7276065427840213,
  "Lq": 0.4112607099143207,
  "W": 0.32810650887573967,
  "Wq": 0.07810650887573964,
  "lambda_effective": 5.2653833314788026,
  "exact": {
    "P0": "8192/44935",
    "L": "15526/8987",
    "Lq": "336/817",
    "W": "1109/3380",
    "Wq": "66/845"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
