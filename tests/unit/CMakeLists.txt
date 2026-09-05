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

add_executable(test_opening
    test_opening.cpp
)

target_link_libraries(test_opening PRIVATE
    kalara_architecture
    kalara_core
)

target_compile_features(test_opening PRIVATE cxx_std_20)
add_test(NAME opening_test COMMAND test_opening)

add_executable(test_annotation
    test_annotation.cpp
)

target_link_libraries(test_annotation PRIVATE
    kalara_architecture
    kalara_core
)

target_compile_features(test_annotation PRIVATE cxx_std_20)
add_test(NAME annotation_test COMMAND test_annotation)

add_executable(test_constraint
    test_constraint.cpp
)

target_link_libraries(test_constraint PRIVATE
    kalara_architecture
    kalara_core
)

target_compile_features(test_constraint PRIVATE cxx_std_20)
add_test(NAME constraint_test COMMAND test_constraint)

add_executable(test_manipulation
    test_manipulation.cpp
)

target_link_libraries(test_manipulation PRIVATE
    kalara_runtime
    kalara_architecture
    kalara_core
)

target_compile_features(test_manipulation PRIVATE cxx_std_20)
add_test(NAME manipulation_test COMMAND test_manipulation)

add_executable(test_library
    test_library.cpp
)

target_link_libraries(test_library PRIVATE
    kalara_runtime
    kalara_architecture
    kalara_core
)

target_compile_features(test_library PRIVATE cxx_std_20)
add_test(NAME library_test COMMAND test_library)

add_executable(test_multilevel
    test_multilevel.cpp
)

target_link_libraries(test_multilevel PRIVATE
    kalara_runtime
    kalara_architecture
    kalara_core
)

target_compile_features(test_multilevel PRIVATE cxx_std_20)
add_test(NAME multilevel_test COMMAND test_multilevel)

add_executable(test_site
    test_site.cpp
)

target_link_libraries(test_site PRIVATE
    kalara_architecture
    kalara_core
)

target_compile_features(test_site PRIVATE cxx_std_20)
add_test(NAME site_test COMMAND test_site)

