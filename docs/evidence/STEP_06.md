# Step 06 — PASS (2026-09-07)

Source follows `f2e8a8e`. Fixture `input-v1`; `tests/input_system_tests.cpp` SHA-256 `272C54A70FE8D7E2B4E0FAFF926977D7DD53FC394EA56A20BDEBF8C1599818CD`.

## Implemented and verified

- A document-owned `ToolController` normalizes pointer and key events and gives every tool enter, down/move/up, key, cancel, exit, and draft-state behavior. Tool replacement cancels before exit. Focus loss, document change, and floor change discard a pending draft.
- Selection uses semantic entity IDs rather than viewport items. Hit targets are measured in device-independent pixels, sorted by the specified semantic priority, distance, and stable ID, with stable overlap cycling. Hidden layers are excluded; locked entities remain inspectable and are rejected for movement.
- Left-to-right marquee requires enclosure. Right-to-left marquee selects crossing geometry and renders a dashed blue boundary. Zero-width or zero-height drags are handled without degenerate geometry failures.
- Semantic snapping covers junctions, endpoints, midpoints, intersections, perpendicular feet, nearest points, alignment, grid, host projections, and supplemental centers. Candidates are ranked by semantic priority, DIP distance, and stable key. Reference-only candidates require explicit opt-in and remain marked in the result.
- Snap acquisition uses an 8 DIP radius and 12 DIP release hysteresis independent of zoom. Disabling snapping clears its latch. Ortho and polar constraints are applied before incompatible snap candidates are filtered; Tab cycles the current ranked set.
- The shared `ActionRegistry` owns Select, F3 snapping, F8 ortho, F9 grid snapping, and single-letter tool routing. Ctrl+K opens command search. Editable text focus suppresses letter shortcuts, while the same action handler serves menu, command search, and viewport keys.
- `PlanViewport` shows selected source entities, marquee state, and active snap labels/markers through the shared GPU-capable/painter renderer. Pointer capture, Escape, focus loss, and session cancellation do not persist preview state.

## Commands and results

- `tools/build.ps1 windows-debug`: PASS 7/7 (`step06-debug.log`).
- `build/windows-debug/input_system_tests.exe`: PASS, 42 assertions (`step06-input.txt`). These cover lifecycle cancellation, overlap cycling, hidden/locked filtering, crossing versus enclosure, zero-height marquee, three zoom scales, hysteresis, disabled latch release, reference opt-in, supplemental center candidates, candidate cycling, ortho compatibility, numeric parsing, and focus routing.
- Native Windows `shell_tests.exe` with Qt 6.7.2 on PATH and `QT_QPA_PLATFORM=windows`: PASS 5/5 (`step06-shell-native.txt`). The suite invokes shared F3 and V actions and proves a focused `QLineEdit` receives W without triggering a tool shortcut.
- Native Windows `render_tests.exe` under the same environment: PASS 9/9 (`step06-render-native.txt`). It performs pointer selection and right-to-left marquee interaction on a real Qt window, verifies focus-loss cancellation, and records the resulting images.
- `tools/build.ps1 windows-release`: PASS 7/7 (`step06-release.log`).
- `git diff --check`: PASS; only Git's configured LF-to-CRLF notices were emitted.

Native UI captures were visually inspected. `step06-selected.png` shows the chosen wall with the blue selection treatment; SHA-256 `C5FE62BAF857CAFCECCD9F53B8113BAB61B8E8E421CBAC4271F48BB1759D1747`. `step06-crossing-marquee.png` shows the dashed crossing marquee over the plan; SHA-256 `38CB8B951714AA2827CA0AD822B3265CE5C3C60BCE5D2594E6A8BB6EE2F13AAA`.

The native shell run used Qt 6.7.2 debug/MSVC on Windows 11 and passed 5 tests in 10.497 s. The requested 1920x1080 DIP shell sample remained constrained by the 125% desktop to 1539x844 DIP. Its 14 mixed capture/resize/theme frame samples measured median 2.8521 ms and p95/p99/max 102.463 ms; this remains instrumentation evidence only and is not a Step 24 performance claim.

## Limits and next work

The selection and snap queries currently scan the project; Step 24 must replace this with the specified scalable spatial index and prove the budgets. Step 07 adds actual wall-chain/rectangle authoring, numeric wall entry, connected topology edits, derived wall joins, and topology-preserving Undo using this input layer.
