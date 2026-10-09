#!/usr/bin/env python3
"""Launch and batch-drive the 800x600 TORA oracle.

This is observation tooling only. It opens saved LP fixtures in TORA, selects
each visible algebraic iteration method, captures the input/iteration/message
screens, and writes a resumable progress manifest under reference/.

The script deliberately requires --execute. It launches TORA in a Wine 800x600
desktop, waits for and validates its window, then drives the LP workflow. Use
--dry-run first. Do not use this to publish captures.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import struct
import subprocess
import sys
import time
import zlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CORPUS = ROOT / "reference/tora-captures/corpus/lp"
INSTALL = ROOT / ".tora-installed"
OUTPUT = ROOT / "reference/tora-captures/methods/lp/corpus"
DISPLAY_CROP = "556,252 808x608"
LEGACY_ORIGIN = (556, 252)
METHODS = ["all_slack", "m_method", "two_phase", "bounded_simplex", "dual_simplex"]
METHOD_Y = {
    "all_slack": 515,
    "m_method": 531,
    "two_phase": 548,
    "bounded_simplex": 564,
    "dual_simplex": 582,
}


class ToraDriver:
    def __init__(self, delay: float, timeout: float):
        self.delay = delay
        self.timeout = timeout
        self.env = os.environ.copy()
        self.env["DISPLAY"] = self.env.get("DISPLAY", ":1")
        self.env["WINEPREFIX"] = str(ROOT / ".tora-oracle-wine")
        self.window_id: str | None = None
        self.window_origin = LEGACY_ORIGIN

    def _arrange_wine_desktop(self) -> None:
        """Keep Wine's 800x600 desktop visible on the current workspace."""
        try:
            clients = json.loads(subprocess.run(
                ["hyprctl", "clients", "-j"], env=self.env, check=True,
                stdout=subprocess.PIPE, text=True,
            ).stdout)
            desktop = next(
                c for c in clients
                if c.get("class") == "explorer.exe" and "ToraOracle800" in c.get("title", "")
            )
            workspace = json.loads(subprocess.run(
                ["hyprctl", "activeworkspace", "-j"], env=self.env, check=True,
                stdout=subprocess.PIPE, text=True,
            ).stdout)["id"]
            address = desktop["address"]
            # A translucent Wine desktop lets unrelated terminal text bleed
            # into captures. Set only this oracle window to full opacity.
            for property_name in ("alpha", "alphainactive"):
                subprocess.run(["hyprctl", "dispatch", "setprop", f"address:{address}",
                                property_name, "1", "override"], env=self.env,
                               check=True, stdout=subprocess.DEVNULL)
            subprocess.run(["hyprctl", "dispatch", "setprop", f"address:{address}",
                            "opaque", "on"], env=self.env,
                           check=True, stdout=subprocess.DEVNULL)
            subprocess.run(["hyprctl", "dispatch", "movetoworkspace",
                            f"{workspace},address:{address}"], env=self.env, check=True,
                           stdout=subprocess.DEVNULL)
            subprocess.run(["hyprctl", "dispatch", "focuswindow", f"address:{address}"],
                           env=self.env, check=True, stdout=subprocess.DEVNULL)
            if not desktop.get("floating", False):
                subprocess.run(["hyprctl", "dispatch", "togglefloating"], env=self.env,
                               check=True, stdout=subprocess.DEVNULL)
            if not desktop.get("pinned", False):
                subprocess.run(["hyprctl", "dispatch", "pin", f"address:{address}"],
                               env=self.env, check=True, stdout=subprocess.DEVNULL)
            subprocess.run(["hyprctl", "dispatch", "resizeactive", "exact", "808", "608"],
                           env=self.env, check=True, stdout=subprocess.DEVNULL)
            subprocess.run(["hyprctl", "dispatch", "moveactive", "exact", "556", "252"],
                           env=self.env, check=True, stdout=subprocess.DEVNULL)
            self.pause(1.0)
        except (OSError, subprocess.CalledProcessError, KeyError, StopIteration,
                json.JSONDecodeError) as exc:
            raise RuntimeError(f"Could not place TORA's Wine desktop on the current workspace: {exc}") from exc

    def _attach_to_running_window(self, focus: bool = True) -> bool:
        result = subprocess.run(
            ["xdotool", "search", "--onlyvisible", "--name", "TORA"], env=self.env,
            stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True,
        )
        for candidate in reversed(result.stdout.splitlines()):
            title = subprocess.run(
                ["xdotool", "getwindowname", candidate], env=self.env,
                stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True,
            ).stdout.strip()
            if "TORA" not in title.upper():
                continue
            if 'Wine Desktop' in title:
                continue
            geometry = subprocess.run(
                ["xdotool", "getwindowgeometry", "--shell", candidate], env=self.env,
                stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True,
            )
            fields = dict(line.split("=", 1) for line in geometry.stdout.splitlines() if "=" in line)
            try:
                width, height = int(fields["WIDTH"]), int(fields["HEIGHT"])
                x, y = int(fields["X"]), int(fields["Y"])
            except (KeyError, ValueError):
                continue
            if 790 <= width <= 820 and 590 <= height <= 630:
                self.window_id = candidate
                # The legacy points were calibrated from Wine's outer desktop
                # at (556,252); the X11 child extends 4px beyond that frame.
                self.window_origin = (x + 4, y + 4)
                if focus:
                    # Raise/focus without waiting for an EWMH acknowledgement.
                    subprocess.run(
                        ["xdotool", "windowraise", candidate], env=self.env,
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                    )
                    subprocess.run(
                        ["xdotool", "windowfocus", candidate], env=self.env,
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                    )
                    subprocess.run(
                        ["xdotool", "windowactivate", candidate], env=self.env,
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                    )
                self.pause(0.5)
                print(f"Attached to TORA window {candidate}: {title!r}, {width}x{height} at {x},{y}")
                return True
        return False

    def launch_tora(self, startup_timeout: float = 45.0) -> None:
        """Start TORA itself and refuse to automate an unrelated desktop."""
        executable = INSTALL / "tora.exe"
        if not executable.is_file():
            raise RuntimeError(f"TORA executable not found: {executable}")
        # Reuse a TORA instance created by a previous interrupted run. Otherwise
        # launch a clean oracle rather than assuming one is already open.
        if self._attach_to_running_window():
            self._arrange_wine_desktop()
            if not self._attach_to_running_window():
                raise RuntimeError("Existing TORA window disappeared while arranging its Wine desktop")
            self.capture(OUTPUT / "startup-screen.png")
            self.enter_main_menu()
            return
        # Wine's explorer creates the 800x600 virtual desktop used by the
        # reference captures. The Z: path avoids relying on a prior install.
        win_path = "Z:" + str(executable).replace("/", "\\")
        log_path = OUTPUT / "tora-launch.log"
        log = log_path.open("w")
        subprocess.Popen(
            ["wine", "explorer", "/desktop=ToraOracle800,800x600", win_path],
            cwd=INSTALL,
            env=self.env,
            stdout=log,
            stderr=subprocess.STDOUT,
            start_new_session=True,
        )
        # The compositor may tile Wine at full size on a different workspace;
        # float it at the calibrated 800x600 reference position automatically.
        time.sleep(2.0)
        self._arrange_wine_desktop()
        deadline = time.monotonic() + startup_timeout
        while time.monotonic() < deadline:
            result = subprocess.run(
                ["xdotool", "search", "--onlyvisible", "--all", "--name", "TORA"],
                env=self.env,
                stdout=subprocess.PIPE,
                stderr=subprocess.DEVNULL,
                text=True,
            )
            candidates = result.stdout.splitlines() if result.returncode == 0 else []
            for candidate in reversed(candidates):
                title = subprocess.run(
                    ["xdotool", "getwindowname", candidate], env=self.env,
                    stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True,
                ).stdout.strip()
                if "TORA" not in title.upper():
                    continue
                geometry = subprocess.run(
                    ["xdotool", "getwindowgeometry", "--shell", candidate], env=self.env,
                    stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True,
                )
                fields = dict(
                    line.split("=", 1) for line in geometry.stdout.splitlines()
                    if "=" in line
                )
                try:
                    width, height = int(fields["WIDTH"]), int(fields["HEIGHT"])
                    x, y = int(fields["X"]), int(fields["Y"])
                except (KeyError, ValueError):
                    continue
                # TORA's drawing surface is 800x600; window-manager framing
                # can change total dimensions, so validate the client bounds.
                if width < 790 or width > 820 or height < 590 or height > 630:
                    continue
                self.window_id = candidate
                self.window_origin = (x, y)
                self.run("xdotool", "windowactivate", "--sync", candidate)
                self.pause(1.0)
                print(f"Validated TORA window {candidate}: {title!r}, {width}x{height} at {x},{y}")
                self.capture(OUTPUT / "startup-screen.png")
                self.enter_main_menu()
                return
            time.sleep(0.5)
        tail = ""
        try:
            tail = "\n".join(log_path.read_text(errors="replace").splitlines()[-20:])
        except OSError:
            pass
        raise RuntimeError(
            f"TORA did not present a visible 800x600 window within {startup_timeout:g}s.\n{tail}"
        )

    def run(self, *args: str) -> None:
        subprocess.run(args, check=True, env=self.env, stdout=subprocess.DEVNULL)

    def pause(self, amount: float | None = None) -> None:
        time.sleep(self.delay if amount is None else amount)

    @staticmethod
    def _png_pixel(path: Path, px: int, py: int) -> tuple[int, int, int]:
        data = path.read_bytes()
        offset, width, height, bit_depth, color_type = 8, 0, 0, 0, 0
        compressed = bytearray()
        while offset < len(data):
            length = struct.unpack(">I", data[offset:offset + 4])[0]
            kind = data[offset + 4:offset + 8]
            chunk = data[offset + 8:offset + 8 + length]
            offset += 12 + length
            if kind == b"IHDR":
                width, height, bit_depth, color_type, _, _, _ = struct.unpack(">IIBBBBB", chunk)
            elif kind == b"IDAT":
                compressed.extend(chunk)
            elif kind == b"IEND":
                break
        if bit_depth != 8 or color_type not in (2, 6) or not (0 <= px < width and 0 <= py < height):
            raise RuntimeError(f"Unsupported screenshot PNG format in {path}")
        bpp = 3 if color_type == 2 else 4
        raw = zlib.decompress(compressed)
        stride = width * bpp
        prior = bytearray(stride)
        for row in range(py + 1):
            start = row * (stride + 1)
            filt = raw[start]
            scan = bytearray(raw[start + 1:start + 1 + stride])
            for i in range(stride):
                left = scan[i - bpp] if i >= bpp else 0
                above = prior[i]
                upper_left = prior[i - bpp] if i >= bpp else 0
                if filt == 1:
                    scan[i] = (scan[i] + left) & 255
                elif filt == 2:
                    scan[i] = (scan[i] + above) & 255
                elif filt == 3:
                    scan[i] = (scan[i] + ((left + above) // 2)) & 255
                elif filt == 4:
                    p = left + above - upper_left
                    pa, pb, pc = abs(p - left), abs(p - above), abs(p - upper_left)
                    predictor = left if pa <= pb and pa <= pc else above if pb <= pc else upper_left
                    scan[i] = (scan[i] + predictor) & 255
            prior = scan
        at = px * bpp
        return prior[at], prior[at + 1], prior[at + 2]

    @staticmethod
    def _looks_gray(rgb: tuple[int, int, int]) -> bool:
        return max(rgb) - min(rgb) < 28 and 100 < sum(rgb) // 3 <= 255

    def enter_main_menu(self) -> None:
        self.recover_main_menu()
        self.capture(OUTPUT / "welcome-screen.png")

    def recover_main_menu(self) -> None:
        state = OUTPUT / "recovery-state.png"
        for attempt in range(10):
            self.capture(state)
            gray = lambda x,y: self._looks_gray(self._png_pixel(state, x,y))
            if gray(335,240) and gray(470,410):
                return  # actual MAIN popup, not the input-mode Go button
            if gray(245,270) and gray(550,340):
                self.key('Return')  # observed entering/leaving-variable message box
                self._attach_to_running_window(focus=True)
                self.pause(0.75)
                continue
            if gray(335,200) and gray(470,270):
                self.click(957,523)  # visible SOLVE/MODIFY MAIN Menu row
                self.pause(0.75)
                continue
            if all(gray(x,y) for x,y in [(440,230),(480,230),(480,240)]):
                self._attach_to_running_window(focus=True)
                self.click(955,534)  # M-value screen Go button
                self.pause(0.75)
                continue
            if gray(190,104) and gray(190,128):
                grid=self._png_pixel(state,400,200)
                initialized=grid[0]<100 and grid[1]>150 and grid[2]>150
                self._attach_to_running_window(focus=True)
                if not initialized and attempt>=2:
                    for x,y,value in [(800,328,'Recovery scratch'),(740,352,'2'),(740,376,'3')]:
                        self.click(x,y);self.key('ctrl+a')
                        self.run('xdotool','type','--clearmodifiers','--delay','20',value)
                    self.key('Return');self.pause(1.5)
                    continue
                self.click(957,812 if initialized else 768)  # initialized grid or blank input
                self.click(957,585)  # No in the observed save prompt
            elif gray(405,277) and gray(405,293):
                self.click(835,745)  # output-format View/Modify
            elif self._has_tableau_heading(state):
                self._attach_to_running_window(focus=True)
                self.click(957,800)  # tableau/final-summary MAIN Menu
            elif gray(350,395) and not gray(470,410):
                self.click(891,800)  # LP input-mode MAIN Menu
            elif gray(400,200) and gray(400,270):
                self.click(957,519)  # SOLVE/MODIFY popup MAIN Menu
            else:
                # Graphical output and the welcome/splash screen share this
                # safe MAIN/Click Here point. Modal cancellation is explicit.
                self.key("Escape")
                self.click(957,800)
            self.pause(0.75)
        raise RuntimeError(f"Could not recover TORA's MAIN popup; inspect {state}")

    def move(self, x: int, y: int) -> None:
        # Coordinates are the calibrated points from the 800x600 desktop at
        # its previous location; shift them with the detected window.
        root_x = self.window_origin[0] + x - LEGACY_ORIGIN[0]
        root_y = self.window_origin[1] + y - LEGACY_ORIGIN[1]
        self.run("xdotool", "mousemove", "--sync", str(root_x), str(root_y))

    def click(self, x: int, y: int, count: int = 1) -> None:
        self.move(x, y)
        self.run("xdotool", "click", "--repeat", str(count), "--delay", "100", "1")
        self.pause()

    def hover(self, x: int, y: int) -> None:
        self.move(x, y)
        self.pause(0.45)

    def key(self, key: str) -> None:
        self.run("xdotool", "key", key)
        self.pause()

    def capture(self, path: Path) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        # TORA recreates its top-level HWND while changing screens. Refresh the
        # handle before each screenshot rather than trusting a stale XID.
        if not self._attach_to_running_window(focus=False):
            raise RuntimeError("Refusing screenshot: no valid TORA window is visible")
        geometry = subprocess.run(
            ["xdotool", "getwindowgeometry", "--shell", self.window_id],
            env=self.env, check=True, stdout=subprocess.PIPE, text=True,
        ).stdout
        fields = dict(line.split("=", 1) for line in geometry.splitlines() if "=" in line)
        x, y = int(fields["X"]), int(fields["Y"])
        width, height = int(fields["WIDTH"]), int(fields["HEIGHT"])
        if not (790 <= width <= 820 and 590 <= height <= 630):
            raise RuntimeError(f"Refusing screenshot of unexpected TORA size {width}x{height}")
        temporary = path.with_name(path.stem + ".capturing.png")
        subprocess.run(
            ["grim", "-g", f"{x},{y} {width}x{height}", str(temporary)],
            check=True,
            env=self.env,
            stdout=subprocess.DEVNULL,
        )
        data = temporary.read_bytes()
        if len(data) < 100 or not data.startswith(b"\x89PNG\r\n\x1a\n") or b"IEND" not in data[-20:]:
            raise RuntimeError(f"Screenshot was empty or interrupted; preserving prior capture: {path}")
        subprocess.run(["magick", "identify", str(temporary)], check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        temporary.replace(path)

    def load_case_from_main(self, basename: str) -> None:
        # Popup MAIN menu -> LP -> existing-file workflow.
        self.click(946, 534)
        self.pause(self.timeout)
        self.capture(OUTPUT / "input-mode.png")
        # The calibrated center click opens the LP input-mode dialog directly.
        self.click(858, 448)
        self.click(955, 635)
        # The native file picker is centered over the fixed TORA desktop.
        self.click(750, 472)
        self.run("xdotool", "key", "ctrl+a")
        self.run("xdotool", "type", "--clearmodifiers", "--delay", "2", basename)
        self.click(932, 471)
        self.pause(self.timeout)
        if not self._attach_to_running_window(focus=False):
            raise RuntimeError(f"TORA window disappeared while opening {basename}")
        title = subprocess.run(
            ["xdotool", "getwindowname", self.window_id], env=self.env,
            stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True,
        ).stdout
        if basename.lower() not in title.lower():
            raise RuntimeError(
                f"TORA did not load {basename}; active TORA title is {title.strip()!r}"
            )

    def return_to_main_from_input(self) -> None:
        # From the LP input grid, MAIN Menu opens TORA's module popup directly.
        # Earlier extra clicks after this point drifted into Linear Equations
        # and made the next fixture load fail unpredictably.
        # The View/Modify target is harmless blank space on the input grid but
        # returns from the output-format screen if a prior interrupted run left
        # TORA there.
        self.recover_main_menu()

    def targeted_probes(self, stage: str = 'all') -> None:
        """Capture targeted UI decisions, without inferring their outcomes."""
        base = ROOT / 'reference/tora-captures/probes/targeted-20261004'
        for scientific in ((False, True) if stage in ('all','notation') else ()):
            name = 'new-scientific' if scientific else 'new-decimal'
            dest = base / name
            self.recover_main_menu()
            self.click(946,534)
            if scientific: self.click(858,511)
            self.capture(dest/'selected-input-format.png')
            self.click(955,635)
            self.pause(self.timeout)
            for x,y,value in [(800,328,name),(740,352,'2'),(740,376,'1')]:
                self.click(x,y);self.key('ctrl+a')
                self.run('xdotool','type','--clearmodifiers','--delay','20',value)
            self.key('Return')
            self.pause(self.timeout)
            self.move(1300,825)
            self.capture(dest/'new-grid.png')
            saved=INSTALL/(name+'-observed.txt')
            if saved.exists():shutil.move(str(saved),str(dest/f'prior-save-{time.time_ns()}.txt'))
            self.click(571,290);self.click(590,326)
            self.click(750,472);self.key('ctrl+a')
            self.run('xdotool','type','--clearmodifiers','--delay','2','Z:'+str(saved).replace('/','\\'))
            self.capture(dest/'save-dialog.png')
            self.click(932,471);self.pause(self.timeout)
            self.capture(dest/'after-save.png')
            if not saved.exists():raise RuntimeError(f'New model was not saved: {name}')
            shutil.copyfile(saved,dest/'tora-saved.txt')
            (dest/'observations.json').write_text(json.dumps({'format_selected_in_ui':name,
                'records':[x.decode('utf-8',errors='replace') for x in saved.read_bytes().splitlines()]},indent=2)+'\n')
        if stage=='notation':return
        if stage in ('all','manual'):
            self.recover_main_menu()
            self.load_case_from_main('01_normal.txt')
            dest=base/'manual-pivot'
            self.capture(dest/'input.png')
            self.open_method('all_slack');self.click(960,595);self.pause(self.timeout)
            for label,point in [('initial',None),('basic-column',(985,466)),
                                ('choose-x2',(875,466)),('choose-x1',(800,466)),
                                ('zero-pivot-row',(700,530)),('choose-sx4',(700,514)),
                                ('next-iteration',(868,432))]:
                if point:self.click(*point)
                self.move(1300,825);self.pause(self.timeout)
                captured=dest/(label+'.png');self.capture(captured)
                if self._looks_gray(self._png_pixel(captured,245,270)) and self._looks_gray(self._png_pixel(captured,550,340)):
                    self.key('Return');self.pause(self.timeout)
                    self._attach_to_running_window(focus=True)
                    self.capture(dest/(label+'-acknowledged.png'))
            if stage=='manual':self.recover_main_menu();return
        self.recover_main_menu()
        self.load_case_from_main('01_normal.txt')
        dest=base/'graph-zoom'
        self.click(844,812);self.click(957,585);self.click(844,812)
        self.hover(946,480);self.click(1075,479);self.pause(self.timeout)
        self.click(960,595);self.pause(self.timeout);self.click(704,570)
        for label,point in [('initial',None),('zoom-in-1',(964,358)),('zoom-in-2',(964,358)),
                            ('zoom-out-1',(1070,358)),('zoom-out-2',(1070,358))]:
            if point:self.click(*point)
            self.move(1300,825);self.pause(self.timeout)
            self.capture(dest/(label+'.png'))
        self.recover_main_menu()
        self.load_case_from_main('01_normal.txt')
        self.open_method('all_slack');self.click(960,595);self.pause(self.timeout)
        self.click(1040,432);self.pause(self.timeout)
        self.capture(base/'printing'/'write-to-printer.png')
        self.key('Escape');self.recover_main_menu()

    def open_method(self, method: str, stop_at_m: bool = False) -> str:
        # Solve Menu asks to save. No is appropriate for these unchanged files.
        self.click(844, 812)
        self.click(957, 585)
        self.click(844, 812)
        self.hover(946, 480)   # Solve Problem
        self.hover(1075, 496)  # Algebraic
        self.hover(1140, 515)  # Iterations
        self.click(1000, METHOD_Y[method])
        if method == "m_method" and not stop_at_m:
            # Keep the oracle's observed default M=100.
            self.click(955, 534)
        return "selected"

    def closing_probes(self, stage: str) -> None:
        """Small resumable probes; never recapture the completed method corpus."""
        base=ROOT/'reference/tora-captures/probes/closing-20261004'
        if stage=='output':
            for name,scientific,decimals in [('scientific',True,'2'),('decimal-three',False,'3')]:
                dest=base/name
                if (dest/'observations.json').exists():continue
                self.recover_main_menu();self.load_case_from_main('13_mixed_sign.txt')
                self.click(844,812);self.click(957,585);self.click(844,812)
                self.hover(946,480);self.hover(1075,496);self.click(1130,498)
                self.pause(self.timeout);self.capture(dest/'format-defaults.png')
                if not self._is_output_format_screen(dest/'format-defaults.png'):
                    raise RuntimeError('Final solution did not reach output format')
                if scientific:self.click(858,502)
                self.click(952,541);self.key('ctrl+a')
                self.run('xdotool','type','--clearmodifiers','--delay','30',decimals)
                self.capture(dest/'format-selected.png');self.click(960,595)
                self.pause(self.timeout);self.move(1300,825);self.capture(dest/'summary.png')
                (dest/'observations.json').write_text(json.dumps({'case':'13_mixed_sign',
                    'requested_scientific':scientific,'requested_N':5,'requested_D':int(decimals),
                    'review':'pending screenshot review'},indent=2)+'\n')
                self.recover_main_menu()
        elif stage=='m':
            for name,value in [('zero','0'),('negative','-1'),('fraction','0.5'),('exponent','1e2'),('nonnumeric','bogus')]:
                dest=base/('m-validated-'+name)
                if (dest/'observations.json').exists():continue
                self.recover_main_menu();self.load_case_from_main('06_equality.txt')
                self.open_method('m_method',stop_at_m=True)
                self._attach_to_running_window(focus=True)
                self.click(1010,485);self.key('Home');self.key('shift+End');self.key('BackSpace')
                self.run('xdotool','type','--clearmodifiers','--delay','30','--',value)
                self.capture(dest/'entered.png');self.click(955,534)
                self.pause(self.timeout);self.move(1300,825);self.capture(dest/'after-go.png')
                accepted=self._is_output_format_screen(dest/'after-go.png')
                (dest/'observations.json').write_text(json.dumps({'entered':value,
                    'reached_output_format':accepted,'review':'pending screenshot review'},indent=2)+'\n')
                if accepted:
                    self.click(960,595);self.pause(self.timeout);self.move(1300,825)
                    self.capture(dest/'initial-output.png')
                if not accepted:
                    self.key('Return');self.pause(self.timeout)
                    if not self._attach_to_running_window():self.launch_tora()
                    else:
                        self.click(1010,485);self.key('ctrl+a')
                        self.run('xdotool','type','--clearmodifiers','--delay','30','100')
                        self.click(955,534);self.pause(self.timeout)
                self.recover_main_menu()
        elif stage=='grid':
            dest=base/'grid-focused'
            if (dest/'observations.json').exists():return
            self.recover_main_menu();self.load_case_from_main('01_normal.txt')
            self._attach_to_running_window(focus=True)
            self.capture(dest/'initial.png')
            # Operate the actual menu commands, with explicit heading selection.
            for name,heading,menu_y in [('copy',(787,496),326),
                                       ('paste',(787,514),342),
                                       ('undo-paste',(787,514),309),
                                       ('delete',(787,514),359),
                                       ('undo-delete',(787,496),309),
                                       ('insert',(787,496),377)]:
                self._attach_to_running_window(focus=True)
                self.key('Escape');self.click(*heading)
                self.click(612,290);self.capture(dest/(name+'-menu.png'))
                self.click(670,menu_y);self.pause(self.timeout)
                if name=='insert':self.click(787,496);self.pause(self.timeout)
                self.move(1300,825);self.capture(dest/(name+'.png'))
            (dest/'observations.json').write_text(json.dumps({'case':'01_normal',
                'commands':['copy','paste','undo-paste','delete','undo-delete','insert'],
                'review':'pending screenshot review'},indent=2)+'\n')
            self.recover_main_menu()

    def file_probes(self) -> None:
        base=ROOT/'reference/tora-captures/probes/file-errors-20261004'
        source=(CORPUS/'01_normal/problem.txt').read_bytes()
        probes={'wrong-marker':source.replace(b'3000\r\n',b'9999\r\n',1),
                'truncated':b'\r\n'.join(source.splitlines()[:15])+b'\r\n',
                'extra-record':source+b'"extra"\r\n',
                'lf-only':source.replace(b'\r\n',b'\n')}
        for name,contents in probes.items():
            dest=base/name;dest.mkdir(parents=True,exist_ok=True)
            if (dest/'after-open.png').exists():continue
            path=INSTALL/('probe-file-'+name+'.txt');path.write_bytes(contents)
            shutil.copyfile(path,dest/'attempted-input.txt')
            self.recover_main_menu();self.click(946,534);self.click(858,448);self.click(955,635)
            self.click(750,472);self.key('ctrl+a')
            self.run('xdotool','type','--clearmodifiers','--delay','2',path.name)
            self.capture(dest/'file-picker.png');self.click(932,471);self.pause(self.timeout)
            self.capture(dest/'after-open.png')
            if name=='wrong-marker':self.click(996,589)
            else:self.key('Return')
            self.pause(1.5)
            running=self._attach_to_running_window(focus=True)
            (dest/'observations.json').write_text(json.dumps({'name':name,
                'after_open_capture':'after-open.png','tora_running_after_acknowledge':running},indent=2)+'\n')
            if running:
                self.capture(dest/'after-acknowledge.png');self.recover_main_menu()
            else:self.launch_tora()

    def validation_probes(self, skip_fallback: bool = False) -> None:
        base=ROOT/'reference/tora-captures/probes/validation-20261004'
        if not skip_fallback:
            self.recover_main_menu();self.load_case_from_main('03_infeasible.txt')
            self.open_method('all_slack');self.capture(base/'cancel-to-m'/'redirect-warning.png')
            self.key('Escape');self.pause(self.timeout)
            self.capture(base/'cancel-to-m'/'after-cancel.png')
            self._attach_to_running_window(focus=True)
            self.click(955,534);self.pause(max(2.0,self.timeout))
            self.capture(base/'cancel-to-m'/'after-m-value.png')
        self.recover_main_menu()

        self.load_case_from_main('probe-3var-2con.txt')
        self.click(844,812);self.click(957,585);self.click(844,812)
        self.hover(946,480);self.click(1075,479);self.pause(self.timeout)
        self.capture(base/'graph-three-vars'/'after-selection.png')
        self.key('Return');self._attach_to_running_window(focus=True);self.recover_main_menu()
        self.load_case_from_main('01_normal.txt')
        dest=base/'keyboard-grid'
        self.click(757,498);self.key('ctrl+c');self.capture(dest/'after-copy.png')
        self.click(612,290);self.capture(dest/'after-copy-menu.png');self.key('Escape')
        self.key('ctrl+d');self.pause(self.timeout);self.capture(dest/'after-delete.png')
        self.key('ctrl+u');self.pause(self.timeout);self.capture(dest/'after-undo.png')
        self.recover_main_menu()

    def alternate_probe(self) -> None:
        dest=ROOT/'reference/tora-captures/probes/alternative-20261004'
        self.recover_main_menu();self.load_case_from_main('05_alternative.txt')
        self.open_method('all_slack');self.click(960,595);self.pause(self.timeout)
        self.click(955,432);self.pause(self.timeout);self.capture(dest/'first-optimal-notice.png')
        self.key('Return');self._attach_to_running_window(focus=True)
        self.capture(dest/'before-alternate.png')
        self.click(875,621);self.pause(self.timeout);self.capture(dest/'choose-zero-cost-x2.png')
        self.click(868,432);self.pause(self.timeout);self.capture(dest/'after-next.png')
        self.key('Return');self._attach_to_running_window(focus=True)
        self.capture(dest/'after-notice.png')
        self.capture_all_iterations('all_slack',dest)
        self.recover_main_menu()

    def capture_all_iterations(self, method: str, dest: Path, return_to_input: bool = True) -> dict:
        self.click(955, 432)
        self.pause(self.timeout)
        self.capture(dest / "terminal-message.png")
        self.key("Return")
        self.capture(dest / "tableau-top.png")
        # Artificial-variable columns widen the actual MSFlexGrid. Find its
        # cyan surface rather than assuming the all-slack scrollbar location.
        left, right, bottom = self._table_bounds(dest / "tableau-top.png")
        offset = getattr(self,'capture_frame_offset',4)
        if bottom < 495:
            # A horizontal scrollbar is present. All Iterations can leave it
            # scrolled to the final variable; return to the leftmost columns.
            self.move(LEGACY_ORIGIN[0] + left + 8 - offset,
                      LEGACY_ORIGIN[1] + bottom + 10 - offset)
            self.run("xdotool", "click", "--repeat", "40", "--delay", "20", "1")
            self.pause(0.5)
        offset = getattr(self,'capture_frame_offset',4)
        scroll_x = LEGACY_ORIGIN[0] + right + 8 - offset
        scroll_y = LEGACY_ORIGIN[1] + bottom - 7 - offset
        self.move(1300, 825)
        self.capture(dest / "tableau-top.png")
        if bottom < 495:
            self._capture_right_columns(dest / "tableau-top-right.png", left, right, bottom)
        prior = None
        pages = 0
        stable = 0
        for page in range(1, 41):
            self.move(scroll_x, scroll_y)
            # Eight rows still overlap by more than half the viewport.
            self.run("xdotool", "click", "--repeat", "8", "--delay", "35", "1")
            self.pause(0.5)
            self.move(1300, 825)
            path = dest / f"tableau-scroll-{page:02d}.png"
            self.capture(path)
            if bottom < 495:
                self._capture_right_columns(dest / f"tableau-scroll-{page:02d}-right.png", left, right, bottom)
            pixels = subprocess.check_output(["magick", str(path), "-crop", "710x310+40+194", "-depth", "8", "rgb:-"])
            digest = hashlib.sha256(pixels).hexdigest()
            stable = stable + 1 if digest == prior else 0
            prior = digest
            pages = page
            if stable >= 2:
                break
        if stable < 2:
            raise RuntimeError(f"Tableau scrolling did not reach a stable bottom view: {dest}")
        if return_to_input:
            self.click(835, 800)  # View/Modify Input Data
            self.pause(self.timeout)
        return {"status": "captured-all-iterations", "captures": pages + 4,
                "scroll_pages": pages, "scroll_bottom_stable": stable >= 2,
                "grid_bounds": [left, right, bottom],
                "horizontal_columns_captured": bottom < 495}

    def _capture_right_columns(self, path: Path, left: int, right: int, bottom: int) -> None:
        for x, capture in ((right - 8, True), (left + 8, False)):
            offset = getattr(self,'capture_frame_offset',4)
            self.move(LEGACY_ORIGIN[0] + x - offset, LEGACY_ORIGIN[1] + bottom + 10 - offset)
            self.run("xdotool", "click", "--repeat", "40", "--delay", "20", "1")
            self.pause(0.5)
            self.move(1300, 825)
            if capture:
                self.capture(path)

    @staticmethod
    def _table_bounds(path: Path) -> tuple[int, int, int]:
        raw = subprocess.check_output(["magick", str(path), "-depth", "8", "rgb:-"])
        width=int(subprocess.check_output(['magick','identify','-format','%w',str(path)],text=True))
        def cyan(x: int, y: int) -> bool:
            at = (y * width + x) * 3
            r, g, b = raw[at:at + 3]
            return r < 100 and g > 150 and b > 150
        surface = [(x, y) for y in range(194, 508)
                   for x in range(40, 765) if cyan(x, y)]
        if not surface:
            raise RuntimeError(f"No TORA tableau surface found: {path}")
        left = min(x for x, _ in surface)
        right = max(x for x, _ in surface)
        bottom = max(y for _, y in surface)
        # The scrollbar itself can contain cyan pixels in its arrow glyph.
        # Locate its wide gray strip so those glyphs do not widen the grid.
        gray_columns = []
        for x in range(40, 765):
            gray = 0
            for y in range(240, 480):
                at = (y * width + x) * 3
                p = raw[at:at + 3]
                gray += min(p) > 100 and max(p) - min(p) < 30
            if gray > 180:
                gray_columns.append(x)
        groups = []
        for x in gray_columns:
            if not groups or x > groups[-1][-1] + 1:
                groups.append([x])
            else:
                groups[-1].append(x)
        scrollbars = [g for g in groups if len(g) >= 8]
        if scrollbars:
            right = scrollbars[-1][0] - 1
        return left, right, bottom

    def _is_output_format_screen(self, path: Path) -> bool:
        # In TORA's fixed 800x600 client, the decimal-place edit box is white
        # at this point. A method-selection warning has a gray dialog surface
        # there instead. Checking this prevents the old driver from clicking
        # through the wrong modal and saving input-screen frames as tableaux.
        sample = self._png_pixel(path, 350, 277)
        # This point is blue on the N-field label's background, but lies inside
        # the gray text area of the centered warning dialogs. Pure equality on
        # the edit-box color was too brittle under Wine's color blending.
        return sample[2] > sample[0] + 60

    @staticmethod
    def _has_tableau_heading(path: Path) -> bool:
        data = path.read_bytes()
        offset, width, height, bit_depth, color_type = 8, 0, 0, 0, 0
        compressed = bytearray()
        while offset < len(data):
            length = struct.unpack(">I", data[offset:offset + 4])[0]
            kind = data[offset + 4:offset + 8]
            chunk = data[offset + 8:offset + 8 + length]
            offset += 12 + length
            if kind == b"IHDR":
                width, height, bit_depth, color_type, _, _, _ = struct.unpack(">IIBBBBB", chunk)
            elif kind == b"IDAT":
                compressed.extend(chunk)
            elif kind == b"IEND":
                break
        if bit_depth != 8 or color_type not in (2, 6):
            return False
        bpp = 3 if color_type == 2 else 4
        raw = zlib.decompress(compressed)
        stride = width * bpp
        prior = bytearray(stride)
        bright = 0
        for y in range(106):
            start = y * (stride + 1)
            filt = raw[start]
            row = bytearray(raw[start + 1:start + 1 + stride])
            for i in range(stride):
                left = row[i - bpp] if i >= bpp else 0
                above = prior[i]
                upper_left = prior[i - bpp] if i >= bpp else 0
                if filt == 1:
                    row[i] = (row[i] + left) & 255
                elif filt == 2:
                    row[i] = (row[i] + above) & 255
                elif filt == 3:
                    row[i] = (row[i] + ((left + above) // 2)) & 255
                elif filt == 4:
                    p = left + above - upper_left
                    pa, pb, pc = abs(p - left), abs(p - above), abs(p - upper_left)
                    predictor = left if pa <= pb and pa <= pc else above if pb <= pc else upper_left
                    row[i] = (row[i] + predictor) & 255
            prior = row
            if 90 <= y <= 97:
                # Exclude input-grid editing instructions and white title
                # fields, which otherwise resemble an output heading.
                for x in range(260, min(380, width)):
                    at = x * bpp
                    r, g, b = row[at:at + 3]
                    if min(r, g, b) > 190 and max(r, g, b) - min(r, g, b) < 50:
                        bright += 1
        return bright >= 8

    def run_method(self, method: str, case_name: str, case_dir: Path) -> dict:
        dest = OUTPUT / case_name / method
        dest.mkdir(parents=True, exist_ok=True)
        prior_files = [p for p in dest.iterdir() if p.is_file() and p.name != "tora-saved.txt"]
        if prior_files:
            archive = OUTPUT.parent / "corpus-superseded-20261003" / case_name / method / str(time.time_ns())
            archive.mkdir(parents=True, exist_ok=True)
            for source in prior_files:
                shutil.move(str(source), str(archive / source.name))
        self.capture(dest / "input.png")
        self.open_method(method)

        if method == "dual_simplex" and case_name in {"06_equality", "08_two_phase"}:
            self.capture(dest / "input-requirement-warning.png")
            self.key("Return")
            return {"status": "directly-observed-input-warning", "captures": 2}

        # A method may reject the current relation types and show a warning
        # before the common output-format dialog. Capture and acknowledge that
        # modal at its actual OK button, rather than treating it as the format
        # screen (the earlier behavior silently produced false tableau files).
        setup = dest / "method-setup.png"
        self.capture(setup)
        had_method_warning = not self._is_output_format_screen(setup)
        if had_method_warning:
            shutil.copyfile(setup, dest / "method-selection-warning.png")
            # TORA gives the default OK button keyboard focus in both its
            # one-button warnings and its OK/Cancel fallback warnings.
            self.key("Return")
            # Wine can acknowledge the input immediately while its old modal
            # remains in the compositor for another frame. Wait for a redraw
            # and recapture once before deciding the warning is unresolved.
            self.pause(max(self.timeout, 2.0))
            self.capture(setup)
            if not self._is_output_format_screen(setup):
                self.pause(1.0)
                self.capture(setup)
        if not self._is_output_format_screen(setup):
            self.capture(dest / "method-selection-warning-unresolved.png")
            raise RuntimeError(
                f"Method selection did not reach TORA's output-format screen: {dest}"
            )
        shutil.copyfile(setup, dest / "output-format.png")
        self.click(960, 595)
        self.pause(max(self.timeout, 1.5))
        self.move(1300, 825)
        self.capture(dest / "iteration-01.png")
        if not self._has_tableau_heading(dest / "iteration-01.png"):
            self.pause(1.0)
            self.capture(dest / "iteration-01.png")
        if not self._has_tableau_heading(dest / "iteration-01.png"):
            raise RuntimeError(
                f"TORA did not show a recognizable tableau after output setup: {dest}"
            )
        # All Iterations runs to TORA's own stopping condition; capture its
        # terminal dialog and overlapping scroll frames of the accumulated rows.
        result = self.capture_all_iterations(method, dest)
        result["tableau_heading_verified"] = True
        if had_method_warning:
            result["status"] = "captured-all-iterations-after-method-warning"
            result["initial_warning"] = "method-selection-warning.png"
            result["captures"] += 2
        else:
            result["captures"] += 1  # method-setup.png is an additional state check.
        return result


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--execute", action="store_true", help="perform GUI input and save captures")
    ap.add_argument("--startup-only", action="store_true", help="launch TORA, capture startup flow, then stop")
    ap.add_argument("--probe-format", nargs=2, type=int, metavar=("N", "D"), help="probe the LP input-mode notation width and decimal-place fields")
    ap.add_argument("--inspect-input", action="store_true", help="capture File and EditGrid menus for selected input fixtures")
    ap.add_argument("--save-roundtrip", action="store_true", help="save selected fixtures using TORA, read bytes, then reopen in TORA")
    ap.add_argument("--fixture", type=Path, help="load an additional observation-only fixture outside the 15-case manifest")
    ap.add_argument("--fixtures", nargs="*", type=Path, help="load several additional observation fixtures")
    ap.add_argument("--repair-plan", type=Path, help="read exact case/method pairs from a saved capture audit plan")
    ap.add_argument("--capture-final", action="store_true", help="capture the algebraic final summary for selected fixtures")
    ap.add_argument("--capture-graphical", action="store_true", help="capture graphical rendering and zoom for selected fixtures")
    ap.add_argument("--targeted-probes", action="store_true", help="observe new notation modes, manual pivot, second zoom and print control")
    ap.add_argument("--file-probes", action="store_true", help="observe malformed file and record framing behavior")
    ap.add_argument("--validation-probes", action="store_true", help="observe fallback Cancel, graph dimensions and grid shortcuts")
    ap.add_argument("--skip-fallback", action="store_true", help="resume graph/grid probes after the completed Cancel-to-M observation")
    ap.add_argument("--alternate-probe", action="store_true", help="observe manual alternate-optimum navigation")
    ap.add_argument("--closing-probes", choices=['output','m','grid'], help="resume focused output-format, M-validation or grid-command observations")
    ap.add_argument("--probe-stage", choices=['all','notation','manual','zoom-print'], default='all', help='resume only unfinished targeted probe stages')
    ap.add_argument("--input-ready", action="store_true", help="recover an already-open LP input screen without closing TORA")
    ap.add_argument("--resume-current", help="continue from a visible TORA tableau, e.g. 01_normal/all_slack")
    ap.add_argument("--dry-run", action="store_true", help="list planned fixture/method combinations")
    ap.add_argument("--cases", nargs="*", help="case names, e.g. 01_normal 10_dual; defaults to all 15")
    ap.add_argument("--methods", nargs="*", choices=METHODS, help="methods; defaults to all five")
    ap.add_argument("--pairs", nargs="*", help="exact CASE/METHOD pairs to run, e.g. 03_infeasible/dual_simplex")
    ap.add_argument("--force", action="store_true", help="recapture selected case/method pairs already in the manifest")
    ap.add_argument("--delay", type=float, default=0.35, help="delay between GUI actions in seconds")
    ap.add_argument("--timeout", type=float, default=0.8, help="extra wait after loading/solving")
    args = ap.parse_args()

    manifest = json.loads((CORPUS / "manifest.json").read_text())
    all_cases = [entry["case"] for entry in manifest]
    fixture_sources = {}
    for source in ([args.fixture] if args.fixture else []) + (args.fixtures or []):
        source = source.resolve()
        if not source.is_file():
            ap.error("observation fixtures must name existing text files")
        fixture_sources[source.stem] = source
        all_cases.append(source.stem)
    if args.repair_plan:
        args.pairs = json.loads(args.repair_plan.read_text())["pairs"]
    requested_pairs = None
    if args.pairs is not None:
        requested_pairs = []
        for item in args.pairs:
            parts = item.split("/", 1)
            if len(parts) != 2 or parts[0] not in all_cases or parts[1] not in METHODS:
                ap.error(f"invalid --pairs entry {item!r}; expected CASE/METHOD")
            requested_pairs.append((parts[0], parts[1]))
        cases = list(dict.fromkeys(case for case, _ in requested_pairs))
        methods = list(dict.fromkeys(method for _, method in requested_pairs))
    else:
        cases = args.cases or (list(fixture_sources) if fixture_sources else all_cases)
        methods = args.methods or METHODS
    unknown = sorted(set(cases) - set(all_cases))
    if unknown:
        ap.error(f"unknown case name(s): {', '.join(unknown)}")
    plan = requested_pairs or [(case, method) for case in cases for method in methods]
    observation_mode = args.startup_only or args.probe_format or args.inspect_input or args.save_roundtrip or args.capture_final or args.capture_graphical or args.targeted_probes or args.file_probes or args.validation_probes or args.alternate_probe or args.closing_probes
    if observation_mode:
        print(f"Planned TORA screen/file observations: {len(cases)} selected fixture(s)")
    else:
        print(f"Planned TORA method runs: {len(plan)}")
        for case, method in plan:
            print(f"  {case}/{method}")
    if args.dry_run:
        return 0
    if not args.execute:
        ap.error("pass --dry-run or --execute")

    OUTPUT.mkdir(parents=True, exist_ok=True)
    progress_path = OUTPUT / "progress.json"
    progress = json.loads(progress_path.read_text()) if progress_path.exists() else {"runs": {}}
    driver = ToraDriver(args.delay, args.timeout)

    # Stage case files where TORA's native file picker can open them by name.
    for case in cases:
        source = fixture_sources.get(case, CORPUS / case / "problem.txt")
        shutil.copyfile(source, INSTALL / f"{case}.txt")

    # Validate/reuse TORA or launch it if absent; never drive an unrelated app.
    resume_case = resume_method = None
    if args.resume_current:
        parts = args.resume_current.split("/", 1)
        if len(parts) != 2 or parts[0] not in all_cases or parts[1] not in METHODS:
            ap.error("--resume-current must be a known CASE/METHOD pair")
        resume_case, resume_method = parts
        if not driver._attach_to_running_window():
            ap.error("--resume-current requires the already-visible TORA tableau")
        dest = OUTPUT / resume_case / resume_method
        dest.mkdir(parents=True, exist_ok=True)
        driver.capture(dest / "iteration-01.png")
        result = driver.capture_all_iterations(resume_method, dest)
        progress["runs"][f"{resume_case}/{resume_method}"] = result
        progress_path.write_text(json.dumps(progress, indent=2) + "\n")
    else:
        if args.input_ready:
            if not driver._attach_to_running_window():
                ap.error("--input-ready requires the already-visible TORA LP input screen")
            driver.return_to_main_from_input()
            driver.capture(OUTPUT / "startup-screen.png")
        else:
            driver.launch_tora()
        if args.startup_only:
            print(f"Validated startup captures in {OUTPUT}")
            return 0
        if args.targeted_probes:
            driver.targeted_probes(args.probe_stage)
            return 0
        if args.file_probes:
            driver.file_probes()
            return 0
        if args.validation_probes:
            driver.validation_probes(args.skip_fallback)
            return 0
        if args.alternate_probe:
            driver.alternate_probe()
            return 0
        if args.closing_probes:
            driver.closing_probes(args.closing_probes)
            return 0
        if args.probe_format:
            format_n, format_d = args.probe_format
            probe = ROOT / "reference/tora-captures/probes" / f"format-n{format_n}-d{format_d}"
            driver.click(946, 534)  # Linear Programming from TORA's MAIN popup
            driver.pause(args.timeout)
            driver.capture(probe / "input-mode-defaults.png")
            for x, y, value in ((932, 546, format_n), (932, 564, format_d)):
                driver.click(x, y)
                driver.key("ctrl+a")
                driver.run("xdotool", "type", "--clearmodifiers", "--delay", "40", str(value))
            driver.capture(probe / "input-mode-probe-values.png")
            driver.click(955, 635)  # Go to Input Screen; do not initialize a model grid
            driver.pause(1.0)
            driver.capture(probe / "after-go-to-input.png")
            print(f"Saved notation-format probe captures in {probe}")
            return 0
    for case in cases:
        basename = f"{case}.txt"
        if case != resume_case:
            driver.load_case_from_main(basename)
        if args.capture_final or args.capture_graphical:
            for view in (["graphical"] if args.capture_graphical else []) + (["final"] if args.capture_final else []):
                probe = ROOT / "reference/tora-captures/output-studies" / case / view
                driver.capture(probe / "input.png")
                driver.click(844, 812)
                driver.click(957, 585)
                driver.click(844, 812)
                driver.hover(946, 480)
                if view == "graphical":
                    driver.click(1075, 479)
                    driver.pause(max(args.timeout, 1.5))
                    driver.capture(probe / "output-format.png")
                    if not driver._is_output_format_screen(probe / "output-format.png"):
                        raise RuntimeError(f"Graph selection did not reach the format screen: {probe}")
                    driver.click(960, 595)
                    driver.pause(max(args.timeout, 1.5))
                    driver.capture(probe / "graph-initial.png")
                    samples = [driver._png_pixel(probe / "graph-initial.png", x, y)
                               for x,y in [(700,450),(702,450),(701,447),(650,400),(651,402)]]
                    if not any(p[0] < 100 and p[1] > 150 and p[2] > 150 for p in samples):
                        raise RuntimeError(f"Graph selection did not show the graphical canvas: {probe}")
                    driver.click(704, 570)
                    driver.move(1300, 825)
                    driver.pause(max(args.timeout, 1.5))
                    driver.capture(probe / "graph-one-stroke.png")
                    driver.click(964, 358)
                    driver.move(1300, 825)
                    driver.pause(args.timeout)
                    driver.capture(probe / "graph-zoom-in.png")
                    driver.click(1070, 358)
                    driver.move(1300, 825)
                    driver.pause(args.timeout)
                    driver.capture(probe / "graph-zoom-out.png")
                else:
                    driver.hover(1075, 496)
                    driver.click(1140, 496)
                    driver.capture(probe / "output-format.png")
                    driver.click(960, 595)
                    driver.move(1300, 825)
                    driver.pause(max(args.timeout, 1.5))
                    driver.capture(probe / "final-output.png")
                    if not driver._has_tableau_heading(probe / "final-output.png"):
                        raise RuntimeError(f"Final solution did not show its output heading: {probe}")
                driver.click(835, 800)
                driver.pause(args.timeout)
            driver.return_to_main_from_input()
            driver.capture(OUTPUT / "startup-screen.png")
            continue
        if args.save_roundtrip:
            probe = ROOT / "reference/tora-captures/roundtrips" / case
            probe.mkdir(parents=True, exist_ok=True)
            saved_name = f"{case}-tora-saved.txt"
            saved = INSTALL / saved_name
            if saved.exists():
                stamp = time.time_ns()
                shutil.move(str(saved), str(probe / f"prior-save-{stamp}.txt"))
            driver.capture(probe / "input-before-save.png")
            driver.click(571, 290)
            driver.click(590, 326)
            driver.capture(probe / "save-as-dialog.png")
            driver.click(750, 472)
            driver.key("ctrl+a")
            win_saved = "Z:" + str(saved).replace("/", "\\")
            driver.run("xdotool", "type", "--clearmodifiers", "--delay", "2", win_saved)
            driver.capture(probe / "save-as-filename.png")
            driver.click(932, 471)
            driver.pause(max(args.timeout, 1.5))
            driver.capture(probe / "input-after-save.png")
            if not saved.is_file() or not saved.stat().st_size:
                raise RuntimeError(f"TORA did not save the requested file: {saved}")
            shutil.copyfile(saved, probe / "tora-saved.txt")
            source = fixture_sources.get(case, CORPUS / case / "problem.txt")
            original = source.read_bytes()
            actual = saved.read_bytes()
            changes = [{'record': i+1, 'input': a.decode('utf-8', errors='replace'),
                        'saved': b.decode('utf-8', errors='replace')}
                       for i,(a,b) in enumerate(zip(original.splitlines(), actual.splitlines())) if a != b]
            driver.return_to_main_from_input()
            driver.load_case_from_main(saved_name)
            driver.capture(probe / "input-reopened.png")
            (probe / "observations.json").write_text(json.dumps({
                'case': case, 'saved_by': 'TORA File > Save As', 'bytes': len(actual),
                'record_count': len(actual.splitlines()), 'byte_identical': actual == original,
                'record_changes': changes, 'reopened_in_tora': True,
            }, indent=2)+'\n')
            driver.return_to_main_from_input()
            driver.capture(OUTPUT / "startup-screen.png")
            continue
        if args.inspect_input:
            probe = ROOT / "reference/tora-captures/probes/input-menus" / case
            driver.capture(probe / "input.png")
            driver.click(571, 290)
            driver.capture(probe / "file-menu.png")
            driver.key("Escape")
            driver.click(611, 290)
            driver.capture(probe / "editgrid-menu.png")
            driver.key("Escape")
            driver.click(757, 498)
            driver.click(612, 290)
            driver.capture(probe / "editgrid-row-selected.png")
            driver.key("Escape")
            driver.return_to_main_from_input()
            driver.capture(OUTPUT / "startup-screen.png")
            continue
        case_methods = [method for pair_case, method in plan if pair_case == case]
        for method in case_methods:
            key = f"{case}/{method}"
            prior = progress["runs"].get(key, {})
            completed = prior.get("status", "").startswith("captured-all-iterations") or prior.get("status") == "directly-observed-input-warning"
            if (completed and not args.force) or (case == resume_case and method == resume_method):
                # run_method leaves TORA on this case's input grid.
                continue
            progress["runs"][key] = {"status": "capture-in-progress", "previous_status": prior.get("status")}
            progress_path.write_text(json.dumps(progress, indent=2) + "\n")
            try:
                result = driver.run_method(method, case, CORPUS / case)
            except Exception as exc:
                progress["runs"][key] = {"status": "capture-failed", "error": str(exc)}
                progress_path.write_text(json.dumps(progress, indent=2) + "\n")
                raise
            progress["runs"][key] = result
            progress_path.write_text(json.dumps(progress, indent=2) + "\n")
            if args.repair_plan:
                audit = json.loads(args.repair_plan.read_text())
                audit["pairs"] = [p for p in audit["pairs"] if p != key]
                audit.setdefault("recaptured", []).append(key)
                args.repair_plan.write_text(json.dumps(audit, indent=2) + "\n")
        driver.return_to_main_from_input()
        driver.capture(OUTPUT / "startup-screen.png")
    print(f"Saved progress manifest: {progress_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
