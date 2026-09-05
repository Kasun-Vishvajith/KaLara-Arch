add_library(kalara_architecture
    include/kalara/architecture/model_info.hpp
    src/model_info.cpp
)

target_include_directories(kalara_architecture PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(kalara_architecture PUBLIC
    kalara_core
)

target_compile_features(kalara_architecture PUBLIC cxx_std_20)
