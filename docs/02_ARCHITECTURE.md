# Architecture and module contracts

## Fixed implementation strategy

C++20 is the runtime language. Qt 6 Widgets provides the native shell, docks, dialogs, accessible controls, printing adapters, and platform integration. Use QOpenGLWidget for the primary 2D viewport, with a shared 2D scene description consumed by a QPainter raster fallback. This is orthographic 2D drawing; OpenGL usage does not imply any 3D feature. Python supports fixture generation and asset validation during development only.

Preserve an existing renderer if it satisfies these boundaries and measured budgets. Do not switch to a web app or add QML solely to make the shell attractive. Build visual quality with consistent tokens, spacing, typography, and interaction design.

## Dependency graph

~~~mermaid
flowchart TD
  E["Editor and tools"] --> R["Runtime command services"]
  R --> A["Architectural model"]
  A --> C["Core geometry and units"]
  P["Persistence and exchange adapters"] --> A
  V["2D scene builder"] --> A
  E --> V
~~~

| Module | Owns | Must not own |
|---|---|---|
| core | Units, vectors, transforms, robust predicates, IDs, result types | Qt widgets, file dialogs, active selection |
| architecture | Entities, topology, references, schema values, rules | Window pointers, OpenGL handles, UI flags |
| runtime | Command service, transactions, history, dependency updates, query snapshots | Tool-specific mouse state |
| editor | Document sessions, tools, selection, inspector, action registry, shell | Independent architectural truth |
| render | Scene extraction, tessellation/cache, painter and GPU backends | Persistent entity mutation |
| persistence | Container, schema codecs, migration, asset embedding, atomic platform I/O | Alternative editing API |
| importers | Bounded parsing into candidate entities and diagnostics | Direct modification of the open document |
| exporters | Snapshot-to-format transformations and losses report | Guessing missing architectural meaning |
| library | Versioned definitions, validated artwork, metadata | Executable plugins/scripts |
| tests | Numerical, command, integration, UI, fixture and performance evidence | Fake production success behavior |

## Key interfaces to implement

- ProjectStore exposes immutable revision snapshots and entity lookup by ID.
- CommandService.prepare(request, baseRevision) returns a ChangePlan or structured diagnostics.
- CommandService.commit(changePlan) validates the base revision, applies atomically, updates indexes, emits one committed ChangeSet.
- ToolController receives normalized pointer/key events, reads snapshots, and produces preview state or command requests.
- SceneBuilder.build(snapshot, viewSpec, preview) returns 2D primitives and bounds. Semantic queries stay independent of the rendered scene.
- ExportService.run(snapshot, exportSpec) reports files, warnings, and omitted representations.
- DocumentSession owns tool state, selected IDs, camera, active floor, panel state, and one document's history binding.
- AssetResolver resolves document-embedded assets first, then pinned installed packs, then procedural fallback.

These are logical contracts, not instructions to create unnecessary abstract base classes for every object.

## Threading and lifecycle

One owning thread serializes committed document mutation. Workers consume immutable snapshots for room reconstruction, heavy validation, thumbnails, and export. Worker results carry project ID, base revision, job ID, and cancellation token. Stale results are discarded, never applied to a newer revision.

A preview carries only changed entities plus necessary dependencies. No full-project JSON serialization on pointer movement. Do not emit one signal per property during a transaction. Emit one committed event with changed IDs/categories after state and indexes are coherent.

GPU resources belong to the current graphics context. Release/recreate them on context changes. CPU scene data survives a graphics reset. The fallback is a real painter-backed viewport over the same scene data, not a screenshot of a failed GL widget.

## UI wiring

Use an ActionRegistry with ID, label, shortcut, icon key, capability predicate, and handler. Menus, toolbar, command search, and help share these definitions. A ToolRegistry defines tool states and status hints. A document-scoped signal informs all panels of selection changes after session state is updated.

Do not accumulate wall placement, scaling math, serialization, and printing inside viewport_widget.cpp. Existing monolithic code may be extracted incrementally with behavior-preserving tests.

## Error contract

Errors have code, severity, human message, entity IDs, field path, and suggested action. No successful result with a hidden failed suboperation. A failed project load leaves the currently open project untouched. A failed command leaves state and history untouched. Logs omit user content unless explicitly enabled for diagnostics.

## Repository adoption

Existing names may differ. Reuse a compatible TransactionManager or ModelManipulator behind these contracts. Record a mapping instead of creating parallel managers. New source files and CMake targets must preserve dependency direction.
