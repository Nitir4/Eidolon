add_library(or_core STATIC tableau.cpp number_formatter.cpp)
add_library(tora::core ALIAS or_core)
tora_target_defaults(or_core)
target_compile_features(or_core PUBLIC cxx_std_17)
target_include_directories(or_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/generated/include"
)
# Keep arithmetic operation order identical across the three compilers.
if(MSVC)
    target_compile_options(or_core PRIVATE /fp:strict)
else()
    target_compile_options(or_core PRIVATE -ffp-contract=off -fno-fast-math)
endif()
