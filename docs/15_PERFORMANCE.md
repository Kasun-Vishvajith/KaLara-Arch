# Performance and responsiveness budgets

These are proposed release targets to measure, not claims of achieved performance.

## Reference environment and fixtures

Baseline target: Windows x64, 4+ physical CPU cores, 16 GB RAM, SSD, integrated GPU capable of the selected renderer, 1920×1080 at 100% scaling. Record exact CPU/GPU/driver/OS/build. Also test 150% and 200% scaling and the user's actual machine when available. Do not infer performance from machine category alone.

| Fixture | Composition |
|---|---|
| Small | 100 walls, 30 openings, 100 symbols, 100 dimensions |
| Residential | 1000 walls, 300 openings, 2000 symbols, 1000 annotations across 5 floors |
| Large | 10000 walls, 3000 openings, 20000 symbols, 10000 annotations across 20 floors |
| Dense view | Large fixture with 10000 visible primitives/entities representative of mixed drafting |

Generate with fixed seeds and valid topology; include heavily connected edits and repeated library artwork. Publish generation parameters and fixture hashes.

## Budgets

| Operation | Required target |
|---|---|
| Pan/zoom/drag, Residential | p95 frame ≤16.7 ms; p99 ≤33.3 ms |
| Dense view | p95 ≤33.3 ms with documented LOD |
| Pointer to visible preview | p95 ≤50 ms |
| Snap query, Residential | p95 ≤4 ms |
| Ordinary local commit | p95 ≤50 ms, excluding explicitly deferred analysis |
| First usable empty window | ≤2 s warm, ≤5 s cold |
| Residential save/open | ≤2 s each, UI remains responsive |
| Large save/open | ≤10 s each with progress/cancel |
| Memory, Residential | ≤750 MiB working set after stabilization |
| Memory, Large | ≤2 GiB working set; no unbounded growth |
| Idle | No continuous redraw; near-idle CPU, target <2% on reference machine |

A fallback renderer may have a separately documented lower large-plan target, but must retain correct geometry and all editing behavior. Ship a clear renderer preference/diagnostic, not a silent unusable mode.

## Strategy

Spatial index for hit tests, snapping, and viewport culling. Cache wall outlines by dependency revision. Rebuild only affected wall neighborhoods/rooms. Cache SVG normalization and thumbnails by asset hash/size/theme. Batch draw primitives and reuse glyph data. Cull offscreen geometry and use level-of-detail for tiny furniture details; never omit essential wall edges or selected objects.

Avoid whole-project copies, JSON snapshots, all-object hit scans, per-frame SVG parsing, and room detection inside paint events. Inspector rebuild coalesces committed events; mouse hover does not regenerate every dock.

Interactive preview prioritizes affected outlines and dimensions. Heavy room/validation results can arrive shortly after commit with an explicit updating state; do not display stale values as current.

## Measurement method

Release build, fixed viewport and script, 10-second warmup, 60-second recorded interaction, at least five runs. Record median, p95, p99 and max; include raw frame samples. Distinguish CPU preparation from frame presentation and note vsync. Measure pointer latency end-to-end where tooling permits; otherwise label the proxy.

Measure cold startup after a documented clean start and warm startup separately. Memory test: 100 open/close cycles and 1000 undo/redo operations, then wait for caches to settle. Explain retained caches, not just peak RAM.

Step 05 establishes instrumentation; later steps must watch regression. Do not defer every performance decision to final polish. Profiling-driven fixes preserve semantics and numerical accuracy.
