# queue-finite-k-04

Methods: Queuing / Finite capacity K.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 2.0 |
| service | 6.0 |
| servers | 2 |
| K | 8 |
| N | None |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.7142858357906698,
  "L": 0.3428556361956944,
  "Lq": 0.009522586373923968,
  "W": 0.1714279639032771,
  "Wq": 0.004761297236610446,
  "lambda_effective": 1.9999982989306226,
  "exact": {
    "P0": "839808/1175731",
    "L": "403106/1175731",
    "Lq": "11196/1175731",
    "W": "201553/1175730",
    "Wq": "933/195955"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
