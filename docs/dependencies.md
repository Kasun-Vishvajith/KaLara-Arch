# KaLara Arch — Dependencies & Toolchain Specification

## Environment Status (Step 00)

| Component | Target / Specification | Host Status (Verified) |
|---|---|---|
| Operating System | Windows 10/11 x64 | Windows (Detected) |
| Version Control | Git 2.x+ | Git 2.54.0.windows.1 (Installed & Verified) |
| Build System | CMake 3.25+ | CMake 4.4.3 (Installed & Verified) |
| Python Environment | Python 3.10+ in `tools/python/.venv/` | Python 3.14.5 (Installed & Verified in venv) |
| C++ Standard | C++20 | Verified (`-std:c++20`) |
| C++ Compiler | MSVC (VS 2022 Build Tools v143) | MSVC 19.44.35228.0 (VS 2022 v17.14.39) (Installed & Verified) |
| Build Generator | Ninja | Ninja 1.12.1 (Installed & Verified) |
| UI Framework | Qt 6 (6.6+ / 6.7+ recommended) | Qt 6.7.2 (`msvc2019_64`) installed in `C:\Qt\6.7.2\msvc2019_64` (Verified) |
| Package / Dependency Manager | System Qt + CMake FindPackage | Functional (`find_package(Qt6 REQUIRED COMPONENTS Core)`) |

## Verification Evidence (Step 00)

1. **C++20 Tiny Test Program**:
   - Source: `tests/env_check/main.cpp`
   - Build Directory: `build/env_check`
   - Result: Compiled with MSVC 19.44.35228.0 and executed successfully:
     `KaLara Arch Step 00: C++ Environment Check OK`

2. **Qt6 Verification Program**:
   - Source: `tests/qt_check/main.cpp`
   - Build Directory: `build/qt_check`
   - Result: Compiled and linked against `Qt6::Core` (Qt 6.7.2) and executed successfully:
     `KaLara Arch Step 00: Qt6 Core Environment Check OK: 6.7.2`
