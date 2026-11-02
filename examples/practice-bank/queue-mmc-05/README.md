# queue-mmc-05

Methods: Queuing / M/M/c.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 2.5 |
| service | 7.0 |
| servers | 2 |
| K | None |
| N | None |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.696969696969697,
  "L": 0.3689064558629776,
  "Lq": 0.01176359872012046,
  "W": 0.14756258234519104,
  "Wq": 0.004705439488048184,
  "lambda_effective": 2.5,
  "exact": {
    "P0": "23/33",
    "L": "280/759",
    "Lq": "125/10626",
    "W": "112/759",
    "Wq": "25/5313"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
