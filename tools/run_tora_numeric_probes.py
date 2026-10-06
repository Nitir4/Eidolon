#!/usr/bin/env python3
"""Focused, resumable Foundation 2 observations; never rerun the LP corpus.

Launch/reuse TORA first, reopen fixtures through its UI, Save As through its
UI, and capture initial tableaus with selected number formats. No binary
inspection and no inferred output numbers. Screenshot review is separate.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

import run_tora_corpus as oracle

ROOT = oracle.ROOT / "reference/tora-captures/probes/foundation2-20261007"

def launch_visible(driver: oracle.ToraDriver) -> None:
    try:
        driver.launch_tora()
    except RuntimeError:
        # Wine may finish creating its desktop after the first positioning
        # request. Reposition the existing window; never kill/restart it.
        if not driver._attach_to_running_window():
            raise
        driver._arrange_wine_desktop()
        driver._attach_to_running_window()
        driver.enter_main_menu()
# Every value is a source lexeme, not a predicted display value. Coefficients
# and RHSs are visible in an initial tableau without depending on pivot rules.
CASES = [
    ("ties-exact", False, "5", "2", ["2.125", "2.375"], ["1.125", "1.375", "2.625"]),
    ("ties-decimal", False, "5", "2", ["2.675", "1.005"], ["0.005", "0.015", "0.025"]),
    ("negative-near-zero", False, "5", "2", ["0.0001", "-0.0001"], ["1", "2", "3"]),
    ("signed-zero", False, "5", "2", ["-0.0", "0.0"], ["0.0", "1", "2"]),
    ("decimal-width", False, "1", "2", ["12345.125", "123456789.375"], ["0.125", "99999.995", "1e20"]),
    ("decimal-integer", False, "5", "0", ["2.5", "3.5"], ["0.5", "1.5", "4.5"]),
    ("decimal-three", False, "5", "3", ["1.0625", "1.1875"], ["0.0005", "0.0015", "1.2345"]),
    ("scientific-n1-d2", True, "1", "2", ["1.25", "1.35"], ["9.95", "0.0125", "12345"]),
    ("scientific-n2-d1", True, "2", "1", ["1.125", "1.375"], ["9.995", "1e-12", "1e25"]),
    ("scientific-n5-d3", True, "5", "3", ["2.675", "1.005"], ["0.0001", "1e-12", "1e25"]),
    ("decimal-n0", False, "0", "2", ["12.125", "1.375"], ["1", "2", "3"]),
    ("scientific-n0", True, "0", "2", ["2.5", "3.5"], ["9.5", "0.1", "100"]),
    ("scientific-precision", True, "10", "2", ["1.23456789012345", "16777217"], ["1.23456789012345", "16777217", "3"]),
    ("scientific-small-large", True, "5", "2", ["1e-100", "1e100"], ["1e-100", "1e100", "3"]),
    ("decimal-high-precision", False, "5", "10", ["1.23456789012345", "1.00000000005"], ["0.00000000005", "1.00000000015", "3"]),
]

def fixture(name: str, objectives: list[str], rhs: list[str]) -> bytes:
    q = lambda value: '"' + value + '"'
    lines = ["3000", "5", "2", "1", q("Foundation 2 " + name), "2", "3", q("Maximize")]
    lines += list(map(q, ["", "", "", ""]))
    lines += list(map(q, objectives + ["", ""]))
    for coefficients, bound in zip((["1", "0"], ["0", "1"], ["1", "1"]), rhs):
        lines += list(map(q, coefficients + ["        <=", bound]))
    for values in (["0", "0", "", ""], ["infinity", "infinity", "", ""], ["n", "n", "", ""]):
        lines += list(map(q, values))
    return ("\r\n".join(lines) + "\r\n").encode("ascii")

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--execute", action="store_true")
    parser.add_argument("--startup-only", action="store_true")
    parser.add_argument("--dismiss-file-error", action="store_true", help="Recover the explicitly observed stale-directory file dialog")
    parser.add_argument("--acknowledge-format-error", action="store_true", help="Acknowledge the already captured N=0 runtime error")
    parser.add_argument("--summary", action="store_true", help="Capture final summaries to read values clipped by tableau cell widths")
    parser.add_argument("--capture-current", action="store_true", help="Inspect the existing reference window without navigation")
    parser.add_argument("--finish-save-case", help="Resume an interrupted native Save As dialog for a named probe")
    parser.add_argument("--cases", nargs="*")
    args = parser.parse_args()
    cases = [c for c in CASES if not args.cases or c[0] in args.cases]
    if args.cases and set(args.cases) - {c[0] for c in CASES}:
        parser.error("Unknown probe case")
    if not args.execute:
        print("Focused cases:", ", ".join(c[0] for c in cases))
        return 0
    # Isolate operational screenshots/logs from completed Phase 0 evidence.
    oracle.OUTPUT = ROOT / "runtime"
    oracle.OUTPUT.mkdir(parents=True, exist_ok=True)
    driver = oracle.ToraDriver(0.12, 0.8)
    if args.capture_current or args.finish_save_case:
        if not driver._attach_to_running_window():
            raise RuntimeError("Expected an existing reference window")
        driver.capture(oracle.OUTPUT / "current-window.png")
        if args.capture_current:
            return 0
        saved = ROOT / args.finish_save_case / "tora-saved.txt"
        driver.click(750, 472)
        driver.key("ctrl+a")
        driver.run("xdotool", "type", "--clearmodifiers", "--delay", "10", "Z:" + str(saved).replace("/", "\\"))
        driver.key("Return")
        driver.pause(2)
        driver.capture(oracle.OUTPUT / "save-resumed.png")
        if not saved.exists():
            raise RuntimeError("Native Save As still pending; inspect save-resumed.png")
    if args.acknowledge_format_error:
        if not driver._attach_to_running_window():
            raise RuntimeError("Expected the observed runtime-error dialog")
        dest = ROOT / "decimal-n0"
        driver.capture(dest / "runtime-error.png")
        (dest / "captured.json").write_text(json.dumps({"name": "decimal-n0", "status": "runtime-error", "message": "Run-time error '5': Invalid procedure call or argument", "screenshot_sha256": hashlib.sha256((dest / "runtime-error.png").read_bytes()).hexdigest()}, indent=2) + "\n")
        driver.key("Return")
        driver.pause(2)
    if args.dismiss_file_error:
        if not driver._attach_to_running_window():
            raise RuntimeError("File-dialog recovery needs the existing TORA window")
        driver.key("Return")
        driver.key("Escape")
        driver.click(957, 768)
        driver.key("n")
    launch_visible(driver)
    if args.startup_only:
        print("TORA startup verified; no completed case repeated", flush=True)
        return 0
    for name, scientific, n, d, objectives, rhs in cases:
        dest = ROOT / name
        marker = "summary-captured.json" if args.summary else "captured.json"
        if (dest / marker).exists():
            print("Already captured:", name, flush=True)
            continue
        dest.mkdir(parents=True, exist_ok=True)
        source = dest / "source.txt"
        source.write_bytes(fixture(name, objectives, rhs))
        install_name = "f2-" + name + ".txt"
        shutil.copyfile(source, oracle.INSTALL / install_name)
        driver.recover_main_menu()
        # Save As changes the reference file picker's current directory. Use
        # an absolute Windows path for every subsequent fixture.
        driver.load_case_from_main("Z:" + str(oracle.INSTALL / install_name).replace("/", "\\"))
        driver.capture(dest / "input.png")
        saved = dest / "tora-saved.txt"
        if not saved.exists() and not args.summary:
            driver.click(571, 290)
            driver.click(590, 326)
            driver.capture(dest / "save-as-dialog.png")
            driver.click(750, 472)
            driver.key("ctrl+a")
            driver.run("xdotool", "type", "--clearmodifiers", "--delay", "2",
                       "Z:" + str(saved).replace("/", "\\"))
            driver.click(932, 471)
            driver.pause(1)
            if not saved.is_file():
                raise RuntimeError("TORA Save As failed: " + name)
            driver.capture(dest / "input-after-save.png")
        if args.summary:
            driver.click(844, 812)
            driver.click(957, 585)
            driver.click(844, 812)
            driver.move(946, 480)
            driver.pause(0.8)
            driver.move(1075, 496)
            driver.pause(0.8)
            driver.click(1130, 498)
        else:
            driver.open_method("all_slack")
        driver.capture(dest / "format-defaults.png")
        if not driver._is_output_format_screen(dest / "format-defaults.png"):
            raise RuntimeError("Probe did not reach output format: " + name)
        if scientific:
            driver.click(858, 502)
        for x, y, value in ((952, 524, n), (952, 541, d)):
            driver.click(x, y)
            driver.key("ctrl+a")
            driver.run("xdotool", "type", "--clearmodifiers", "--delay", "20", value)
        driver.capture(dest / "format-selected.png")
        driver.click(960, 595)
        driver.pause(1)
        driver.move(1300, 825)
        screen = dest / ("summary.png" if args.summary else "iteration-01.png")
        driver.capture(screen)
        if name in ("decimal-n0", "scientific-n0", "scientific-small-large") and driver._looks_gray(driver._png_pixel(screen, 400, 300)):
            shutil.copyfile(screen, dest / "runtime-error.png")
            (dest / marker).write_text(json.dumps({"name": name, "status": "runtime-error", "message": "Pending direct screenshot review", "screenshot_sha256": hashlib.sha256(screen.read_bytes()).hexdigest()}, indent=2) + "\n")
            driver.key("Return")
            driver.pause(2)
            launch_visible(driver)
            print("Captured runtime error:", name, flush=True)
            continue
        if driver._looks_gray(driver._png_pixel(screen, 400, 300)):
            shutil.copyfile(screen, dest / "initial-message.png")
            driver.key("Return")
            driver.capture(screen)
        if not args.summary and not driver._has_tableau_heading(screen):
            raise RuntimeError("Format was not accepted; inspect capture: " + name)
        (dest / marker).write_text(json.dumps({
            "name": name, "scientific": scientific, "N": n, "D": d,
            "objective_lexemes": objectives, "rhs_lexemes": rhs,
            "source_sha256": {p.name: hashlib.sha256(p.read_bytes()).hexdigest()
                              for p in dest.iterdir() if p.suffix in (".png", ".txt")},
            "review": "pending manual transcription; no output inferred",
        }, indent=2) + "\n")
        print("Captured:", name, flush=True)
        driver.recover_main_menu()
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
