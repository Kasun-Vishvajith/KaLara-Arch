# Building on Windows

Run `tools/build.ps1 windows-debug`, `tools/build.ps1 windows-release`, or `tools/build.ps1 core-tests` from the repository root. The helper discovers VS Build Tools with vswhere, imports its x64 environment, locates bundled Ninja, and uses QTDIR (or the verified C:/Qt/6.7.2/msvc2019_64 kit). Set QTDIR for other machine paths. Portable presets contain no machine paths. Generated files stay in build/.

Start build/windows-debug/kalara_arch.exe with the selected Qt bin directory on PATH. This is a development shell, not the complete drafting product. It supports separate empty document tabs, workspace docks, light/dark theme, command search, and keyboard New/Close. No drawing or save commands are exposed yet.

CTest uses offscreen Qt for deterministic widget tests, and explicitly adds the linked Qt binary directory to its test process PATH. Native Windows evidence requires running shell_tests.exe with QT_QPA_PLATFORM=windows from the repository root. Tests use temporary INI settings and cannot overwrite the user's workspace layout. Native visual captures report actual DIP dimensions and screen scaling; a clamped capture is not proof of the requested larger resolution.
