add_library(kalara_exporters
    include/kalara/exporters/export_format.hpp
    src/export_format.cpp
)

target_include_directories(kalara_exporters PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(kalara_exporters PUBLIC
    kalara_architecture
)

target_compile_features(kalara_exporters PUBLIC cxx_std_20)
