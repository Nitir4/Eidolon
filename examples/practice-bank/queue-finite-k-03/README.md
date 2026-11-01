# queue-finite-k-03

Methods: Queuing / Finite capacity K.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 1.5 |
| service | 5.0 |
| servers | 2 |
| K | 7 |
| N | None |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.7391307642280532,
  "L": 0.3069018632189623,
  "Lq": 0.006902620943484783,
  "W": 0.2046017589185418,
  "Wq": 0.004601758918541786,
  "lambda_effective": 1.4999962113773875,
  "exact": {
    "P0": "640000000/865881967",
    "L": "265740789/865881967",
    "Lq": "5976855/865881967",
    "W": "88580263/432939890",
    "Wq": "398457/86587978"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
