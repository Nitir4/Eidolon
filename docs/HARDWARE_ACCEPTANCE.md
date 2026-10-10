# Hardware acceptance still requiring real devices

The completed offscreen tests and synthetic screen changes are SDK regression
evidence. They do not establish physical hotplug, OS printer-panel interaction
or acceptance on every supported OS version. On this laptop, actual Wayland
fullscreen rendering at1920×1080 and the real Linux Qt print dialog
(cancel/PDF selection) have now passed; the PDF matches139 original normal
print tokens and page count. One display is connected and Qt lists no configured
printer. Physical mixed-monitor/hotplug and printer checks remain outstanding.

Use a verified package from the CI run recorded in STATUS.json. Record the
package SHA, OS/version, GPU/compositor, monitor model, native resolution, scale
and printer/driver. Keep captures/results private unless publication is
separately authorized. A failed case must remain failed until investigated.

## Common example and screens

Enter Maximize z = 3x1 + 2x2 with x1+x2 <= 4, x1 <= 2, x2 <= 3 and default
nonnegative variables. Expected final values: x1=2, x2=2, z=10. First RHS
sensitivity range: 2 through 5; dual price 2. Test populated input, All-slack
iterations, graphical one-stroke solution and final/sensitivity output. Save
the document before moving displays and compare it afterward.

## Real displays

For each available resolution (1366×768, 1920×1080 and 3840×2160) and supported
OS scale (100%, 125%, 150%, 200%), check full content, readable glyphs, unclipped
buttons, table scrolling, graph alignment and keyboard navigation. Record
unavailable configurations as not tested, never passed.

With two monitors at different scales, move each populated screen to the second
monitor and back. Change that monitor's scale while the window remains open.
Disconnect/reconnect it, then restore the original OS display settings. Check
that the window returns to a reachable area, no text becomes boxes/blurry, and
model values, iteration history and selection remain intact. Repeat for each
target OS/compositor actually available.

## Printer and native panel

On a host with a real default printer, print original input plus final/sensitivity,
All-slack/M/Two-phase/Bounded/Dual traces and a graph. The default route should
send directly. Record physical output, paper size, driver and page count;
compare with original TORA using the same input/format/driver where possible.
Do not assume another driver reproduces the captured OraclePS page geometry.

Without a default printer, open the native selection/PDF panel, cancel it, and
confirm input/history are unchanged. Then choose PDF output and inspect the
actual document. Windows/macOS native panel interaction remains a manual OS
check; offscreen CI tests its cancellation routing through an injected selector.

## Recording a result

Record case ID, environment, steps, expected result, actual result, pass/fail/not
tested, private evidence path and any mismatch. A desktop screenshot alone does
not establish a printed numeric comparison or a successful monitor transition.
These checks require devices; they are not outstanding solver implementation.


## Repeat the real-desktop Linux check

```sh
cmake --build --preset dev --target desktop_native_acceptance
QT_QPA_PLATFORM=wayland build/dev/tests/desktop_native_acceptance \
  --problem build/dev/tests/codec-goldens/01_normal.txt \
  --output build/manual-native-check --fullscreen --print-dialog
```

Use a fresh output directory. This manual tool opens only its own window,
cancels the real dialog, and chooses its private PDF path. It refuses a physical
print job. It does not change monitor settings or terminate existing TORA
processes. It is intentionally outside offscreen CTest. Inspect all six images
and `verification.json`; this SDK source check is separate from deployed-package
startup. See `RELEASE_READINESS.md` for evidence and limits.
