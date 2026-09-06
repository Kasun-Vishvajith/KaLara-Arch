add_library(kalara_editor
    include/kalara/editor/edit_state.hpp
    include/kalara/editor/viewport_state.hpp
    include/kalara/editor/grid_settings.hpp
    include/kalara/editor/viewport_widget.hpp
    include/kalara/editor/library_browser_widget.hpp
    include/kalara/editor/level_manager_widget.hpp
    include/kalara/editor/site_plan_widget.hpp
    include/kalara/editor/validation_widget.hpp
    include/kalara/editor/main_window.hpp
    src/viewport_widget.cpp
    src/library_browser_widget.cpp
    src/level_manager_widget.cpp
    src/site_plan_widget.cpp
    src/validation_widget.cpp
    src/main_window.cpp
)

target_include_directories(kalara_editor PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(kalara_editor PUBLIC
    kalara_runtime
    kalara_exporters
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGLWidgets
)

target_compile_features(kalara_editor PUBLIC cxx_std_20)

add_executable(kalara_arch
    src/main.cpp
)

target_link_libraries(kalara_arch PRIVATE
    kalara_editor
)

target_compile_features(kalara_arch PRIVATE cxx_std_20)
