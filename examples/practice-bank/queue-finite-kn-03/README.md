# queue-finite-kn-03

Methods: Queuing / Finite K and N.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 1.5 |
| service | 5.0 |
| servers | 2 |
| K | 7 |
| N | 8 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.08690625969276378,
  "L": 2.59598444412714,
  "Lq": 0.9783719867753007,
  "W": 0.320964942168778,
  "Wq": 0.12096494216877801,
  "lambda_effective": 8.088062286759197,
  "exact": {
    "P0": "1000000/11506651",
    "L": "29871087/11506651",
    "Lq": "11257785/11506651",
    "W": "9957029/31022170",
    "Wq": "750519/6204434"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
