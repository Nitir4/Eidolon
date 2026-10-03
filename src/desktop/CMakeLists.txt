qt_standard_project_setup(REQUIRES 6.8)
qt_add_executable(ToraClone main.cpp)
tora_target_defaults(ToraClone)
target_link_libraries(ToraClone PRIVATE tora::core Qt6::Widgets)
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
