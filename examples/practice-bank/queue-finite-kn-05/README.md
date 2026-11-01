# queue-finite-kn-05

Methods: Queuing / Finite K and N.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 2.5 |
| service | 7.0 |
| servers | 2 |
| K | 9 |
| N | 10 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.01622242145620508,
  "L": 4.6313319308240315,
  "Lq": 2.7217139932228878,
  "W": 0.3464666068709533,
  "Wq": 0.20360946401381044,
  "lambda_effective": 13.367325563208002,
  "exact": {
    "P0": "2951578112/181944361387",
    "L": "842644730525/181944361387",
    "Lq": "495200514375/181944361387",
    "W": "33705789221/97284380522",
    "Wq": "2829717225/13897768646"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
