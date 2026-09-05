cmake_minimum_required(VERSION 3.25)

project(KaLaraArch
    VERSION 0.1.0
    DESCRIPTION "Open-source, human-first, AI-native architectural planning system"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Qt Automoc, Autouic, Autorcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt 6 Packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Enable Testing
enable_testing()

# Project Subdirectories following layer structure
add_subdirectory(core)
add_subdirectory(architecture)
add_subdirectory(runtime)
add_subdirectory(exporters)
add_subdirectory(editor)
add_subdirectory(tests)
