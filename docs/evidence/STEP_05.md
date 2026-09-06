# Step 05 — PASS (2026-09-07)

Source follows `2a95e34`. Fixture `render-v1`; `tests/render_tests.cpp` SHA-256 `6AB901F91685458DAF8D14331879D7AB0294B02512DF0F19D45C23126F2F3E22`.

## Implemented and verified

- Qt-free scene extraction converts semantic walls from immutable project snapshots into fills and face lines. Source revisions travel with primitives; rendering never mutates the model.
- A baseline wall bounds index filters by world rectangle and floor before scene extraction. Derived bounds include wall thickness.
- `Camera2D` uses orthographic 2D millimetres, camera-local mapping before display conversion, Y-up world coordinates, cursor-anchored zoom, DIP pan, fit with margin, and bounded scale. There is no 3D camera or persistent viewport geometry.
- `PlanViewport` uses the same scene and drawing routine for a `QOpenGLWidget` GPU-capable surface and ordinary `QWidget` painter fallback. The user can switch fallback explicitly; the status reports the active renderer. Offscreen tests select painter automatically.
- Adaptive grid, major/minor visual hierarchy, theme-resolved architectural ink, upright text, zoom/pan input, Fit Plan, 100/150/200% image rendering, and an accessible viewport name.
- A bounded frame recorder stores actual paint durations and reports median, p95, p99, and max. Painting is event driven; an idle test demonstrates no continuous redraw.
- Each `DocumentSession` now owns its `ProjectStore` and `CommandService`; the main window does not own global mutable document state.

## Commands and results

- `tools/build.ps1 windows-debug`: PASS 6/6 (`step05-debug.log`). Camera/renderer suite covers 1000 large-coordinate round trips, cursor anchoring, pan, culling, bounds, recorder quantiles, identical shared-render output, DPR dimensions, and idle redraw behavior.
- Native Windows `shell_tests.exe` with Qt bin on PATH and `QT_QPA_PLATFORM=windows`: PASS 5/5 (`step05-native.txt`). `QOpenGLWidget::isValid()` passed before capture; switching to painter fallback passed and produced a second native capture.
- `tools/build.ps1 windows-release`: PASS 6/6 (`step05-release.log`).

The deterministic painter images at DPR 1.0, 1.5, and 2.0 exactly match the image produced through the GPU-mode viewport's shared scene renderer at each DPR. Image sizes are 800x500, 1200x750, and 1600x1000 physical pixels for an 800x500 DIP viewport, and all contain nonbackground plan pixels. Native light/dark GPU and dark painter captures were inspected. A first native run exposed low-contrast fixed ink and a dominant grid in dark mode; palette-resolved ink and restrained major/minor grid tokens fixed it before acceptance.

Native frame samples during the UI test: count 14, median 3.3032 ms, p95/p99/max 58.4743 ms. This sample mixes resizes, theme changes, screen captures, and backend replacement, has no warmup, and is recorded as instrumentation proof only. It does not satisfy or fail Step 24's interaction budget. The requested 1920x1080 DIP view was constrained by the 125% desktop to 1539x844 DIP; full target-resolution visual evidence remains NOT RUN.

## Limits and next work

The bounds index is a correct baseline with linear rebuild/query; performance indexing and representative residential/large fixtures remain for Step 24. Wall joins/openings are not yet derived. Step 06 adds selection, normalized tool lifecycle, snapping, keyboard input, preview cancellation, and cycling over this viewport.
