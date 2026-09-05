add_library(kalara_runtime
    include/kalara/runtime/runtime_context.hpp
    include/kalara/runtime/selection_manager.hpp
    include/kalara/runtime/snapping_engine.hpp
    include/kalara/runtime/model_manipulator.hpp
    include/kalara/runtime/validation_engine.hpp
    include/kalara/runtime/transaction.hpp
    src/runtime_context.cpp
    src/model_manipulator.cpp
    src/validation_engine.cpp
    src/transaction.cpp
)

target_include_directories(kalara_runtime PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(kalara_runtime PUBLIC
    kalara_architecture
)

target_compile_features(kalara_runtime PUBLIC cxx_std_20)
