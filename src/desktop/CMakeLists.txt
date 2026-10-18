qt_standard_project_setup(REQUIRES 6.8)
qt_add_library(desktop_shell STATIC transportation_printing.cpp transportation_printing.hpp lp_printing.cpp lp_printing.hpp application_shell.cpp application_shell.hpp lp_screens.cpp lp_input_controller.cpp lp_input_controller.hpp lp_simplex_controller.cpp lp_simplex_controller.hpp lp_graphical_controller.cpp lp_graphical_controller.hpp lp_final_controller.cpp lp_final_controller.hpp lp_changes_controller.cpp lp_changes_controller.hpp lp_changes_screens.cpp assignment_controller.cpp assignment_controller.hpp assignment_screens.cpp equations_document.cpp equations_document.hpp equations_controller.cpp equations_screens.cpp equations_screens.hpp games_document.cpp games_document.hpp games_controller.cpp games_screens.cpp games_screens.hpp integer_controller.cpp integer_controller.hpp integer_screens.cpp planning_document.cpp planning_document.hpp planning_screens.cpp planning_screens.hpp networks_document.cpp networks_document.hpp networks_controller.cpp networks_screens.cpp networks_screens.hpp queuing_document.cpp queuing_document.hpp queuing_controller.cpp queuing_screens.cpp queuing_screens.hpp transportation_controller.cpp transportation_screens.cpp transportation_screens.hpp)
tora_target_defaults(desktop_shell)
target_include_directories(desktop_shell PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(desktop_shell PUBLIC Qt6::Widgets Qt6::PrintSupport tora::compat tora::transportation_compat tora::assignment tora::games tora::equations tora::queuing tora::networks tora::planning tora::integer)
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
