# Dependency manifest

Observed 2026-09-06; local installed toolchain retained. No downloads performed.

| Dependency | Actual pin/source | License/linkage/runtime role | Installation |
|---|---|---|---|
| Qt | 6.7.2 msvc2019_64, C:/Qt/6.7.2/msvc2019_64; Qt6Core.dll SHA256 8A1AF6B5EA341EF0D01573A9005E5C68206CFEF6853B5584E8A737C26C9D9EE7 | Shared Core/Gui/Widgets; Qt open-source module obligations must be verified with shipped notices before distribution | Existing kit |
| MSVC | 19.44.35228 / toolset 14.44.35207 | Microsoft toolchain terms; compiler development only, redistributable needed at release | Existing VS Build Tools 17.14.37614.0 |
| Windows SDK | 10.0.26100.0 | Microsoft SDK terms, development | Existing kit |
| CMake | 4.4.3 | BSD-3-Clause, development | Existing C:/Program Files/CMake |
| Ninja | 1.12.1 | Apache-2.0, development | Existing VS bundled Ninja |
| Git | 2.54.0.windows.1 | GPL-2.0, development only | Existing installation |
| Python | 3.12.14 | PSF, development only; no application runtime role | Codex bundled runtime |
| Probe test | Repository-owned environment-v1 source | Apache-2.0; CTest orchestration; no third-party framework | In repository |

JSON, polygon operations, archive, full test framework and installer dependencies remain UNSELECTED. They must be pinned and their licenses recorded before their corresponding feature builds can pass. The probe does not use these dependencies. Application-owned code targets Apache-2.0; this does not relicense the installed Qt binaries. Distribution license/notice gate remains NOT RUN.

## Step 01 additions

Qt Test 6.7.2 from the same shared kit is the pinned development-only widget test framework. Core uses repository-owned tests run by CTest; no third-party dependency. No Qt headers or linkage enter kalara_core. Editor depends on core and Qt Widgets; model/runtime/render/adapters will be introduced as implemented in later steps, without empty targets standing in for features.
