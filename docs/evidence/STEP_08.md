# Step 08 — Topology editing and hosted openings

**Status:** PASS  
**Platform:** Windows 11 x64; Qt 6.7.2 MSVC shared debug; CMake 4.4.3; baseline commit `2ecd8ca`.

## Implemented vertical slice

- Added stable semantic door/window entities hosted by wall ID with interval, anchor, hinge, swing, procedural pattern, and optional sill/height metadata.
- Added atomic placement, interval editing, hinge/swing flip, wall reversal, split, compatible merge, and dependency-aware wall deletion. Split retains the start wall ID and remaps later openings; merge retains the requested wall ID and refuses significant junctions.
- Derived wall fills are physically split around opening intervals. Door/window jamb, frame, leaf, slide, pocket, and swing graphics are procedural geometry with no background-color erasure or SVG dependency.
- Door/window tools accept width and pattern, highlight the projected host, show green/red fit feedback, and commit on click. Selected openings expose a center move grip and two host-tangent width grips. Release creates one validated command; cancellation leaves the model unchanged.
- Visible commands expose split, merge, reversal, hinge/swing changes, and a hosted-deletion plan. JSON v1 and its executable schema now round-trip openings.

## Verification

- `tools/build.ps1 windows-debug`: PASS 9/9 CTest (`step08-debug.log`).
- `tools/build.ps1 windows-release`: PASS 9/9 CTest (`step08-release.log`).
- `build/windows-debug/topology_opening_tests.exe`: PASS 23 checks (`step08-topology.txt`). Fixed results include 6000 mm host; 2000/900 placement; reversal to 4000 mm; split-through-opening rejection; split at 1000 mm remap to 1000 mm; merge restoration; overlap/overrun rejection; dependency deletion; exact Undo.
- `build/windows-debug/model_codec_tests.exe tests/fixtures`: PASS 24 checks, 3629-byte deterministic JSON (`step08-codec.txt`). Fixture `project-v1-valid.json` SHA-256 `91ED108613B3675E873D8E79BD8438B97474DEDB2A153B9D7B73E85B97BCCD99`.
- Native Windows `render_tests.exe -o docs/evidence/step08-native-render.txt,txt`: PASS 14/14, zero skipped. Covers actual fill gaps, procedural symbol line ownership, invalid-preview non-commit, valid-preview commit, selection and existing interaction regressions.
- Native fit preview: `step08-opening-preview.png`, SHA-256 `50942B6B343ED3182A3A4FA8D33E2852D1D31E2A746E94ADE16CB2BEC07AC90C`.
- `git diff --check`: PASS; line-ending notices only.

## Result

All Step 08 mandatory acceptance cases are demonstrated. Vulkan headers remain an optional Qt discovery notice and do not affect the painter/OpenGL-backed 2D build or tests.
