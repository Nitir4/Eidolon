add_executable(core_smoke core_smoke.cpp)
tora_target_defaults(core_smoke)
target_link_libraries(core_smoke PRIVATE tora::core)
add_test(NAME core.smoke COMMAND core_smoke)
set_tests_properties(core.smoke PROPERTIES TIMEOUT 15)

add_executable(all_slack_safety all_slack_safety.cpp)
tora_target_defaults(all_slack_safety)
target_link_libraries(all_slack_safety PRIVATE tora::lp)
add_test(NAME lp.all-slack.safety COMMAND all_slack_safety)
set_tests_properties(lp.all-slack.safety PROPERTIES TIMEOUT 15)

add_executable(simplex_probe simplex_probe.cpp)
tora_target_defaults(simplex_probe)
target_link_libraries(simplex_probe PRIVATE tora::lp)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(NAME lp.all-slack.goldens COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/verify_all_slack.py" "$<TARGET_FILE:simplex_probe>"
    "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/all-slack.json" "${CMAKE_CURRENT_BINARY_DIR}/all-slack-comparison.tsv")
set_tests_properties(lp.all-slack.goldens PROPERTIES TIMEOUT 30)

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

add_executable(lp_model lp_model.cpp)
tora_target_defaults(lp_model)
target_link_libraries(lp_model PRIVATE tora::lp)
add_test(NAME lp.model COMMAND lp_model "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/lp-models.txt")
set_tests_properties(lp.model PROPERTIES TIMEOUT 15)

add_executable(lp_validation lp_validation.cpp)
tora_target_defaults(lp_validation)
target_link_libraries(lp_validation PRIVATE tora::lp)
add_test(NAME lp.validation COMMAND lp_validation)
set_tests_properties(lp.validation PROPERTIES TIMEOUT 15)

add_executable(codec_roundtrip codec_roundtrip.cpp)
tora_target_defaults(codec_roundtrip)
target_link_libraries(codec_roundtrip PRIVATE tora::compat)
add_test(NAME compat.roundtrip COMMAND codec_roundtrip
    "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/lp-models.txt"
    "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/tora-file-settings.tsv"
    "${CMAKE_CURRENT_BINARY_DIR}/codec-goldens")
set_tests_properties(compat.roundtrip PROPERTIES TIMEOUT 15 FIXTURES_SETUP codec_golden_files)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(NAME compat.hashes COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/verify_codec_hashes.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/tora-file-settings.tsv"
    "${CMAKE_CURRENT_BINARY_DIR}/codec-goldens")
set_tests_properties(compat.hashes PROPERTIES FIXTURES_REQUIRED codec_golden_files TIMEOUT 15)
add_executable(codec_errors codec_errors.cpp)
tora_target_defaults(codec_errors)
target_link_libraries(codec_errors PRIVATE tora::compat)
add_test(NAME compat.errors COMMAND codec_errors)
set_tests_properties(compat.errors PROPERTIES TIMEOUT 15)
add_executable(file_store file_store.cpp)
tora_target_defaults(file_store)
target_link_libraries(file_store PRIVATE tora::compat)
add_test(NAME compat.files COMMAND file_store "${CMAKE_CURRENT_BINARY_DIR}/file-store-check")
set_tests_properties(compat.files PROPERTIES TIMEOUT 15)

add_executable(codec_probe "${PROJECT_SOURCE_DIR}/tools/codec_probe.cpp")
tora_target_defaults(codec_probe)
target_link_libraries(codec_probe PRIVATE tora::compat)

add_executable(codec_grid codec_grid.cpp)
tora_target_defaults(codec_grid)
target_link_libraries(codec_grid PRIVATE tora::compat)
add_test(NAME compat.grid COMMAND codec_grid "${CMAKE_CURRENT_BINARY_DIR}/grid-file-check")
set_tests_properties(compat.grid PROPERTIES TIMEOUT 15)

