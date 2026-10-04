# Foundation 5 — native shell and navigation

Gate: implementation and local verification complete; three-OS verification
is being run. Stop for the user's review when verification finishes. Do not
begin Foundations 2, 3 or 4. The user changed the approved order to
1 → 5 → 2 → 3 → 4 after Foundation 1 passed all three OS builds/tests/packages.

## Result and scope

The blank bootstrap has been replaced by a native Qt 6 Widgets main window,
the blue/cyan TORA palette, original welcome text, MAIN module menu and six
additional screen frameworks. The exact LP menu order is Graphical/Algebraic,
Final solution/Iterations, then All-slack starting solution, M-method,
Two-phase method, Bounded simplex, Dual simplex. The File and EditGrid menus
retain their observed labels and shortcuts. Other OR modules are disabled.

Welcome → MAIN → LP input mode → empty input page → SOLVE/MODIFY navigation
works. New/existing and decimal/scientific radio states are retained when
returning to a page. MAIN/View-Modify/Exit commands work. Output format and M
frameworks are registered for direct layout review; defaults are N=5/D=2 and
M=100. The numerical formatter, model, files, editing and solvers are **not**
implemented or simulated. Their commands/fields are visibly unavailable.

The first interactive launch maximizes a normally resizable window, and the
blue canvas fills its content area. Centered panels use native/font-based
hints, not legacy image coordinates. Every page has scrolling for small
logical viewports. Settings store normal geometry/maximized state, and
individual screen rectangles bound restored/disconnected-monitor positions.
Screen/DPR/font events invalidate layouts and repaint native widgets.

`desktop_shell` is a separate static library. Modules register named screen
factories and menus; duplicate identifiers/routes are rejected. An extension
fixture verifies that a registration appears in MAIN and navigates without
rewriting the shell. No additional OR module is built.

## Missing-glyph correction

The first MAIN preview still reproduced the user's boxes: styled descendants
and internal popup menus could select the malformed qt6ct application font
even though the welcome title had a repaired font. The fix now repairs the
application font, checks descendants after polishing and uses Qt's classic
Fusion widget style consistently on all OSes. This uses the required Qt
Widgets toolkit and leaves the user's global theme configuration untouched.
Regression checks cover every screen and popup, including character coverage.

The corrected native Wayland MAIN was inspected on the laptop at 1920×1080:
the canvas fills desktop 2's available area and all menu text is readable.
The system panel remains outside the application's normal maximized window.
The clone does not require changing the desktop to 800×600 or running Wine.

## Validation evidence

| Check | Result |
| --- | --- |
| Local Debug build/full CTest suite | 7/7 passed |
| Qt-free headless build/test | 1/1 passed |
| Navigation, defaults, native command states and method labels/order | Passed at all four test scales |
| Invalid/duplicate registration and extension navigation | Passed |
| Font repair on every screen/popup; valid preference regression | Passed |
| 1366×768, 1920×1080, 3840×2160 at 100/125/150/200% | Passed: 84 route/layout combinations per OS suite |
| Accessible controls, focus, image DPR, blue canvas and white preview text | Passed |
| Restored geometry, negative monitor origins, desktop gaps and unplugged monitor rectangles | Passed |
| Font/DPR change layout invalidation | Passed |
| Workflow lint and whitespace checks | Passed |
| Windows/Linux/macOS CI, deployed startup and three native packages | Pending this run |

Local test XML: `build/dev/foundation5-tests.xml`. Original clone screen
images: `build/foundation5/screens/scale-{1,1.25,1.5,2}/`, seven pages each.
CI is configured to upload its XML and these original UI screenshots in each
OS's test-results artifact. This is simulated DPI/resolution coverage, not
proof of physical mixed-monitor behavior on every platform. Manual Windows
10/11, macOS Intel/Apple Silicon and mixed-monitor hotplug acceptance remain
hardware checks; the macOS package is universal but CI executes on its host
architecture. Packaging baseline/signing limits remain in FOUNDATION1_REPORT.md.

## Oracle comparison and review limits

Used the already completed captures; no TORA instances/corpus were restarted.
Compared welcome/MAIN/input mode/empty input/solve menu/output format with
the saved screenshots indexed in TRACEABILITY.md. Labels, ordering, palette,
centered groups, headings and footer placement follow those observations.
The clone is a scalable native recreation, not a stretched screenshot.

The centered MAIN/SOLVE panels stay visible and currently activate their
popup by click/keyboard; exact legacy hover behavior belongs to the later
solve-screen module. SOLVE Menu is available on the empty input page solely
to review navigation, although the reference disables it before a grid exists.
No save prompt appears because this preview creates no problem document.
Welcome artwork/copyright is original clone text. These preview differences
are documented; this gate does not assert complete LP behavior parity.

No TORA screenshots/files/binaries/assets are committed or packaged. Original
clone screenshots contain only this implementation. Foundations 2/3/4 remain
unstarted and await the user's Foundation 5 approval.

## Laptop review

The app is open on desktop 2. Click **Linear Programming → Go to Input Screen
→ SOLVE Menu**. **View/Modify Input Data** and **MAIN Menu** return to earlier
screens. Save/edit/solve commands are intentionally disabled in this preview.
To launch again from the repository:

```sh
build/dev/src/desktop/ToraClone
```

For the registered format and M layouts:

```sh
build/dev/src/desktop/ToraClone --no-settings --screen lp/output-format
build/dev/src/desktop/ToraClone --no-settings --screen lp/m-value
```
