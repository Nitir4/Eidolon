add_executable(core_smoke core_smoke.cpp)
tora_target_defaults(core_smoke)
target_link_libraries(core_smoke PRIVATE tora::core)
add_test(NAME core.smoke COMMAND core_smoke)
set_tests_properties(core.smoke PROPERTIES TIMEOUT 15)

if(TORA_BUILD_DESKTOP)
    add_test(NAME desktop.smoke COMMAND ToraClone --smoke-test)
    set_tests_properties(desktop.smoke PROPERTIES
        TIMEOUT 15
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endif()
