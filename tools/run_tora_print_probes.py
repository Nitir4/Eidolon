"""Launch TORA and capture actual printer jobs in a private, isolated queue.

Requires an already configured user-owned CUPS server and Wine prefix. Never
installs packages, changes the host printer, or commits reference material.
Completed capture markers are reused. No golden output is calculated here.
"""
import argparse
import hashlib
import json
import time
import socket
from pathlib import Path

import run_tora_corpus as oracle
from run_tora_file_probes import IsolatedFileDriver, windows


def entries():
    base = Path("reference/tora-captures")
    normal = base / "roundtrips/01_normal/tora-saved.txt"
    cases = [("summary-normal", "final", normal, False, 2)]
    for name in ["final-scientific", "final-decimal-three", "final-free",
                 "final-alias", "sens-negative-lower", "sens-min-reduced",
                 "sens-negative-lower-positive-cost", "sens-negative-lower-upper",
                 "sens-negative-rhs-active", "sens-both-bounds", "final-upper", "final-fixed"]:
        cases.append((name, "final", base / "probes/phase2-final" / name /
                      "tora-saved.txt", name == "final-scientific",
                      3 if name == "final-decimal-three" else 2))
    for name in ["03_infeasible", "04_unbounded", "05_alternative"]:
        cases.append(("summary-" + name, "final", base / "roundtrips" / name /
                      "tora-saved.txt", False, 2))
    for method, name in [("all_slack", "01_normal"), ("m_method", "06_equality"),
                         ("two_phase", "08_two_phase"), ("bounded", "09_bounded"),
                         ("dual_simplex", "10_dual")]:
        cases.append(("trace-" + method, method, base / "roundtrips" / name /
                      "tora-saved.txt", False, 2))
    cases.extend([("graph-normal", "graphical", normal, False, 2),
                  ("graph-scientific", "graphical", normal, True, 2)])
    cases.extend([
        ("trace-m-same-input", "m_method", base / "roundtrips/08_two_phase/tora-saved.txt", False, 2),
        ("trace-two-phase-same-input", "two_phase", base / "roundtrips/06_equality/tora-saved.txt", False, 2),
    ])
    return cases


def run(driver, root, spool, selected):
    for name, method, source, scientific, decimals in entries():
        if selected and name not in selected:
            continue
        dest = root / name
        dest.mkdir(parents=True, exist_ok=True)
        if (dest / "captured.json").exists():
            print("Already captured:", name, flush=True)
            continue
        if not source.is_file():
            raise RuntimeError("Missing existing oracle input: " + str(source))
        driver.recover_main_menu()
        driver.load_case_from_main(windows(source))
        (dest / "tora-saved.txt").write_bytes(source.read_bytes())
        driver.capture(dest / "input.png")
        if method in ["final", "graphical"]:
            driver.click(844, 812)
            driver.click(957, 585)
            driver.click(844, 812)
            driver.hover(946, 480)
            if method == "final":
                driver.hover(1075, 496)
                driver.click(1130, 498)
            else:
                driver.click(1075, 479)
        else:
            driver.open_method("bounded_simplex" if method == "bounded" else method)
        driver.capture(dest / "output-format.png")
        if not driver._is_output_format_screen(dest / "output-format.png"):
            raise RuntimeError("Unexpected setup screen: " + name)
        if scientific:
            driver.click(858, 502)
        if decimals != 2:
            driver.click(952, 541)
            driver.key("ctrl+a")
            driver.run("xdotool", "type", "--clearmodifiers", str(decimals))
        driver.capture(dest / "format-selected.png")
        driver.click(960, 595)
        driver.pause(1)
        driver.capture(dest / "initial-output.png")
        if driver.ui("dialog") == 0:
            driver.key("Return")
        if method == "graphical":
            driver.click(704, 570)
            driver.pause(2)
        elif method != "final":
            driver.click(955, 432)  # All Iterations
            driver.pause(1)
            if driver.ui("dialog") == 0:
                driver.key("Return")
        driver.capture(dest / "before-print.png")
        if name in ["summary-03_infeasible", "summary-04_unbounded"]:
            (dest / "captured.json").write_text(json.dumps({
                "method": method, "input": str(source), "format": [int(scientific),5,decimals],
                "print_available": False, "review": "pending direct control inspection",
                "sha256": {p.name: hashlib.sha256(p.read_bytes()).hexdigest()
                           for p in dest.iterdir() if p.suffix in [".png", ".txt"]}},indent=2)+"\n")
            print("Captured unavailable printer control:",name,flush=True)
            continue
        existing = set(spool.glob("d*-*"))
        driver.click(*( (1180, 358) if method == "graphical" else (1075, 427) ))
        deadline = time.monotonic() + 15
        job = None
        while time.monotonic() < deadline:
            new = sorted(set(spool.glob("d*-*")) - existing)
            if len(new) == 1 and new[0].stat().st_size > 100:
                job = new[0]
                # EndDoc/IPP completion must have finished before preserving it.
                size = job.stat().st_size
                driver.pause(.3)
                if job.stat().st_size == size:
                    break
            driver.pause(.2)
        driver.capture(dest / "after-print.png")
        if job is None or not job.read_bytes().startswith(b"%!PS-Adobe"):
            raise RuntimeError("No complete actual PostScript job: " + name)
        document = job.read_bytes()
        if b"%%EOF" not in document:
            raise RuntimeError("Incomplete printer document: " + name)
        (dest / "original.ps").write_bytes(document)
        record = {"method": method, "input": str(source),
                  "format": [int(scientific), 5, decimals],
                  "source": "actual TORA Write to Printer/Print Graph submission",
                  "job": job.name, "review": "pending direct page inspection",
                  "sha256": {p.name: hashlib.sha256(p.read_bytes()).hexdigest()
                             for p in dest.iterdir() if p.suffix in [".png", ".txt", ".ps"]}}
        (dest / "captured.json").write_text(json.dumps(record, indent=2) + "\n")
        print("Captured actual printer output:", name, len(document), flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--wine-prefix", type=Path, required=True)
    parser.add_argument("--cups-server", required=True)
    parser.add_argument("--spool", type=Path, required=True)
    parser.add_argument("--unix-library-dir", type=Path)
    parser.add_argument("--output", type=Path,
                        default=Path("reference/tora-captures/probes/printing"))
    parser.add_argument("--case", action="append", default=[])
    args = parser.parse_args()
    known = {e[0] for e in entries()}
    if set(args.case) - known:
        parser.error("Unknown cases: " + ", ".join(sorted(set(args.case) - known)))
    if not args.spool.is_dir():
        parser.error("The isolated queue spool directory must exist")
    host, port = args.cups_server.rsplit(":", 1)
    try:
        with socket.create_connection((host, int(port)), timeout=2):
            pass
    except OSError as error:
        parser.error("Isolated printer server is unavailable: " + str(error))
    root = args.output.resolve()
    oracle.OUTPUT = root / "runtime"
    oracle.OUTPUT.mkdir(parents=True, exist_ok=True)
    driver = IsolatedFileDriver(.10, .7)
    driver.env.update(WINEPREFIX=str(args.wine_prefix.resolve()),
                      CUPS_SERVER=args.cups_server)
    if args.unix_library_dir:
        driver.env["LD_LIBRARY_PATH"] = str(args.unix_library_dir.resolve())
    driver.launch_tora()
    driver.capture(root / "launch.png")
    run(driver, root, args.spool.resolve(), set(args.case))


if __name__ == "__main__":
    main()
