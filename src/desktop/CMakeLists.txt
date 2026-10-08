qt_standard_project_setup(REQUIRES 6.8)
qt_add_library(desktop_shell STATIC application_shell.cpp application_shell.hpp lp_screens.cpp lp_input_controller.cpp lp_input_controller.hpp lp_simplex_controller.cpp lp_simplex_controller.hpp)
tora_target_defaults(desktop_shell)
target_include_directories(desktop_shell PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(desktop_shell PUBLIC Qt6::Widgets Qt6::PrintSupport tora::compat)
qt_add_executable(ToraClone main.cpp)
tora_target_defaults(ToraClone)
target_link_libraries(ToraClone PRIVATE tora::core desktop_shell)
set_target_properties(ToraClone PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "TORA Clone"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.toraclone.desktop"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)
install(TARGETS ToraClone
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
