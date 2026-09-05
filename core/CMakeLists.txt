add_library(kalara_core
    include/kalara/core/logging.hpp
    include/kalara/core/config.hpp
    src/logging.cpp
    src/config.cpp
)

target_include_directories(kalara_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(kalara_core PUBLIC cxx_std_20)
