# Official references and dependency decisions

Reference check date: 2026-09-06. Links below informed technical choices; exact release dependencies must be selected and pinned against the actual development machine. Do not infer a tested compatibility matrix from these links.

| Subject | Primary source | Use in this pack |
|---|---|---|
| Qt 2D OpenGL widget | [QOpenGLWidget](https://doc.qt.io/qt-6/qopenglwidget.html) | Supports painter/GL rendering; respect context lifetime and widget framebuffer |
| Atomic file writing adapter | [QSaveFile](https://doc.qt.io/qt-6/qsavefile.html) | Temporary write plus commit; retain error handling and safe-write policy |
| Windows deployment | [Qt Windows Deployment](https://doc.qt.io/qt-6/windows-deployment.html) | Package actual Qt/runtime dependencies; verify clean machine |
| SVG support | [Qt SVG](https://doc.qt.io/qt-6/qtsvg-index.html) | Defined renderer support; use restricted validated artwork subset |
| License decisions | [Qt Licensing](https://doc.qt.io/qt-6/licensing.html) | Verify actual modules/license obligations separately from app license |
| Open-source distribution | [Qt LGPL obligations](https://www.qt.io/development/open-source-lgpl-obligations) | Check notices/linkage/distribution requirements for chosen build |
| CAD units | [Autodesk INSUNITS](https://help.autodesk.com/view/ACD/2025/ENU/?guid=GUID-A58A87BB-482B-4042-A00A-EEF55A2B4FD8) | Explicit DXF modelspace unit declaration and measured consumer check |
| Independent DXF checking | [ezdxf units documentation](https://ezdxf.mozman.at/docs/concepts/units.html) | Development-only independent parsing and unit checks |

## Binding versus proposed decisions

Binding: C++20, Qt 6 Widgets, Windows x64, canonical mm, semantic model, local-only 2D, in-process mutation service. Proposed implementation family: QOpenGLWidget with shared painter fallback; preserve a compatible working renderer during adoption. An implementation change requires evidence and an ADR, not an unannounced switch of application stack.

To pin in Step 00 or before first use: Qt patch/kit, compiler/SDK, CMake/Ninja, JSON library, polygon operations, ZIP library, C++ test framework, Python tooling dependencies, installer tool. Record immutable versions and licenses. Do not use floating latest or an unpinned branch in release builds.

Qt Widgets and SVG modules support the selected UI strategy, but merely choosing them does not prove accessibility, performance, malicious-file handling, or deployment correctness. Those are application responsibilities covered by tests.

## Standards policy

No licensed ISO/AIA symbol standard was supplied or verified here. Consequently the pack uses generic editable conventions and provenance fields, not claims of certification. Import a properly licensed standards pack later through the same metadata/artwork system when the user provides it. Do not copy third-party symbols from search results and invent usage rights.

## Architecture decision record template

Decision ID/date; problem and evidence; options; chosen approach; affected requirements; migration; dependencies/licenses; performance/accuracy consequences; validation; approval if outside authorized scope. Keep ADRs short and specific. They cannot silently remove a mandatory product requirement.
