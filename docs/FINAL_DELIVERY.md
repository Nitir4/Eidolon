# Final package delivery

## Current delivery — 13 November 2026

The main screen now has a small, muted `GitHub ↗` link at its lower-right edge.
It opens `https://github.com/Nitir4/Eidolon` in the default browser. It belongs
only to the main-screen page and is hidden when navigating into a module.
No menu item or Help page was added. Keyboard link activation is enabled.

Implementation: `src/desktop/application_shell.cpp`. The existing shell
navigation regression now intercepts the desktop URL handler, verifies the
clicked destination and checks that the link hides after entering LP.
Shell tests passed at 100%, 125%, 150% and 200% scaling. Native and packaged
main-screen captures were visually inspected.

The native development suite passed 188/188 in 31.99 seconds; the isolated
Ubuntu 24.04 / Qt 6.10.3 Release suite passed 188/188 in 23.78 seconds.
Windows x64 and universal macOS Release cross-builds succeeded; package checks
resolved 14 Windows and 12 macOS executable/library dependency records and
verified both architectures in every bundled Mach-O. No new Windows/macOS
native or Wine GUI acceptance is claimed for this revision.

All packages were built from clean source
`059af221ab26466854d7bd861f48473036606feb`. All 1089 tracked input files were
fingerprinted and unchanged throughout builds and packaging. Subsequent
delivery changes affect documentation only. The source commit is intentionally
dated 30 November 2026, after the previous 29 November branch tip; the delivery
record commit is dated 1 December, per the owner's requested Git-date ordering.
The actual build date is 13 November.

Fresh package-only ZIPs and SHA256SUMS are in `dist/final-delivery-20261113/`
and `/home/nitir505/easy access/Eidolon packages/`:

- `Eidolon-Windows-x64.zip`: Windows 10 1809+ x64; extract and run `Eidolon.exe`.
- `Eidolon-macOS-Universal.zip`: macOS 13.3+, Intel or Apple Silicon; open `Eidolon.app`.
- `Eidolon-Linux-x86_64.zip`: Linux x86_64; launch the extracted executable AppImage.

ZIP CRCs, safe member paths, manifest hashes and executable permissions were
verified. No source, examples, question bank or external test scripts are
included. Copies in easy access match; previous ZIPs are retained in its
`previous-20261111/` subdirectory. Windows/macOS remain unsigned/unnotarized
portable test builds requiring target-machine acceptance.

The Linux package passed ten offscreen screen startups, actual Wayland and
dedicated X11 startup with SDK paths removed, all 281 extracted payload hashes,
and 92 ELF version checks (maximum required GLIBC 2.38). The stable local
`eidolon` launcher now selects the new Release AppImage and passes startup;
its prior launcher and release are preserved. Existing user windows were not
closed or restarted. Reopen Eidolon to load the new version.

