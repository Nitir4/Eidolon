add_library(or_lp STATIC problem.cpp validation.cpp primal_simplex.cpp graphical.cpp)
add_library(tora::lp ALIAS or_lp)
tora_target_defaults(or_lp)
target_compile_features(or_lp PUBLIC cxx_std_17)
target_include_directories(or_lp PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(or_lp PUBLIC tora::core)
# Solver reconstruction has capture-visible binary64 residuals. Apply the
# core's operation-order policy to LP arithmetic too (private flags do not
# propagate through target_link_libraries).
if(MSVC)
    target_compile_options(or_lp PRIVATE /fp:strict)
else()
    target_compile_options(or_lp PRIVATE -ffp-contract=off -fno-fast-math)
endif()
