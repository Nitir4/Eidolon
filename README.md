# Native TORA LP clone

Phase 0 is approved. Foundation 1 supplies the C++17/CMake skeleton, native
Qt 6 Widgets bootstrap, smoke tests and three-OS packaging pipeline. LP
algorithms and the TORA screen workflow remain behind their individual gates.
One source tree and the `main` branch serve Windows, macOS and Linux.

See [current status](docs/STATUS.json), [Foundation 1 report](docs/FOUNDATION1_REPORT.md)
and [build instructions](docs/BUILD.md). With CMake 3.24+, Ninja, a C++17 compiler
and Qt 6.8+ Widgets installed:

```sh
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

Set `CMAKE_PREFIX_PATH` to your Qt SDK if needed. The `headless` preset builds
without Qt; `ci` builds Release artifacts. The [workflow](.github/workflows/build.yml)
builds, tests, packages and checks deployed startup on all three OSes.

The approved analysis is in [the Phase 0 report](docs/PHASE0_REPORT.md),
[analysis index](docs/tora-analysis/README.md),
[architecture](docs/ARCHITECTURE.md) and [open questions](docs/OPEN_QUESTIONS.md).

The observation scripts in tools/ are reference tooling only. TORA binaries,
Wine runtime, screenshots and saved reference files are private/gitignored.
Do not publish them or include them in release artifacts without the user's
explicit approval.

Run `python3 tools/audit_phase0.py` to validate the saved local evidence without
launching TORA or repeating any completed cases. It requires the private
reference corpus; its pass is an evidence audit, not Gate 0 approval.
