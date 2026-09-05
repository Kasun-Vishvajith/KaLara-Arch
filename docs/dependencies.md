# KaLara Arch — Dependencies & Toolchain Specification

## Environment Status (Step 00)

| Component | Target / Specification | Current Host Status |
|---|---|---|
| Operating System | Windows 10/11 x64 | Windows (Detected) |
| Version Control | Git 2.x+ | Git 2.54.0.windows.1 (Installed & Verified) |
| Build System | CMake 3.25+ | CMake 4.4.3 (Installed & Verified) |
| Python Environment | Python 3.10+ in `tools/python/.venv/` | Python 3.14.5 (Installed & Verified in venv) |
| C++ Standard | C++20 | Requires C++20 capable compiler |
| C++ Compiler | MSVC (VS 2022 Build Tools v143) or LLVM Clang 17+ or MinGW-w64 (GCC 13+) | **Missing** (No C++ compiler found on system) |
| Build Generator | Ninja or MSBuild | Requires installation |
| UI Framework | Qt 6 (6.6+ / 6.7+ recommended, e.g., Core, Gui, Widgets) | **Missing** (Qt 6 not found on system) |
| Package / Dependency Manager | vcpkg, CMake FetchContent, or system Qt | Pending toolchain setup |

## Recommended Toolchain Options for Windows

### Option 1: Visual Studio 2022 Build Tools (MSVC) + Qt 6 (Recommended for Windows)
1. **Compiler & Tools**: Visual Studio 2022 Build Tools with C++ workload (MSVC `cl.exe`, CMake, Ninja)
   - Command: `winget install Microsoft.VisualStudio.2022.BuildTools --override "--passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"`
2. **Qt 6**: Qt 6.7+ for MSVC 2022 x64 (via official Qt installer, `aqtinstall`, or `vcpkg install qtbase:x64-windows`)

### Option 2: MinGW-w64 (GCC / Clang) + Qt 6
1. **Compiler**: MinGW-w64 (UCRT64 via MSYS2 or WinLibs)
2. **Qt 6**: Qt 6.7+ MinGW build
