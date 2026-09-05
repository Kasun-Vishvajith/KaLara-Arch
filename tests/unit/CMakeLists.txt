add_executable(test_foundation
    test_foundation.cpp
)

target_link_libraries(test_foundation PRIVATE
    kalara_core
    kalara_architecture
    kalara_runtime
    kalara_exporters
)

target_compile_features(test_foundation PRIVATE cxx_std_20)
add_test(NAME foundation_test COMMAND test_foundation)

add_executable(test_geometry
    test_geometry.cpp
)

target_link_libraries(test_geometry PRIVATE
    kalara_core
)

target_compile_features(test_geometry PRIVATE cxx_std_20)
add_test(NAME geometry_test COMMAND test_geometry)

add_executable(test_data_model
    test_data_model.cpp
)

target_link_libraries(test_data_model PRIVATE
    kalara_architecture
    kalara_core
)

target_compile_features(test_data_model PRIVATE cxx_std_20)
add_test(NAME data_model_test COMMAND test_data_model)

add_executable(test_viewport
    test_viewport.cpp
)

target_link_libraries(test_viewport PRIVATE
    kalara_editor
    kalara_runtime
    kalara_architecture
    kalara_core
)

target_compile_features(test_viewport PRIVATE cxx_std_20)
add_test(NAME viewport_test COMMAND test_viewport)

add_executable(test_wall
    test_wall.cpp
)

target_link_libraries(test_wall PRIVATE
    kalara_architecture
    kalara_core
)

target_compile_features(test_wall PRIVATE cxx_std_20)
add_test(NAME wall_test COMMAND test_wall)

add_executable(test_room
    test_room.cpp
)

target_link_libraries(test_room PRIVATE
    kalara_architecture
    kalara_core
)

target_compile_features(test_room PRIVATE cxx_std_20)
add_test(NAME room_test COMMAND test_room)
