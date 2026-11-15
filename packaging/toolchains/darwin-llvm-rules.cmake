# CMake cannot query sw_vers on Linux; set these after generic platform setup.
set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,-rpath,")
set(CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG "-Wl,-rpath,")
# Each architecture needs an indexed archive; ar over universal .o files is insufficient.
set(CMAKE_CXX_ARCHIVE_CREATE "/usr/bin/llvm-libtool-darwin-18 -static -o <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_CXX_ARCHIVE_APPEND "/usr/bin/llvm-libtool-darwin-18 -static -o <TARGET> <LINK_FLAGS> <OBJECTS> <TARGET>")
set(CMAKE_CXX_ARCHIVE_FINISH "")
