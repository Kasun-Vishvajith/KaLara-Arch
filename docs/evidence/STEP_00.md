# Step 00 — PASS (2026-09-06)

Pack adopted under the startup instruction. Baseline: 868dc8b (Fresh Branch), with no tracked files. All supplied documentation and library READMEs were untracked user inputs; preserved without replacement. Adoption adds evidence and build probes, and updates progress. No superseded source existed.

## Environment

Windows NT 10.0.26200.0, x64 target. MSVC 19.44.35228, toolset directory 14.44.35207, Visual Studio Build Tools 17.14.37614.0. Windows SDK 10.0.26100.0. Qt 6.7.2 msvc2019_64 shared kit at C:/Qt/6.7.2/msvc2019_64. CMake 4.4.3, Ninja 1.12.1, Git 2.54.0.windows.1, bundled development Python 3.12.14. Qt and MSVC were not in initial PATH; discovered with qmake -query, vswhere, and filesystem inventory. Compilation/linking verifies this kit pairing for the probe only.

## Commands and results

- `tools/probe/build.cmd`: PASS. Initializes vcvars64, configures Ninja in build/probe, builds two C++20 executables, invokes CTest. Full output: step00-build.log.
- `ctest --test-dir build/probe --output-on-failure`: PASS 1/1, 0.06 seconds on final run. Fixture environment-v1: hypot(6000,4000) expected 7211.102550927979 mm within 1e-9 mm, with an explicit nonzero failure exit (not a disabled release assertion).
- Qt run: prepend C:/Qt/6.7.2/msvc2019_64/bin to PATH and run build/probe/qt_probe.exe. PASS: visible=1 capture=1 platform=windows, exit 0. Screenshot step00-window.png inspected; visible text and window content confirmed. QTimer closes the probe after capture.
- Optional Vulkan headers unavailable; Qt Widgets probe configures and runs without them. This is not GPU viewport proof.
- Generated output is isolated in ignored build/. No application source existed to preserve or regress.

## Remaining scope

Application features R01–R28 remain MISSING as detailed in BASELINE_AUDIT.md. This gate proves the environment, not the product. Packaging, non-ASCII path and clean standard-user/offline release checks NOT RUN. Next: Step 01.
