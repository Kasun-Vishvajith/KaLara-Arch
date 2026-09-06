# KaLara Arch × Arcada — GUI Essence Extraction, Gap Analysis & Addition Plan

**Purpose:** Extract the essence of Arcada (github.com/mehanix/arcada — React + PixiJS floor planner) and define exactly what to add/change in the KaLara Arch GUI to reach that interaction standard, plus the new functionalities to go beyond it.

**Constraint:** Everything here is *editor-layer* work. All model mutations still pass through the TransactionManager choke point (Rule 3), canonical unit stays mm (Principle 3), IDs stay stable (Rule 1). Arcada's patterns are UX patterns, not architecture changes.

---

## PART 1 — The Arcada Essence (what makes it feel good)

Arcada is a direct-manipulation 2D floor planner. Its "feel" comes from five things:

1. **Tool modes, not menu commands.** The app is always in exactly one mode: View / Edit / Erase / Draw Wall / Add Door / Add Window / Measure / Add Furniture. Every click does exactly one thing, predictably. The left icon rail shows the active mode at a glance.
2. **Live preview with a running length label.** While drawing a wall or measuring, a rubber line follows the cursor and a label shows the live dimension. You never commit anything blind.
3. **Direct on-canvas editing.** Walls are edited by dragging their square node handles; furniture is edited by dragging gizmo handles (move / resize H / resize V / resize corner / rotate). There is no separate "edit dialog" for geometry.
4. **Context-sensitive gizmos.** The gizmo adapts to what's selected: doors hide the rotate handle, windows hide horizontal+corner handles (locked to wall axis), attached objects rotate with their host wall.
5. **Constant micro-feedback.** Every mode change, snap toggle, and label toggle fires a toast notification ("Click on wall to add door. Right click to change orientation"), and the Help button shows an animated GIF for the *currently active tool*.

Arcada's internal model is thin (nodes + walls + furniture items; no rooms/dimensions/site). KaLara's model is far richer — the goal is **Arcada's interaction quality on top of KaLara's architecture**.

---

## PART 2 — Arcada GUI Component Inventory → Qt Mapping

| # | Arcada component | What it does | KaLara/Qt equivalent |
|---|---|---|---|
| 1 | **ToolNavbar** (70px left icon rail) | Vertical icon strip, grouped: Add / Modes / Floors / Tools / File | `QToolBar` (Qt::LeftToolBarArea) with `QActionGroup` (exclusive checkable actions) — better than a dock for a tool rail |
| 2 | **AddMenu** ("+" button, hover-expand) | Sub-menu: Add furniture / Draw wall / Add window / Add door | `QToolButton` with `QMenu` (InstantPopup) on the toolbar |
| 3 | **FurnitureAddPanel** (right drawer) | Category dropdown + 2-column card grid with item images; click card → item spawns on canvas | Upgrade existing Library dock: add icon/thumbnail grid, keep search box, keep category selector |
| 4 | **Mode buttons** (View / Edit / Erase) | Exclusive tool modes | Checkable `QAction`s in exclusive `QActionGroup`; cursor changes per mode |
| 5 | **Floor controls** | Current floor indicator, up/down floor, delete floor | Compact widget in toolbar or Levels dock header (⬆ floor ⬇) |
| 6 | **Measure tool** | Click-drag → preview line + live length label | New `ViewportInteractionMode::Measure` in ViewportWidget |
| 7 | **Snap toggle** | Grid snap on/off, toast feedback | Toolbar toggle action + settings for spacing |
| 8 | **Toggle size labels** | Show/hide furniture W×H labels on canvas | View menu toggle + per-category visibility |
| 9 | **HelpDialog** | Context help with animated GIF per active tool | `QDialog` with per-tool help pages (static images acceptable; GIF via QMovie) |
| 10 | **WelcomeModal** | First-run guidance | `QDialog` on first launch (show once, store flag in QSettings) |
| 11 | **ConfirmationModal** | Confirm destructive ops (delete floor) | `QMessageBox::question` — use for Delete Floor / overwrite |
| 12 | **Notifications** (Mantine toasts) | Non-blocking feedback for every mode/action | `QStatusBar` message (timed) or lightweight toast overlay widget |
| 13 | **Live preview + size Label** (Pixi Graphics + Label) | Rubber-band line, wall length, furniture W/H | Painted in `paintGL()` (dashed line + text); world-mm math |
| 14 | **TransformLayer gizmo** | Border + handles: Rotate, H-resize, V-resize, HV-resize, Move + live labels | New `GizmoLayer` class in editor, rendered in `paintGL()`, hit-tested in mouse events |
| 15 | **WallNode handles** | Draggable dark squares at wall endpoints/joins | Rendered node grips in viewport; drag → wall endpoint mutation via transaction |
| 16 | **EditorStore (Zustand)** | `{ mode, floor, activeTool, snap }` singleton | Small `EditorState` singleton (already have ViewportState; add tool/floor/snap) |

