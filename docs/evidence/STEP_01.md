# Step 01 — PASS (2026-09-06)

Build source: foundation changes following 79148fa. Fixture shell-v1, source tests/shell_tests.cpp and tests/core_tests.cpp. Scope: application foundation only.

## Implemented / VERIFIED

- C++20 CMake targets kalara_core, kalara_editor, kalara_arch with one-way dependency; portable debug/release/core build and test presets.
- Qt-free structured diagnostic type and logger that emits codes rather than private document messages; standalone core compile/test.
- Individually owned DocumentSession instances with separate selection/camera state and selection notification after state update.
- One ActionRegistry used for menu actions and searchable commands; duplicate IDs reject; New/Close/Exit, dock visibility/reset, theme, search, About are functional. No inert drafting toolbar.
- Native tabbed light/dark shell, named project/inspector docks, status bar, user layout persistence/reset, initial off-screen main-window recovery.

## Test commands / results

- `tools/build.ps1 core-tests`: PASS 1/1, independent configuration without finding/linking Qt (step01-core.log).
- `tools/build.ps1 windows-debug`: PASS 2/2 CTest (step01-debug.log); native visual case intentionally skips under offscreen platform. No claim that the offscreen run is native UI evidence.
- `tools/build.ps1 windows-release`: PASS 2/2 on the earlier shell revision before explicit Ctrl+W fix; final release rebuild remains pending and must run before release acceptance.
- Prepend Qt bin to PATH, set QT_QPA_PLATFORM=windows, run `build/windows-debug/shell_tests.exe -o docs/evidence/step01-native.txt,txt`: PASS 5 cases, no skipped tests. Native keyboard Ctrl+N/Ctrl+W, open/close, session isolation, saved dock restoration/reset and light/dark captures demonstrated.
- Inspected light small and dark large captures. 1366x768 DIP actually achieved at DPR 1.25. Requested 1920x1080 was clamped by Windows to 1539x844 DIP, capture clipped at display edges. That larger-resolution visual acceptance is NOT VERIFIED and remains for Step 23. Filename records requested size, log records actual size.

## Failures found and fixed

1. Developer environment exposed duplicate PATH/Path entries; import retained the wrong one. Deduplicated case-insensitively, retaining vcvars output first. A fresh CMake configure restored proper compiler flags after failed detection.
2. Initial Qt test process failed loading DLLs (0xc0000135). CTest now explicitly prepends linked Qt DLL directory. Both configurations subsequently passed.
3. Native keyboard Close needed explicit Ctrl+W; native test also now activates the window and waits for focus before sending keys. Initial failure output preserved in step01-native-initial-failure.txt.

## Limits / next work

This shell is PARTIAL R01/R25, not a drafting application. Model/history/rendering/persistence are MISSING pending later steps. Full visual/accessibility acceptance, DPI matrix, floating-dock monitor-removal recovery, installer/license notices and non-ASCII paths remain unverified. Step 02: typed units and robust geometry with numerical fixtures.
