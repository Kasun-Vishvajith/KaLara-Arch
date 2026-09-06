# Verification, release gates, and definition of done

## Test layers

Core unit/property tests cover units, transforms, intersections, polygon areas, topology, and degenerate input. Command tests prove atomicity, dependency updates, cancel, history, and locks. Persistence tests prove semantic round-trip and failure survival. UI interaction tests cover action routing, tool lifecycle, dialogs, and focus. Visual inspection checks composition and print rendering. Performance tests prove the explicit budgets.

Use CTest with the selected pinned C++ test framework; Qt Test is appropriate for widget interaction. Development-only Python can generate fixtures and independently inspect exports. Do not require Python in the shipped application.

## Golden workflows

| ID | Workflow and pass evidence |
|---|---|
| G01 | New empty, draw enclosure, place openings, room, furniture, save/reopen; numeric geometry unchanged |
| G02 | X-only 6000→7500 connected resize; Y dimensions unchanged, 900 mm door preserved, net area 27.74 m² |
| G03 | Uniform geometry scale with thickness/width policy; expected lengths and area; one Undo |
| G04 | Diagonal axis-only target math; impossible request rejected unchanged |
| G05 | Drag shared node, cancel, repeat/commit, Undo/Redo; no gap or stale opening |
| G06 | Split wall before opening remaps host; split through opening blocks |
| G07 | Measure angle/distance, pin dimension, move reference; correct updated measurement |
| G08 | Floor copy, underlay, explicit snap, scoped edit; unselected floors/site unchanged |
| G09 | Empty SVG directories; complete starter library usable and exported |
| G10 | Custom SVG embed, external asset removal, reopen, update instances, Undo |
| G11 | Crash recovery and failed save preserve original; recovery opens as copy |
| G12 | A3 fixed-scale PDF, DXF independent reader/viewer, schedule CSV |
| G13 | Keyboard inspector editing, focus loss mid-drag, high DPI and both themes |
| G14 | Complete session disconnected from internet on clean Windows install |

## Fixture expectations

Create a 6000×4000 mm centerline enclosure with 200 mm walls, centered 900 mm door on south wall and 1200 mm window on north. Net room=22.04 m². A standalone 3000/4000/5000 triangle verifies distance/angle. Add a courtyard, T junction, diagonal wall, conflicting fixed constraint, user SVG, invalid project, and two floors.

Expected numbers are fixed independently from implementation. Do not compute test expected values by calling the same algorithm under test. Generated test fixtures must be reproducible.

## Test evidence templates

For each step: requirement IDs, implementation commit, configure/build commands, test command and result, actual numeric assertions, manual actions/result, screenshots where useful, platform, fixture hash, limitations, next step. Attach failing logs as well as passing evidence.

For an unavailable environment: mark native checks NOT RUN/BLOCKED and identify the needed environment. Do not replace a required Windows application run with source inspection.

## Release gates

1. Every requirement row has implementation and evidence.
2. No critical/high defect remains: data loss, broken ordinary workflow, invalid measurements, undo corruption, unusable UI.
3. No mandatory feature hidden behind a stub or deferred to AI.
4. G01–G14 pass with actual evidence.
5. Performance budgets pass on recorded reference hardware.
6. Independent CAD and PDF scale verification passes.
7. Clean Windows standard-user package runs without build tools, Python, AI, or network.
8. Third-party asset/dependency licenses, runtime components, and notices match distribution.
9. Reproducible release build, checksums, release notes, supported limits, recovery instructions, and known minor issues exist.

Do not call release-ready when an external-viewer or Windows check is blocked. A release candidate with explicit missing evidence is an honest intermediate artifact, not a passed gate.

## Packaging

Produce a portable ZIP first and an installer using the chosen Windows packaging tool only after portable validation. Package Qt platform/image/SVG support actually required by the selected module set, application assets, licenses, and runtime prerequisites. Use the official Qt Windows deployment workflow and inspect the resulting package rather than assuming the helper found everything.

Program installation directory is read-only at runtime. Settings/cache/recovery/user library use user-data directories. Uninstall must not delete user projects. Test launch from a path containing spaces/non-ASCII characters, read-only project error, no developer PATH, and missing graphics support fallback.

## Final acceptance walkthrough

A new user can install, create an accurate furnished two-floor plan, inspect/change measurements with preview and Undo, produce drawing sheets/schedules, save/recover, and export measured CAD/PDF without assistance. Visual inspection and performance evidence must support the claim that the local application is polished.
