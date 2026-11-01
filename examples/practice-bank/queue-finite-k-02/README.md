# queue-finite-k-02

Methods: Queuing / Finite capacity K.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 1.0 |
| service | 4.0 |
| servers | 2 |
| K | 6 |
| N | None |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.7777784371087283,
  "L": 0.25396241417983517,
  "Lq": 0.003963897674473804,
  "W": 0.2539639211962972,
  "Wq": 0.003963921196297175,
  "lambda_effective": 0.9999940660214454,
  "exact": {
    "P0": "131072/168521",
    "L": "42798/168521",
    "Lq": "668/168521",
    "W": "21399/84260",
    "Wq": "167/42130"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
