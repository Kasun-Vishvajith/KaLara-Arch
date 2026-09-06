# Reproducible development environment

Use a user-owned project folder such as D:\Dev\KaLaraArch. Inspect existing Git status first. Do not initialize over an unrelated repository or delete user caches as a troubleshooting shortcut.

## Toolchain

C++20, Windows x64, MSVC-compatible Qt 6 kit, CMake, Ninja, Git. Preserve an existing compatible working kit. Step 00 records exact compiler, Qt patch, SDK, CMake, Ninja, and Python versions; never guess paths or mix incompatible Qt/compiler ABIs.

Create docs/dependencies.md with dependency version, source, immutable revision/checksum, license, linkage, runtime role, and installation method. Pin the actual JSON, polygon-operation, archive, and test libraries before their corresponding feature builds pass. Prefer adequate existing dependencies.

## Layout

Source: core/, architecture/, runtime/, editor/, render/, persistence/, importers/, exporters/, library/, tests/, tools/, docs/.

Generated: build/, out/, dist/, tools/python/.venv/. Ignore generated data in Git. Portable CMakePresets.json; ignored CMakeUserPresets.json for machine paths. Implement windows-debug, windows-release, and core-tests presets with matching build/test configuration.

## Routine safety

Read/edit repository files, run approved local builds, create a local Python environment, update evidence, and commit under startup authorization. Normal development needs no administrator shell. Do not alter security settings, install arbitrary binaries, modify unrelated projects, expose secrets, or execute scripts embedded in imported assets.

If a compiler or Qt kit is unavailable, complete the available audit and report the exact prerequisite. Do not fake native checks. Actual environment access rules still apply to installation.

## Step 00 proof

Record tool versions, a minimal compile and test, and a Qt window run. Native Windows proof requires Windows. Record working source/build separation and the baseline commit. Before release test a non-ASCII path and standard user account.

Application-owned code targets Apache-2.0. Dependencies and artwork retain their own terms. Verify the actual Qt module/license/linkage combination and ship required notices; the application license does not relicense its dependencies. Use docs/19_REFERENCES_AND_DEPENDENCIES.md.
