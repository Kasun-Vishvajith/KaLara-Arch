add_library(kalara_architecture
    include/kalara/architecture/model_info.hpp
    include/kalara/architecture/entity_id.hpp
    include/kalara/architecture/metadata.hpp
    include/kalara/architecture/semantics.hpp
    include/kalara/architecture/wall.hpp
    include/kalara/architecture/wall_join.hpp
    include/kalara/architecture/opening.hpp
    include/kalara/architecture/room.hpp
    include/kalara/architecture/room_boundary_detector.hpp
    include/kalara/architecture/annotation.hpp
    include/kalara/architecture/level.hpp
    include/kalara/architecture/building.hpp
    include/kalara/architecture/site.hpp
    include/kalara/architecture/project.hpp
    src/model_info.cpp
    src/entity_id.cpp
    src/wall.cpp
    src/wall_join.cpp
    src/opening.cpp
    src/room.cpp
    src/room_boundary_detector.cpp
    src/annotation.cpp
)

target_include_directories(kalara_architecture PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(kalara_architecture PUBLIC
    kalara_core
)

target_compile_features(kalara_architecture PUBLIC cxx_std_20)
