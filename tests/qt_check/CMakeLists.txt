cmake_minimum_required(VERSION 3.25)
project(KaLaraArchQtCheck LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

add_executable(qt_check main.cpp)
target_link_libraries(qt_check PRIVATE Qt6::Core)
