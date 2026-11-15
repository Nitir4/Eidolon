cmake_minimum_required(VERSION 3.24)

if(CMAKE_HOST_APPLE)
    # Floating-point std::to_chars is supplied by libc++ starting in macOS 13.3.
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.3" CACHE STRING "Minimum macOS version")
    if(CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "13.3")
        message(FATAL_ERROR "Eidolon requires macOS 13.3 or newer (floating-point std::to_chars)")
    endif()
endif()

project(Eidolon VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(GNUInstallDirs)
include(CTest)

option(TORA_BUILD_DESKTOP "Build the native Qt Widgets executable" ON)

configure_file(
    cmake/build_info.hpp.in
    generated/include/tora/build_info.hpp
    @ONLY
)

function(tora_target_defaults target)
    target_compile_features(${target} PRIVATE cxx_std_17)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /Zc:__cplusplus)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_subdirectory(src/core)
add_subdirectory(src/equations)
add_subdirectory(src/queuing)
add_subdirectory(src/lp)
add_subdirectory(src/transportation)
add_subdirectory(src/assignment)
add_subdirectory(src/games)
add_subdirectory(src/integer)
add_subdirectory(src/networks)
add_subdirectory(src/planning)
add_subdirectory(src/compat)

if(TORA_BUILD_DESKTOP)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Widgets PrintSupport)
    add_subdirectory(src/desktop)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
