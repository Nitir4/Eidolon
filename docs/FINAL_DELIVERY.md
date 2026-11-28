# Final package delivery

Delivery preparation started on 11 November 2026. The previous branch tip has
an author/committer date of 25 November 2026; new commits are deliberately dated
26 November onward at the user's request. These Git dates do not represent the
actual build dates.

The source includes the complete implemented application plus transportation
balancing, initial-allocation explanations, basic-zero/MODI explanations, LP
layout/print fixes and the Linux AppRun correction. The private question-bank
ZIP is excluded from source control.

The current native development suite passed 188/188 on this host in 20.10
seconds. Fresh Linux Release, Windows x64 and universal macOS builds are being
prepared from this source. Package verification results will be recorded here
after they are observed.

Delivery names will identify the target operating system. Each ZIP contains
only its application package and required runtime/legal files, with no source,
question bank or separate practice/test bundle. Windows/macOS cross-compilation
does not establish native acceptance; those packages require testing on their
target machines. macOS requires 13.3 or later; Windows targets Windows 10 1809
or later, x64. Linux targets x86_64.

GitHub Actions remain disabled. The authorized pushes and branch integration
will not run workflows. No release, signing or notarization is implied.
