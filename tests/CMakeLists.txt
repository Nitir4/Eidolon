add_executable(core_smoke core_smoke.cpp)
tora_target_defaults(core_smoke)
target_link_libraries(core_smoke PRIVATE tora::core)
add_test(NAME core.smoke COMMAND core_smoke)
set_tests_properties(core.smoke PROPERTIES TIMEOUT 15)

if(TORA_BUILD_DESKTOP)
    add_executable(desktop_font desktop_font.cpp)
    tora_target_defaults(desktop_font)
    target_include_directories(desktop_font PRIVATE "${PROJECT_SOURCE_DIR}/src/desktop")
    target_link_libraries(desktop_font PRIVATE Qt6::Widgets)
    add_test(NAME desktop.font COMMAND desktop_font)
    set_tests_properties(desktop.font PROPERTIES
        TIMEOUT 15
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )

    add_test(NAME desktop.smoke COMMAND ToraClone --smoke-test)
    set_tests_properties(desktop.smoke PROPERTIES
        TIMEOUT 15
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endif()
