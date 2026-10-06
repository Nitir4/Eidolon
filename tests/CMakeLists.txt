add_executable(core_smoke core_smoke.cpp)
tora_target_defaults(core_smoke)
target_link_libraries(core_smoke PRIVATE tora::core)
add_test(NAME core.smoke COMMAND core_smoke)
set_tests_properties(core.smoke PROPERTIES TIMEOUT 15)

add_executable(core_numerics core_numerics.cpp)
tora_target_defaults(core_numerics)
target_link_libraries(core_numerics PRIVATE tora::core)
add_test(NAME core.numerics COMMAND core_numerics)
set_tests_properties(core.numerics PROPERTIES TIMEOUT 15)

add_executable(core_formatting core_formatting.cpp)
tora_target_defaults(core_formatting)
target_link_libraries(core_formatting PRIVATE tora::core)
add_test(NAME core.formatting COMMAND core_formatting "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/tora-number-format.tsv")
set_tests_properties(core.formatting PROPERTIES TIMEOUT 15)

if(TORA_BUILD_DESKTOP)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Test)
    qt_add_executable(desktop_shell_test desktop_shell.cpp)
    set_target_properties(desktop_shell_test PROPERTIES AUTOMOC ON)
    tora_target_defaults(desktop_shell_test)
    target_link_libraries(desktop_shell_test PRIVATE desktop_shell Qt6::Test)
    foreach(scale IN ITEMS 1 1.25 1.5 2)
        add_test(NAME desktop.shell.scale-${scale} COMMAND desktop_shell_test)
        set_tests_properties(desktop.shell.scale-${scale} PROPERTIES
            TIMEOUT 45
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=${scale};QT_QPA_PLATFORMTHEME=;QT_STYLE_OVERRIDE=Fusion"
        )
    endforeach()
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
