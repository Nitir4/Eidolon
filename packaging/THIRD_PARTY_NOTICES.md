# Runtime dependencies and upstream legal documents

The application dynamically links Qt Core, Gui, Widgets and PrintSupport.
Deployment also brings platform-specific Qt dependencies/plugins, including
Network/DBus and the Linux Wayland client where needed. The manifest records
actual SDK version, deployed file hashes and the upstream Qt legal bundle path.

The pinned CI SDK is Qt 6.10.3. `licenses/Qt-6.10.3/` contains upstream LICENSES
texts and third-party `qt_attribution.json` metadata from official qtbase and
qtwayland source tags. `SOURCES.json` lists each original URL and SHA-256.
The bundle includes upstream entries for components not necessarily enabled
in these binaries; it is not a build SBOM. Including a license text does not
choose a license for the application or imply that every source component is
shipped. No Qt Wayland compositor is required by the application.

Qt source for this SDK is available at
https://download.qt.io/archive/qt/6.10/6.10.3/single/.
The licensing/component overview is
https://doc.qt.io/qt-6.10/licenses-used-in-qt.html.
Locally built packages with other SDK versions preserve supplied SDK license
files and do not mislabel their attributions as 6.10.3.

Linux deployment also copies available distribution copyright documents to
`usr/share/doc/`. Additional system libraries and Windows compiler/graphics
runtimes retain their original licenses. The Qt bundle and file inventory do
not establish complete corresponding-source obligations or redistribution
rights for every vendor runtime. These remain development artifacts until the
owner reviews the exact dependency inventory and applicable distribution terms.

No TORA executable, saved reference file, screenshot, icon or help text is
included. The application's own license has not been selected by its owner.
