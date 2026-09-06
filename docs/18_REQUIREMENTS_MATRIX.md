# Requirement traceability

All rows are required. During implementation add source/commit, evidence path, and current status. This delivery intentionally marks every row NOT VERIFIED because no implementation was built in this documentation task.

| ID | Requirement | Steps | Evidence gate | Initial status |
|---|---|---|---|---|
| R01 | Strictly local 2D Windows application; no runtime AI/3D | 00,01,25,26 | G14 | NOT VERIFIED |
| R02 | Typed mm geometry, units and numerical precision | 02,03 | Unit/geometry fixtures | NOT VERIFIED |
| R03 | Stable IDs, typed references and topology | 03,07,08 | G05,G06 | NOT VERIFIED |
| R04 | Atomic commands, cancel and undo/redo | 04 | G03,G05 | NOT VERIFIED |
| R05 | Smooth viewport, pan/zoom/grid and fallback | 05,24 | Frame/fallback evidence | NOT VERIFIED |
| R06 | Snapping, selection, tool states and keyboard input | 06 | G05,G13 | NOT VERIFIED |
| R07 | Semantic wall creation, join/edit/thickness | 07,08 | G01,G05,G06 | NOT VERIFIED |
| R08 | Hosted doors/windows and fit/flip/width | 08 | G01,G06 | NOT VERIFIED |
| R09 | 2D primitives, offset, trim/extend, hatch/text | 09 | Draft fixtures | NOT VERIFIED |
| R10 | Transforms, clipboard, groups/layers/locks | 10 | Clipboard/mirror/lock fixtures | NOT VERIFIED |
| R11 | Distance/angle/area measurement | 11 | G07 | NOT VERIFIED |
| R12 | Associative dimensions and annotation styles | 11,19 | G07,G12 | NOT VERIFIED |
| R13 | Supported constraints and driving dimensions | 12 | Constraint fixtures | NOT VERIFIED |
| R14 | X/Y/uniform scoped scaling and all edge cases | 13 | G02,G03,G04 | NOT VERIFIED |
| R15 | Semantic library, procedural defaults, SVG paths | 14 | G09,G10 | NOT VERIFIED |
| R16 | Rooms/holes/metadata, columns/stairs/fixtures | 15 | Room/stair fixtures | NOT VERIFIED |
| R17 | Floors/underlays and explicit cross-floor scopes | 16 | G08 | NOT VERIFIED |
| R18 | Site/north/setbacks and reference calibration | 16 | Site/calibration fixtures | NOT VERIFIED |
| R19 | Empty/template startup and explicit unknowns | 17 | G01/template fixture | NOT VERIFIED |
| R20 | Portable schema/assets, atomic save and recovery | 18 | G10,G11 | NOT VERIFIED |
| R21 | Sheets/title blocks/scales/schedules/CSV | 19 | G12 | NOT VERIFIED |
| R22 | PDF/SVG/JSON/PNG/JPEG and print | 20 | G12 | NOT VERIFIED |
| R23 | Bounded DXF import/export and measured external proof | 21 | Independent CAD fixture | NOT VERIFIED |
| R24 | Integrated diagnostics and rule provenance | 22 | G01–G12 | NOT VERIFIED |
| R25 | Polished light/dark UI, high DPI, help, accessibility | 23 | G13/visual suite | NOT VERIFIED |
| R26 | Measured performance and resource stability | 24 | Performance suite | NOT VERIFIED |
| R27 | Clean Windows packaging, offline use and notices | 25 | G14 | NOT VERIFIED |
| R28 | All mandatory gates pass; no placeholders | 26 | Release audit | NOT VERIFIED |

A row can pass only when all detailed linked domain requirements also pass. The short table does not narrow those contracts. R14 includes impossible diagonal targets, scope, locks, preserved sizes, conversion disclosure, cancel, and Undo—not merely a visible scale button.

Artwork placeholders are permitted by R15 with procedural behavior. Runtime feature placeholders fail R28. Release status is distinct from specification completeness.
