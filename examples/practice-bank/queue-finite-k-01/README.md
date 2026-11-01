# queue-finite-k-01

Methods: Queuing / Finite capacity K.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 0.5 |
| service | 3.0 |
| servers | 2 |
| K | 5 |
| N | None |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.8461543693084054,
  "L": 0.16782850575025335,
  "Lq": 0.0011629725851316334,
  "W": 0.3356592943224789,
  "Wq": 0.0023259609891455152,
  "lambda_effective": 0.4999965994953651,
  "exact": {
    "P0": "124416/147037",
    "L": "24677/147037",
    "Lq": "171/147037",
    "W": "24677/73518",
    "Wq": "57/24506"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
