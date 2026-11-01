# queue-finite-k-05

Methods: Queuing / Finite capacity K.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 2.5 |
| service | 7.0 |
| servers | 2 |
| K | 9 |
| N | None |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.6969697359645903,
  "L": 0.3689059048481795,
  "Lq": 0.011763139621856803,
  "W": 0.14756239991686015,
  "Wq": 0.004705257059717305,
  "lambda_effective": 2.499999356584259,
  "exact": {
    "P0": "5289227976704/7588891889809",
    "L": "2799587029405/7588891889809",
    "Lq": "89269194875/7588891889809",
    "W": "559917405881/3794444968342",
    "Wq": "2550548425/542063566906"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
