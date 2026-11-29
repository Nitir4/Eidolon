# Final package delivery

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
