# Native deployment and release readiness

The captured LP implementation remains complete. This follow-up fixes Linux
Wayland deployment, removes a stale welcome footer, and supplies versioned Qt
legal documents. Final CI and package audit are pending; previous accepted
installer delivery is preserved in `docs/STATUS.json`.

## Verified locally

- Full CTest suite: 74/74, no failures (90.55 seconds).
- AppImage explicitly starts on the laptop's existing Wayland compositor with
  Qt SDK paths removed. Previously this explicit backend failed with exit127.
- Six actual fullscreen Widgets views at1920×1080 on eDP-1: welcome, MAIN menu,
  populated input, All-slack, graph and final/sensitivity. Route-checked capture
  tool asserts bounds, buttons and preserved model; images were inspected.
- Real Linux Qt print dialog opened/cancelled, then its PDF controls selected
  an owned temporary file. Independent PDF reader confirms original normal
  print's page count and139 tokens. No physical print job was sent.
- Qt6.10.3 upstream legal bundle contains130 legal/attribution files with
  source URLs and SHA-256. Tests reject corruption and unsafe paths. A different
  local SDK6.11.2 does not receive mislabeled6.10.3 notices.

## Deployment design

Linux stages the SDK's available Wayland platform plugins and client shell,
decoration and graphics integrations. `linuxdeploy` scans their ELF dependencies
and fixes runtime paths. X11 and offscreen plugins remain supported. The single
three-OS workflow installs Qt's Wayland archive on Linux and checks the shipped
AppImage on private Weston and Xvfb servers. Those protocol checks supplement
rather than replace physical monitor acceptance.

The Qt legal bundle records upstream component information, including entries
that may not be enabled in these binaries. It is not a build SBOM or legal
approval. Application licensing, relevant vendor redistributables and
corresponding-source obligations require an owner release review.

## Remaining resources

One physical1920×1080 display is connected; no configured printer appears in
Qt. Physical mixed-DPI handoff/hotplug, other physical resolutions, physical
printing and Windows/macOS panel review remain unverified. No repository
Actions signing secrets or targeted signing variables were configured when
checked; trusted Windows signing and macOS signing/notarization require those
credentials. No new OR module or phase was started.

Private evidence: `build/release-readiness/`; raw images and saved problems
remain excluded from source and release packages.
