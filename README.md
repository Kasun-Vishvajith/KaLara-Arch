# KaLara Arch — Complete 2D Codex Development Pack

**Revision:** 2026-09-06 · **Target:** Windows desktop · **Product:** local, strictly 2D architectural drafting and documentation.

Build an application that feels like a carefully made drafting instrument: exact, immediate, calm, and visually refined. A beginner can draw a room in minutes; an experienced user can control connected geometry, edit exact dimensions, compose sheets, and exchange measured CAD linework.

This pack specifies the software; it does not claim that the software has been implemented or tested. The six supplied documents were reviewed as design inputs. No current application source was supplied, so every old “Done” claim requires repository verification.

## Start here

1. Extract the complete pack.
2. For existing code, keep this pack beside the repository initially and use the existing-repository prompt in [START_HERE.md](START_HERE.md). Adopt through a reviewable diff, not blind replacement.
3. For a fresh project, use this folder as the project root and the fresh-project prompt.
4. Follow [AGENTS.md](AGENTS.md), [scope](docs/01_PRODUCT_SCOPE.md), and [roadmap](docs/17_EXECUTION_ROADMAP.md).
5. Complete the numbered steps in order. The final milestone is a complete offline release, without a runtime AI layer.

## Signature features

- Semantic walls and hosted openings with reliable joins.
- Change dimensions dialog: selected geometry, connected network, active floor, or explicitly selected floors; X, Y, or uniform scaling; anchor; preview; one Undo.
- Distinct Resize Layout, Scale Geometry, Calibrate Underlay, Display Units, and Drawing Scale operations.
- Distance, angle, area, and associative dimension tools.
- Plan-native doors, windows, stairs, fixtures, furniture, hatches, schedules, and drawing sheets.
- Procedural symbols immediately, with actual folders for later SVG artwork. Missing artwork never disables drafting.
- A polished Qt shell, measured frame budgets, crash recovery, keyboard workflows, and verified export scale.

## Navigation

| Purpose | Document |
|---|---|
| Start and resume Codex | [START_HERE.md](START_HERE.md) |
| Agent rules | [AGENTS.md](AGENTS.md) |
| Safe setup | [KA_LARA_ARCH_SAFE_ENV.md](KA_LARA_ARCH_SAFE_ENV.md) |
| Resolve old plans | [Source reconciliation](docs/00_SOURCE_RECONCILIATION.md) |
| Product boundary | [Scope](docs/01_PRODUCT_SCOPE.md) |
| Layers and dependencies | [Architecture](docs/02_ARCHITECTURE.md) |
| Persistent entities | [Model](docs/03_MODEL_AND_SCHEMA.md) |
| Atomic editing | [Commands and history](docs/04_COMMANDS_AND_HISTORY.md) |
| Numerical contracts | [Geometry](docs/05_GEOMETRY_AND_TOPOLOGY.md) |
| Visual quality | [UI specification](docs/06_UI_AND_VISUAL_DESIGN.md) |
| Input and tool states | [Tools](docs/07_TOOLS_AND_INPUT.md) |
| Signature scaling behavior | [Scaling](docs/08_SCALING_AND_SMART_EDITING.md) |
| Architectural authoring | [Objects](docs/09_ARCHITECTURAL_OBJECTS.md) |
| SVGs and fallback assets | [Library](docs/10_LIBRARY_AND_SVG.md) |
| Measurements and sheets | [Documentation](docs/11_DIMENSIONS_AND_DOCUMENTATION.md) |
| Save and recover | [Persistence](docs/12_PROJECTS_AND_RECOVERY.md) |
| CAD and printing | [Interchange](docs/13_INTERCHANGE_AND_PRINT.md) |
| Rules and diagnostics | [Validation](docs/14_VALIDATION_AND_CONSTRAINTS.md) |
| Smoothness budgets | [Performance](docs/15_PERFORMANCE.md) |
| Release evidence | [Testing](docs/16_TESTING_AND_RELEASE.md) |
| Implementation order | [Roadmap](docs/17_EXECUTION_ROADMAP.md) |
| Feature traceability | [Requirements](docs/18_REQUIREMENTS_MATRIX.md) |
| Official sources | [References](docs/19_REFERENCES_AND_DEPENDENCIES.md) |
| Session handoff | [Progress](docs/PROGRESS.md) |
| Artwork folders | [library/README.md](library/README.md) |

All editing is 2D plan drawing and 2D sheet composition. Multiple floors are separate plans. No 3D viewport, extrusion, IFC, Blender bridge, or future 3D milestone exists in this pack. Codex develops the application; it is not required to use the application.

Professional quality is an acceptance target, not a certification claim. Included artwork placeholders are not a licensed standards collection.
