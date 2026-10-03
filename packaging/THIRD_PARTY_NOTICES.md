# Runtime dependencies

This application dynamically links Qt 6 Core, Gui and Widgets from The Qt
Company and contributors. The CI build uses the open-source Qt 6.8.3
distribution. Qt component license files supplied by the SDK are copied into
the development package when available. See the applicable LGPLv3/GPLv3
licenses and component exceptions at https://doc.qt.io/qt-6.8/licenses-used-in-qt.html.

Qt source for that build is available from
https://download.qt.io/archive/qt/6.8/6.8.3/single/.
An application built locally with another Qt version uses that SDK's version
and corresponding source/licenses instead. The packaging manifest records
the actual qmake version and dependency-file hashes.

Additional system libraries collected by platform deployment tools retain
their original licenses. These bootstrap packages are development artifacts;
the complete runtime notices and release review must accompany a public
production distribution. No TORA executable, reference file, screenshot,
icon or help text is included in this application's packages.
