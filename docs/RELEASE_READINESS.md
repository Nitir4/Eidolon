# Native deployment and release readiness

The remaining implementation and deployment checks for the captured LP profile
are complete. Source `19463eba2ccb3cd094843afbc81cfa77d99d0908` passed [the three-OS CI run](https://github.com/Nitir4/TORA-Clone/actions/runs/38062462111), and the
six downloaded artifacts passed an independent audit. Other OR modules were
not started. Previous accepted deliveries and failed-run evidence are preserved.

## Results

| Check | Observed result |
|---|---|
| Windows, Linux and macOS CTest | 74/74 each; 222 overall, zero failures or skips |
| LP iteration/final/sensitivity comparisons | All 31,654 recorded fields per OS match the established capture rules |
| Printing | 25 observed states, 23 PDFs and 4,259 text tokens/page counts match per OS |
| Synthetic display changes | 12 scenarios and 48 populated captures per OS; all scale/resolution checks pass |
| Shipped Linux AppImage | Offscreen packaging startup plus Wayland and X11 protocol checks pass in CI |
| Final Linux package on this laptop | Explicit Wayland startup passes with SDK paths removed; all 281 extracted manifest hashes match |
| Windows installer | 163 payload hashes; install/reinstall, clean-runtime startup, Unicode/spaced paths, Start menu/registry, safe removal and student-file preservation pass |
| macOS packaging | Universal app/framework architecture checks and startup pass; DMG produced |
| Qt notices | 130 exact upstream legal/attribution files plus provenance index included and hash-verified for Qt 6.10.3 on all OSes |

All three package hashes match their sidecars. Every portable Windows payload
hash was checked, as were the installer hash and native verification metadata.
No raw TORA screenshot, saved reference file, icon, help text or executable is
included in these application packages.

## Actual desktop checks

On the laptop's existing Wayland compositor, six source-build Widgets views
were captured fullscreen at 1920×1080: welcome, MAIN menu, populated input,
All-slack, graphical and final/sensitivity. The route-checked manual harness
asserts display bounds and button sizes and preserves encoded model bytes.
The real Linux Qt print dialog cancelled, then selected a private PDF through
its own controls. The independent reader matched the original normal print's
page count and 139 tokens. No physical print job was sent.

Those manual source checks used the installed Qt 6.11.2 SDK. The final CI
package uses Qt 6.10.3; its actual laptop Wayland startup and interactive LP
input-mode/notation view were separately checked. Its captured window was
956×1046 because Hyprland tiled it; the text was readable and the window closed
normally. This capture establishes deployed rendering, not a populated-model
or physical mixed-monitor check. No display settings or existing TORA instances
were changed. The stale welcome footer was removed with regression coverage.

## Deployment corrections

The first follow-up run, [38062038002](https://github.com/Nitir4/TORA-Clone/actions/runs/38062038002),
exposed Windows checkout conversion of original legal text and a missing
Linux Qt SVG dependency used by a Wayland decoration plugin. Git attributes
now preserve upstream bytes, and Linux SDK installation includes `qtsvg` with
`qtwayland`. Isolated `core.autocrlf=true` checkouts reproduced 122 altered
hashes before the fix and zero altered hashes across 130 files afterward.
No numerical tolerances or golden data were changed.

Linux stages available Wayland platform plugins and client shell, decoration
and graphics integrations. `linuxdeploy` scans their ELF dependencies and fixes
runtime paths. X11 and offscreen support remains. The single OS-matrix workflow
checks the deployed AppImage on private Weston and Xvfb servers. These protocol
checks supplement physical acceptance. No compositor is bundled in the app.

The versioned Qt legal index records original URLs and SHA-256. Sourcewide
metadata may mention components not enabled in the deployed binaries; this
index is not a build SBOM or production legal approval.

## Packages and evidence

Final packages: `dist/release-readiness/final/TORA-Clone-<os>/`. The Windows
artifact contains both the portable ZIP and per-user setup EXE; macOS contains
the universal DMG, and Linux contains the AppImage. For downloads outside this
workspace, use the linked successful CI run's artifacts.

Private audit evidence is under `build/release-readiness/`: `ci-verified.json`,
`packages-verified.json`, `linux-payload-verified.json`, `final-live-wayland/`,
`native-desktop/`, `native-pdf-audit/` and `final-deployed-view/`. Raw captures
and test problem files remain excluded from source and application packages.
The local full suite passed 74/74 in 90.55 seconds; focused packaging checks
passed after the deployment corrections. Workflow lint and source whitespace
checks passed. Original upstream legal whitespace is preserved byte-for-byte.

## Resource-dependent acceptance

One physical 1920×1080 display was connected and Qt listed no configured
printer. Physical mixed-DPI handoff/hotplug, other physical resolutions,
physical printing and Windows/macOS native panel review remain unverified.
No repository Actions signing secrets or targeted signing variables were
configured when checked; trusted Windows signing and macOS notarization need
developer credentials. The owner must also decide application licensing and
review applicable vendor redistribution/corresponding-source requirements
before a production release. These development artifacts are unsigned.
Further outside-profile numerical compatibility remains governed by the
recorded questions in `OPEN_QUESTIONS.md`.
