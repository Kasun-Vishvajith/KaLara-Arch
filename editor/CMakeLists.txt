add_library(kalara_editor
    include/kalara/editor/main_window.hpp
    src/main_window.cpp
)

target_include_directories(kalara_editor PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(kalara_editor PUBLIC
    kalara_runtime
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

target_compile_features(kalara_editor PUBLIC cxx_std_20)

add_executable(kalara_arch
    src/main.cpp
)

target_link_libraries(kalara_arch PRIVATE
    kalara_editor
)

target_compile_features(kalara_arch PRIVATE cxx_std_20)
