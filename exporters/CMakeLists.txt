add_library(kalara_exporters
    include/kalara/exporters/export_format.hpp
    include/kalara/exporters/export_settings.hpp
    include/kalara/exporters/dxf_writer.hpp
    include/kalara/exporters/dxf_exporter.hpp
    include/kalara/exporters/svg_exporter.hpp
    include/kalara/exporters/json_exporter.hpp
    include/kalara/exporters/pdf_exporter.hpp
    src/export_format.cpp
    src/dxf_writer.cpp
    src/dxf_exporter.cpp
    src/svg_exporter.cpp
    src/json_exporter.cpp
    src/pdf_exporter.cpp
)

target_include_directories(kalara_exporters PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(kalara_exporters PUBLIC
    kalara_architecture
    Qt6::Gui
)

target_compile_features(kalara_exporters PUBLIC cxx_std_20)
