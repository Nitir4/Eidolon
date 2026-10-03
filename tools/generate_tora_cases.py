"""Generate observation-only LP fixtures in the observed TORA text shape.

The format is intentionally kept in reference/: it is not treated as a
complete codec until each generated file is reopened successfully by TORA.
"""
from pathlib import Path
import json

ROOT = Path("reference/tora-captures/corpus/lp")

CASES = [
    ("01_normal", "Normal", "Maximize", [3, 2], [[1, 1, "<=", 4], [1, 0, "<=", 2], [0, 1, "<=", 3]]),
    ("02_degenerate", "Degenerate", "Maximize", [1, 1], [[1, 1, "<=", 2], [1, 0, "<=", 1], [0, 1, "<=", 1]]),
    ("03_infeasible", "Infeasible", "Maximize", [1, 1], [[1, 0, "<=", 1], [1, 0, ">=", 3], [0, 1, "<=", 5]]),
    ("04_unbounded", "Unbounded", "Maximize", [1, 1], [[1, -1, ">=", 1], [1, 0, ">=", 0], [0, 1, ">=", 0]]),
    ("05_alternative", "Alternative optima", "Maximize", [1, 1], [[1, 1, "<=", 4], [1, 0, "<=", 4], [0, 1, "<=", 4]]),
    ("06_equality", "Equality", "Maximize", [3, 2], [[1, 1, "=", 4], [1, 0, ">=", 1], [0, 1, ">=", 1]]),
    ("07_big_m", "Big M", "Maximize", [1, 1], [[1, 1, ">=", 4], [1, 0, "<=", 3], [0, 1, "<=", 3]]),
    ("08_two_phase", "Two phase", "Minimize", [1, 1], [[1, 1, "=", 5], [1, 0, ">=", 2], [0, 1, ">=", 2]]),
    ("09_bounded", "Bounded variables", "Maximize", [4, 3], [[1, 1, "<=", 10], [1, 0, "<=", 6], [0, 1, "<=", 7]]),
    ("10_dual", "Dual simplex seed", "Minimize", [2, 3], [[1, 1, ">=", 4], [1, 2, ">=", 6], [2, 1, ">=", 5]]),
    ("11_zero_rhs", "Zero RHS", "Maximize", [2, 1], [[1, 1, "<=", 0], [1, 0, "<=", 2], [0, 1, "<=", 2]]),
    ("12_negative_rhs", "Negative RHS", "Maximize", [2, 1], [[1, 1, ">=", -1], [1, 0, "<=", 3], [0, 1, "<=", 3]]),
    ("13_mixed_sign", "Mixed signs", "Maximize", [2, -1], [[1, -1, "<=", 2], [-1, 1, "<=", 2], [1, 1, "<=", 5]]),
    ("14_tight_bounds", "Tight bounds", "Maximize", [1, 2], [[1, 1, "<=", 3], [1, 0, "<=", 1], [0, 1, "<=", 2]]),
    ("15_minimization", "Minimization", "Minimize", [2, 3], [[1, 1, ">=", 4], [1, 0, ">=", 1], [0, 1, ">=", 1]]),
]

def q(value: object) -> str:
    return '"' + str(value) + '"'

def fmt(value: float) -> str:
    return f"{value:.2f}"

def make(title: str, sense: str, objective: list[float], constraints: list[list[object]]) -> str:
    # Header and record order are copied from the verified smoke file.
    lines = ["3000", "5", "2", "1", q(title), "2", "3", q(sense)]
    lines += [q(""), q(""), q(""), q("")]
    lines += [q(fmt(objective[0])), q(fmt(objective[1])), q(""), q("")]
    for a, b, rel, rhs in constraints:
        lines += [q(fmt(a)), q(fmt(b)), q("        " + rel), q(fmt(rhs))]
    lines += [q("0.00"), q("0.00"), q(""), q("")]
    lines += [q("infinity"), q("infinity"), q(""), q("")]
    lines += [q("n"), q("n"), q(""), q("")]
    return "\r\n".join(lines) + "\r\n"

ROOT.mkdir(parents=True, exist_ok=True)
manifest = []
for name, title, sense, objective, constraints in CASES:
    case_dir = ROOT / name
    case_dir.mkdir(exist_ok=True)
    (case_dir / "problem.txt").write_bytes(make(title, sense, objective, constraints).encode("ascii"))
    manifest.append({"case": name, "title": title, "sense": sense, "objective": objective, "constraints": constraints, "status": "generated_not_yet_verified"})
(ROOT / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