---

## PART 3 — Core Interaction Patterns to Adopt (detailed specs)

### 3.1 Tool system
- Add `ViewportInteractionMode` entries: `Select` (default, combines Arcada View+Edit), `DrawWall`, `AddDoor`, `AddWindow`, `Measure`, `PlaceLibraryItem`, plus keep marquee/rubberband inside Select mode.
- One exclusive mode at a time. Switching modes cancels any in-progress operation (Arcada does `resetTools()` on every `setTool` — copy this).
- Every mode change → status-bar/toast hint: *"Wall drawing mode — click to draw, double-click to end."*

### 3.2 Wall chain drawing (the #1 missing feature)
Spec copied from `AddWallManager`:
1. First click sets the start node; a preview line + live length label (in mm, e.g. `3 450 mm`) follows the cursor.
2. Each subsequent click commits one wall segment between previous node and click point; the chain continues.
3. **Double-click ends the chain.** Esc cancels. Clicking an existing wall node connects to it (closed loops).
4. Validation: reject clicks closer than ~300 mm to the previous node (Arcada uses 0.3 m) and to existing nodes when starting.
5. Shift = orthogonal lock (already in KaLara) — preview snaps to 0/90/180/270°.
6. Every committed segment = one transaction entry (or one transaction per chain — pick one, document it, make Undo behave accordingly).
7. While drawing: new walls snap to existing endpoints (reuse SnappingEngine glyphs — green square etc. already exist).

### 3.3 Wall node editing
- In Select mode, wall endpoints/joins render as small square grips (~8 px screen size, constant under zoom).
- Drag a grip → live wall polygon redraw (preview during drag), commit on release via transaction.
- A grip connected to 2+ walls moves all connected walls (this is the join-propagation behavior — the risky one; enforce the invariants from AGENTS.md Rule 35: joined endpoints must coincide within 1 mm).
- Grips render larger on touch devices (Arcada doubles the size on mobile — cheap win).

### 3.4 Furniture/library transform gizmo (TransformLayer equivalent)
Handles on selected object (screen-space size, ~10 px):
- **Move** (center): drag anywhere inside.
- **H-resize** (E/W edge midpoint), **V-resize** (N/S midpoint), **HV-resize** (corners).
- **Rotate** (handle floating above the top edge).
- Live W×H labels on the border edges, counter-rotated so they stay readable (Arcada rotates labels by `-target.angle`).
- **Context-sensitive visibility** (copy exactly):
  - Door: no rotate handle (orientation comes from host wall).
  - Window: no H / HV / rotate handles (width along wall is the only free axis).
  - Wall-hosted objects move along the host wall axis only.
- Gizmo = editor-only overlay. Never persisted, never enters the model.