The new test prerelease is
[v0.1.0-test.20261113](https://github.com/Nitir4/Eidolon/releases/tag/v0.1.0-test.20261113).
The earlier prerelease is retained. Source integration uses matching `main`
and `future-scope` refs without rewriting history. Actions remain disabled.
Build logs, screenshots, source fingerprints, installation details and the
final remote/release receipt are in `build/github-link-20261113/`; packaging
logs/manifests are in
`build/platform-followup/linux-baseline/final-delivery-20261113/`.

## Previous delivery — 11 November 2026

Delivery preparation started on 11 November 2026. The previous branch tip has
an author/committer date of 25 November 2026; new commits are deliberately dated
26 November onward at the user's request. These Git dates do not represent the
actual build dates.

The source includes the complete implemented application plus transportation
balancing, initial-allocation explanations, basic-zero/MODI explanations, LP
layout/print fixes and the Linux AppRun correction. The private question-bank
ZIP is excluded from source control.

The native development suite passed 188/188 on this host in 20.10 seconds.
The fresh isolated Ubuntu 24.04 / Qt 6.10.3 Linux Release suite also passed
188/188 in 22.10 seconds. All three packages were built from clean source
commit `a9de9b7afa74c416753f6a79a18d914dae3f16c9`; the subsequent delivery
record changes documentation only. All 1089 tracked input files were
fingerprinted and remained unchanged through the builds and packaging.

Delivery names identify the target operating system. Each ZIP contains
only its application package and required runtime/legal files, with no source,
question bank or separate practice/test bundle. Windows/macOS cross-compilation
does not establish native acceptance; those packages require testing on their
target machines. macOS requires 13.3 or later; Windows targets Windows 10 1809
or later, x64. Linux targets x86_64.

GitHub Actions remain disabled. The authorized pushes and branch integration
will not run workflows. No release, signing or notarization is implied.

## Delivered files

Files are in `dist/final-delivery-20261111/`, with matching copies under
`/home/nitir505/easy access/Eidolon packages/`. SHA256SUMS accompanies them.

| File | Target | Launch after extracting |
| --- | --- | --- |
| `Eidolon-Windows-x64.zip` | Windows 10 1809+ x64 | Run `Eidolon.exe`; keep the DLLs and plugin folders beside it |
| `Eidolon-macOS-Universal.zip` | macOS 13.3+, Intel or Apple Silicon | Open `Eidolon.app`; retain accompanying legal files |
| `Eidolon-Linux-x86_64.zip` | Linux x86_64 | Make the extracted AppImage executable if necessary, then launch it |

The Linux archive contains one AppImage; dependencies and legal notices are
inside it. The Windows and macOS archives contain only the application,
runtime dependencies, legal notices and package metadata. There are no
question-bank files, source trees, practice sheets or external test scripts.

## Checks actually performed

- Windows Release cross-build succeeded. The packager checked x64 PE files
  and resolved 14 executable/library dependency records. The extracted actual
  package passed ten screen startups under Wine 11.20. Six Windows numerical
  regression executables also passed under Wine, including transportation
  allocation/degeneracy, safety, final summary, Assignment, Games and sensitivity.
- macOS Release cross-build succeeded. The packager checked both arm64 and
  x86_64 in every bundled Mach-O, 12 executable/library dependency records,
  bundled frameworks and the application-relative runtime search path.
- Linux Release packaging and ten offscreen module startups succeeded.
  Packaged startup passed on this host's existing Wayland compositor and a
  dedicated Xvfb X11 server with Qt/loader SDK paths removed. All 281 extracted
  payload hashes match. The 92 ELF files require at most GLIBC 2.38; this
  version audit alone does not certify all Linux distributions.
- All ZIP CRCs, safe member paths, contained manifest file hashes and executable
  ZIP permissions were checked; copies in easy access match the source ZIPs.

## Remaining acceptance and failed attempts

No native Windows or macOS host was available. These are unsigned/unnotarized
portable test packages, not a newly certified three-OS release. Target-machine
GUI, file access, printing and OS security acceptance remain for testers.
Physical printers and mixed-DPI hardware are still unverified.

The first Wine startup check mistakenly used the removed `welcome` route;
the valid MAIN and nine module routes all passed after correcting the check.
An additional Wine Qt transportation GUI-test run timed out after 90 seconds;
diagnostic attempts exposed Qt platform-plugin discovery outside the packaged
application directory and later Wine-session stalls. They are not passing GUI
tests. Task-owned Wine diagnostics were stopped. The same transportation GUI
coverage passes in both full Linux suites. No application source was changed
to accommodate Wine.

Initial sandboxed namespace launches failed before compilation. After approval
and the permission change, cross-builds ran in the existing isolated userspace.
Duplicate builds resumed around the interruption were stopped, and final
sequential build checks preceded packaging. Logs and manifests are retained
under `build/final-delivery-20261111/` and
`build/platform-followup/linux-baseline/final-delivery-20261111/`.

## Source integration

The final source and this delivery record are to be pushed to `future-scope`
and fast-forwarded into `main`. The older validation branch is already an
ancestor of this source; no history rewrite or branch deletion is needed.
The exact final remote hashes and disabled-Actions check are recorded in the
local delivery receipt after the push.