if(TORA_BUILD_DESKTOP)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Test)
    qt_add_executable(desktop_simplex_test desktop_simplex.cpp)
    set_target_properties(desktop_simplex_test PROPERTIES AUTOMOC ON)
    tora_target_defaults(desktop_simplex_test)
    target_link_libraries(desktop_simplex_test PRIVATE desktop_shell Qt6::Test)
    target_compile_definitions(desktop_simplex_test PRIVATE TORA_CODEC_GOLDENS="${CMAKE_CURRENT_BINARY_DIR}/codec-goldens" TORA_SIMPLEX_GOLDENS="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/all-slack.json")
    foreach(scale IN ITEMS 1 1.25 1.5 2)
        add_test(NAME desktop.simplex.scale-${scale} COMMAND desktop_simplex_test
            -o "${CMAKE_CURRENT_BINARY_DIR}/desktop-simplex-scale-${scale}.xml,junitxml" -o -,txt)
        set_tests_properties(desktop.simplex.scale-${scale} PROPERTIES TIMEOUT 45 FIXTURES_REQUIRED codec_golden_files
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=${scale};QT_QPA_PLATFORMTHEME=;QT_STYLE_OVERRIDE=Fusion")
    endforeach()
    qt_add_executable(desktop_input_test desktop_input.cpp)
    set_target_properties(desktop_input_test PROPERTIES AUTOMOC ON)
    tora_target_defaults(desktop_input_test)
    target_link_libraries(desktop_input_test PRIVATE desktop_shell tora::compat Qt6::Test)
    target_compile_definitions(desktop_input_test PRIVATE TORA_CODEC_GOLDENS="${CMAKE_CURRENT_BINARY_DIR}/codec-goldens")
    target_compile_definitions(desktop_input_test PRIVATE TORA_INPUT_EDITS="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/tora-input-edits.tsv")
    foreach(scale IN ITEMS 1 1.25 1.5 2)
        add_test(NAME desktop.input.scale-${scale} COMMAND desktop_input_test
            -o "${CMAKE_CURRENT_BINARY_DIR}/desktop-input-scale-${scale}.xml,junitxml" -o -,txt)
        set_tests_properties(desktop.input.scale-${scale} PROPERTIES TIMEOUT 45 FIXTURES_REQUIRED codec_golden_files
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=${scale};QT_QPA_PLATFORMTHEME=;QT_STYLE_OVERRIDE=Fusion")
    endforeach()
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

add_executable(m_method_probe simplex_probe.cpp)
tora_target_defaults(m_method_probe)
target_compile_definitions(m_method_probe PRIVATE TORA_M_METHOD_PROBE)
target_link_libraries(m_method_probe PRIVATE tora::lp)
add_test(NAME lp.m-method.goldens COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/verify_all_slack.py" "$<TARGET_FILE:m_method_probe>"
    "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/m-method.json" "${CMAKE_CURRENT_BINARY_DIR}/m-method-comparison.tsv")
set_tests_properties(lp.m-method.goldens PROPERTIES TIMEOUT 30)

if(TORA_BUILD_DESKTOP)
    qt_add_executable(desktop_m_method_test desktop_m_method.cpp)
    set_target_properties(desktop_m_method_test PROPERTIES AUTOMOC ON)
    tora_target_defaults(desktop_m_method_test)
    target_link_libraries(desktop_m_method_test PRIVATE desktop_shell Qt6::Test)
    target_compile_definitions(desktop_m_method_test PRIVATE TORA_CODEC_GOLDENS="${CMAKE_CURRENT_BINARY_DIR}/codec-goldens" TORA_M_GOLDENS="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/m-method.json")
    foreach(scale IN ITEMS 1 1.25 1.5 2)
        add_test(NAME desktop.m-method.scale-${scale} COMMAND desktop_m_method_test
            -o "${CMAKE_CURRENT_BINARY_DIR}/desktop-m-method-scale-${scale}.xml,junitxml" -o -,txt)
        set_tests_properties(desktop.m-method.scale-${scale} PROPERTIES TIMEOUT 45 FIXTURES_REQUIRED codec_golden_files
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=${scale};QT_QPA_PLATFORMTHEME=;QT_STYLE_OVERRIDE=Fusion")
    endforeach()
endif()

add_executable(m_method_safety m_method_safety.cpp)
tora_target_defaults(m_method_safety)
target_link_libraries(m_method_safety PRIVATE tora::lp)
add_test(NAME lp.m-method.safety COMMAND m_method_safety)
set_tests_properties(lp.m-method.safety PROPERTIES TIMEOUT 15)

add_executable(two_phase_probe simplex_probe.cpp)
tora_target_defaults(two_phase_probe)
target_compile_definitions(two_phase_probe PRIVATE TORA_TWO_PHASE_PROBE)
target_link_libraries(two_phase_probe PRIVATE tora::lp)
add_test(NAME lp.two-phase.goldens COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/verify_all_slack.py" "$<TARGET_FILE:two_phase_probe>"
    "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/two-phase.json" "${CMAKE_CURRENT_BINARY_DIR}/two-phase-comparison.tsv")
set_tests_properties(lp.two-phase.goldens PROPERTIES TIMEOUT 30)

add_executable(two_phase_safety two_phase_safety.cpp)
tora_target_defaults(two_phase_safety)
target_link_libraries(two_phase_safety PRIVATE tora::lp)
add_test(NAME lp.two-phase.safety COMMAND two_phase_safety)
set_tests_properties(lp.two-phase.safety PROPERTIES TIMEOUT 15)

if(TORA_BUILD_DESKTOP)
    qt_add_executable(desktop_two_phase_test desktop_two_phase.cpp)
    set_target_properties(desktop_two_phase_test PROPERTIES AUTOMOC ON)
    tora_target_defaults(desktop_two_phase_test)
    target_link_libraries(desktop_two_phase_test PRIVATE desktop_shell Qt6::Test)
    target_compile_definitions(desktop_two_phase_test PRIVATE TORA_CODEC_GOLDENS="${CMAKE_CURRENT_BINARY_DIR}/codec-goldens" TORA_TWO_PHASE_GOLDENS="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/two-phase.json")
    foreach(scale IN ITEMS 1 1.25 1.5 2)
        add_test(NAME desktop.two-phase.scale-${scale} COMMAND desktop_two_phase_test
            -o "${CMAKE_CURRENT_BINARY_DIR}/desktop-two-phase-scale-${scale}.xml,junitxml" -o -,txt)
        set_tests_properties(desktop.two-phase.scale-${scale} PROPERTIES TIMEOUT 45 FIXTURES_REQUIRED codec_golden_files
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=${scale};QT_QPA_PLATFORMTHEME=;QT_STYLE_OVERRIDE=Fusion")
    endforeach()
endif()

add_executable(bounded_probe simplex_probe.cpp)
tora_target_defaults(bounded_probe)
target_compile_definitions(bounded_probe PRIVATE TORA_BOUNDED_PROBE)
target_link_libraries(bounded_probe PRIVATE tora::lp)
add_test(NAME lp.bounded.goldens COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/verify_all_slack.py" "$<TARGET_FILE:bounded_probe>"
    "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/bounded.json" "${CMAKE_CURRENT_BINARY_DIR}/bounded-comparison.tsv")
set_tests_properties(lp.bounded.goldens PROPERTIES TIMEOUT 30)

add_executable(bounded_safety bounded_safety.cpp)
tora_target_defaults(bounded_safety)
target_link_libraries(bounded_safety PRIVATE tora::lp)
add_test(NAME lp.bounded.safety COMMAND bounded_safety)
set_tests_properties(lp.bounded.safety PROPERTIES TIMEOUT 15)

if(TORA_BUILD_DESKTOP)
    qt_add_executable(desktop_bounded_test desktop_bounded.cpp)
    set_target_properties(desktop_bounded_test PROPERTIES AUTOMOC ON)
    tora_target_defaults(desktop_bounded_test)
    target_link_libraries(desktop_bounded_test PRIVATE desktop_shell Qt6::Test)
    target_compile_definitions(desktop_bounded_test PRIVATE TORA_CODEC_GOLDENS="${CMAKE_CURRENT_BINARY_DIR}/codec-goldens" TORA_BOUNDED_GOLDENS="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/bounded.json")
    foreach(scale IN ITEMS 1 1.25 1.5 2)
        add_test(NAME desktop.bounded.scale-${scale} COMMAND desktop_bounded_test
            -o "${CMAKE_CURRENT_BINARY_DIR}/desktop-bounded-scale-${scale}.xml,junitxml" -o -,txt)
        set_tests_properties(desktop.bounded.scale-${scale} PROPERTIES TIMEOUT 45 FIXTURES_REQUIRED codec_golden_files
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=${scale};QT_QPA_PLATFORMTHEME=;QT_STYLE_OVERRIDE=Fusion")
    endforeach()
endif()
