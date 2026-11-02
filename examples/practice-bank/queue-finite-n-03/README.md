# queue-finite-n-03

Methods: Queuing / Finite source N.

Open [input.txt](input.txt) in the matching module.

| Parameter | Value |
|---|---|
| arrival | 1.5 |
| service | 5.0 |
| servers | 2 |
| K | 8 |
| N | 8 |

Expected answer (independent mathematics; original comparison pending):

```json
{
  "P0": 0.0867504468070919,
  "L": 2.605673219837348,
  "Lq": 0.9873751857885522,
  "W": 0.3220263715353164,
  "Wq": 0.1220263715353164,
  "lambda_effective": 8.091490170243977,
  "exact": {
    "P0": "20000000/230546363",
    "L": "600728484/230546363",
    "Lq": "227635758/230546363",
    "W": "100121414/310910605",
    "Wq": "37939293/310910605"
  }
}
```

Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.

[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)