### 3.5 Doors / windows placement
- AddDoor / AddWindow mode: click on a wall → opening spawns centered on click point, wall is cut live.
- **Right-click flips orientation** (hinge side / opening direction). Toast: *"Right click to change orientation."*
- Door renders leaf + swing arc; window renders cutout + frame (already in KaLara's renderer).

### 3.6 Measure tool
- Click-drag → dashed preview line + live length label (mm). Release ends it. Pure overlay, no model change, no transaction.
- Snap to endpoints/midpoints using the existing SnappingEngine.

### 3.7 Feedback systems (cheap, high value)
- Toast/status feedback on: every tool switch, snap toggle, grid toggle, label toggle, floor switch, save, export.
- Context Help dialog (F1 / `?` toolbar button) showing a short how-to **for the active tool**.
- First-run Welcome modal: "New project / Open project / Quick tour" (this is also the natural home for the future Template wizard).

---

## PART 4 — Gap Analysis: KaLara Today vs the Arcada Standard

### Already good in KaLara (keep — Arcada doesn't have these)
Structured model, rooms + auto area, associative dimensions, site/setback panel, validation engine, DXF/SVG/PDF/JSON export, transactions, multi-level with real elevations + underlay, AIA layers.

### Missing entirely (the authoring layer) — CRITICAL
| Gap | Arcada reference | Priority |
|---|---|---|
| Draw-wall tool (chain drawing) | §3.2 | P0 |
| Wall node grips + drag-to-edit | §3.3 | P0 |
| Door/window placement mode + flip | §3.5 | P0 |
| Transform gizmo (move/resize/rotate) for furniture | §3.4 | P0 |
| Measure tool | §3.6 | P1 |
| Tool rail / tool modes in UI | Part 2 #1–4 | P0 |
| Live preview + running dimension label | §3.2, §3.6 | P0 |
| Toast/hint feedback system | Part 2 #12 | P1 |
| Context help + welcome modal | Part 2 #9–11 | P2 |

### Present but weaker than Arcada — MAJOR
| Gap | Notes |
|---|---|
| No property inspector | Arcada edits directly on canvas; KaLara needs *both* gizmos and an inspector (wall thickness/length, room name/type, door width) |
| No copy/duplicate (Ctrl+C/V/D) | Daily-essential |
| Rotation only 90° CCW | Need arbitrary angle + mirror/flip |
| No zoom-to-fit / zoom-to-selection | Standard expectation |
| No category visibility toggles (layers) | Arcada's "toggle size labels" is the seed of this |
| Grid/snap not user-configurable | Arcada has snap toggle; add spacing settings |
| No image export (PNG/JPG) | For sharing; trivial vs PDF |
| Library = text list | Arcada uses visual cards with thumbnails |
| Menu bar only (File/Edit) | Tool rail + Draw/Insert menus needed |

---

## PART 5 — What to ADD / CHANGE (implementation notes for Qt/C++)

### 5.1 New UI components
1. **Tool rail** (`editor/src/tool_rail.cpp`): `QToolBar` left area; checkable exclusive actions; groups: [Add ▾] [Select] [Measure] | floor stepper | [snap toggle] [labels toggle] | [help]. Status tip + toast on activate.
2. **GizmoLayer** (`editor/src/gizmo_layer.cpp/hpp`): renders selection border, 5 handles, live W/H labels in `paintGL()`; hit-test in `mousePressEvent`; maps drag to model mutations through TransactionManager. Handles are screen-space constant size.
3. **PreviewLayer**: dashed rubber line + length label while drawing/measuring; cleared on mode switch/Esc.
4. **Inspector dock** (right area, new tab): numeric editors (QDoubleSpinBox in mm or per display unit) bound to selection — wall (start/end, thickness, type), room (name, type), opening (width, height, sill, flip), furniture (W/D, rotation). Every commit = transaction.
5. **Toast widget**: borderless translucent overlay, 2.5 s auto-hide (or `QStatusBar::showMessage(msg, 2500)` — good enough).
6. **Help/Welcome dialogs** (`editor/src/help_dialog.cpp`): per-tool text + screenshot; welcome shows once (QSettings).

### 5.2 Changes to existing code
- `viewport_widget.hpp`: extend `ViewportInteractionMode` with `DrawWall, AddDoor, AddWindow, Measure`; add `Esc` → cancel in-progress op + deselect.
- `viewport_widget.cpp`: route mouse events by mode; render PreviewLayer + GizmoLayer in `paintGL()`; right-click in AddDoor/AddWindow → flip (currently right-click may be unused or pan — decide explicitly and document).
- Add `EditState` singleton (tool, snapEnabled, gridSpacing, labelsVisible) — mirrors Arcada's Zustand store.
- Library dock: add thumbnail grid mode (icons from `library/` data; render symbols to PNG at build time via a small tool if needed).
- Cursor per mode: crosshair for DrawWall/Measure, arrow for Select, forbidden for Erase-over-locked.
- Confirmation `QMessageBox` before Delete Floor and before closing with unsaved changes (track dirty flag on TransactionManager).

### 5.3 New model/API operations (via choke point only)
- `splitWallAt(point)` / `mergeWallNodes` (needed for node-level editing)
- `setOpeningFlipped(id)` (hinge side)
- `duplicateEntities(ids, offset)` (copy/paste)
- `rotateEntities(ids, angleDeg, pivot)` — arbitrary angle
- `mirrorEntities(ids, axis, pivot)`

---

## PART 6 — New Functionalities Beyond Arcada (differentiators)

Arcada has no concept of rooms-as-data, dimensions, or validation — KaLara already beats it there. Recommended additions neither app has but fit KaLara's roadmap:

1. **Room drawing tool**: click inside closed walls → room; or trace polygon → explicit room. (Auto-detect exists; make it user-invoked + manual fallback.)
2. **Dimension placement tool**: click two points → associative dimension; auto-dimension room on request.
3. **Layers / visibility panel**: per-category + per-AIA-layer visibility, saved per project.
4. **Grid & snap settings dialog**: spacing, subdivisions, snap radius, which snap types are active (matches Rule 12).
5. **Template / New Project wizard**: Empty Canvas vs Templates (the already-documented Step 41A) — hook into WelcomeModal.
6. **Autosave + recovery**: timed autosave to `%TEMP%/kalara_autosave.kla`, restore prompt after crash.
7. **Image export** (PNG/JPG with white background option, transparent option).
8. **Zoom to fit / zoom to selection** (`Ctrl+0` / `Ctrl+2`-style).
9. **Undo history panel**: list of transaction names, click to jump (HistoryManager exists — surface it).
10. **Align/distribute** for multi-selection (align left/center/right, distribute spacing).
11. **Keyboard shortcut editor** (defer — nice-to-have).
12. **Welcome/recent files screen** on startup (recent projects via QSettings).

---

## PART 7 — Suggested Roadmap (checkpoint-compatible with AGENTS.md)

| New step | Scope | Acceptance evidence |
|---|---|---|
| S20-A | Tool rail + tool modes + toasts + EditState | Each mode switches; hint shows; build passes |
| S20-B | **Draw-wall tool** (chain, preview, live label, Esc/double-click end, connect to existing nodes) | Draw L-shaped room programmatically + manually; measured lengths exact; Undo removes chain correctly |
| S20-C | Wall node grips + drag editing + split/merge | Invariants: joined endpoints coincide ≤1 mm after drag; room area updates; single Undo per drag |
| S20-D | Door/window placement + right-click flip + hosted constraints | Opening snaps onto wall; flip works; wall cutout correct; deleting wall removes openings (already true) |
| S20-E | Transform gizmo (move/H/V/HV/rotate + context visibility + live labels) | Furniture resized/rotated; door/window handle rules verified; measurements match label values |
| S20-F | Measure tool + zoom-fit + copy/duplicate + arbitrary rotation/mirror | Measured values exact vs known geometry |
| S20-G | Inspector dock + layers panel + grid/snap settings + image export | Edit wall thickness via inspector → model + viewport + dimension update; round-trip save/load |
| S20-H | Welcome/recent/templates entry + autosave + help dialog | Crash-recovery test; template creates structured project |

Each step: `git status` → implement → build → test (numerical where geometry) → checkpoint report → wait for CONTINUE.

---

## Appendix — Proposed Mouse/Key Map (aligned with existing KaLara shortcuts)

| Input | Mode | Action |
|---|---|---|
| LMB click | Select | Select entity (Shift/Ctrl adds) |
| LMB drag (empty) | Select | Marquee select |
| LMB drag (entity) | Select | Move (gizmo handles take precedence) |
| LMB click | DrawWall | Place wall node; chain continues |
| Double-click / Esc | DrawWall | End chain / cancel |
| Shift (hold) | DrawWall/Measure | Orthogonal lock |
| RMB | AddDoor/AddWindow | Flip orientation |
| Wheel | All | Zoom at cursor (existing) |
| MMB / Alt+LMB | All | Pan (existing) |
| Delete/Backspace | Select | Delete selection (existing) |
| Ctrl+C / Ctrl+V / Ctrl+D | Select | Copy / paste / duplicate |
| R / Shift+R | Select | Rotate 90° CW / CCW (keep R=CCW) |
| M | Global | Measure tool toggle |
| V / Esc | Global | Back to Select mode |
| Ctrl+0 / Ctrl+2 | Global | Zoom fit / zoom selection |
| F1 | Global | Context help for active tool |

---

*Source material: Arcada @ master (ToolNavbar, AddWallManager, TransformLayer, WallNode, MeasureToolManager, EditorStore, FurnitureAddPanel, HelpDialog) reviewed September 2026. All Arcada-derived behavior is re-specified here in mm and in KaLara's layer architecture; no code is copied.*
