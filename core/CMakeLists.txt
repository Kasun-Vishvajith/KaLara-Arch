add_library(kalara_core
    include/kalara/core/logging.hpp
    include/kalara/core/config.hpp
    include/kalara/core/geometry/tolerances.hpp
    include/kalara/core/geometry/units.hpp
    include/kalara/core/geometry/angle.hpp
    include/kalara/core/geometry/point_vector.hpp
    include/kalara/core/geometry/transform.hpp
    include/kalara/core/geometry/segment.hpp
    include/kalara/core/geometry/rect.hpp
    include/kalara/core/geometry/ops.hpp
    src/logging.cpp
    src/config.cpp
    src/geometry/units.cpp
    src/geometry/segment.cpp
    src/geometry/ops.cpp
)

target_include_directories(kalara_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(kalara_core PUBLIC cxx_std_20)
