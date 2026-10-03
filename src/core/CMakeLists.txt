# Target boundary only. Numerical implementation belongs to Foundation 2.
add_library(or_core INTERFACE)
add_library(tora::core ALIAS or_core)
target_compile_features(or_core INTERFACE cxx_std_17)
target_include_directories(or_core INTERFACE
    "${PROJECT_BINARY_DIR}/generated/include"
)
