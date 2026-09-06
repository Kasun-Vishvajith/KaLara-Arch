# KaLara Arch — Graphical User Interface (GUI) Comprehensive Manual & Architectural Reference

**Application Version:** 0.1.0 (Phases 00–19 Complete)  
**Target Environment:** Windows 10/11 x64  
**Primary Frameworks:** C++20, Qt 6 (QtWidgets, QtGui, QtCore, OpenGLWidgets)  
**Document Purpose:** Comprehensive functional manual, shortcut catalog, interaction guide, and developer reference for modifying or extending the KaLara Arch desktop interface.

---

# 1. High-Level UI Architecture & Technology Stack

KaLara Arch is an open-source, human-first architectural planning system. The graphical user interface is engineered according to **Principle 1** (Structured Architectural Model First): the internal project model is the authoritative source of truth, and the 2D viewport and dockable panels are reactive view-controllers.

```text
┌─────────────────────────────────────────────────────────────────────────┐
│                              MainWindow                                 │
│  (QMenuBar: File, Edit, Export | QStatusBar: Coordinates, Zoom, Hints)  │
├──────────────────┬───────────────────────────────────┬──────────────────┤
│   Dock Panel     │         Central Viewport          │   Dock Panel     │
│  (Left Area)     │        (QOpenGLWidget Canvas)     │  (Right Area)    │
│                  │                                   │                  │
│  Architectural   │   - World-to-Screen Transform     │  - Levels &      │
│  2D Library      │   - Real-Time 2D Rendering        │    Stories       │
│  Browser Widget  │   - Semantic Snapping Engine      │  - Site Planning │
│                  │   - Live Manipulation & Gizmos    │    & Setbacks    │
├──────────────────┴───────────────────────────────────┴──────────────────┤
│                          Dock Panel (Bottom Area)                       │
│             Diagnostics & Architectural Validation Engine               │
│               (Rule 16 Non-Professional Approval Notice)                │
└─────────────────────────────────────────────────────────────────────────┘
```

### Core Libraries & Subsystems Used:
- **`QtWidgets` (`QMainWindow`, `QDockWidget`, `QMenuBar`, `QStatusBar`, `QLabel`, `QPushButton`, `QTreeWidget`, `QSpinBox`, etc.)**: Provides the native desktop window shell, docking infrastructure, menu bar commands, and property editors.
- **`QOpenGLWidget` / `QtGui` (`QPainter`, `QPen`, `QBrush`, `QPolygonF`, `QFont`, `QColor`)**: Powers hardware-accelerated 2D vector rendering for the architectural canvas, ensuring smooth anti-aliased linework, real-time panning, and responsive zooming.
- **`QtCore` (`QObject`, Signals & Slots, `QString`, `QPoint`, `QRectF`)**: Facilitates loose coupling between the viewport, transaction manager, and docking widgets.
- **`QPdfWriter` / `QPageLayout` / `QPageSize`**: Produces publication-grade PDF documentation sheets with precise architectural drawing scales (1:50, 1:100, 1:200).
- **Standard C++20 Library**: Handles all underlying geometry math, vector transformations, memory management, and file I/O.

---

# 2. Viewport Canvas: Visual Grammar & "Connected Dots"

When you open KaLara Arch, the 2D viewport presents a clear, layered vector floor plan. Every visual element has a precise semantic meaning:

```text
       ┌────────────────────── North Wall (5000 mm) ─────────────────────┐
       │   ======================= [Window] ========================   │
       │   │                                                       │   │
West   │   │                                                       │   │ East
Wall   │   │                     Living Room                       │   │ Wall
(4000) │   │                   (Area: 20.0 m²)                     │   │ (4000)
       │   │                                                       │   │
       │   │                                                       │   │
       │   ======================== [Door] =========================   │
       └─────●───────────────────────/───────)─────────────────────●───┘
          Join Node               Door Leaf  Swing Arc          Join Node
             │
             └─► Associative Dimension Line: |<─────── 5000 mm ───────>|
```

### 1. Wall Linework & Thickness
- **Outer Filled Polygons (`A-WALL`)**: Walls are not merely single lines; they are volumetric 2D architectural elements with real thickness (default: `200 mm`). The system renders solid dark-grey fills (`#282c34`) with crisp black contour boundaries.
- **Wall Centerline (`A-WALL-CNTR`)**: The internal reference baseline connecting the wall's start and end coordinates.

### 2. Join Nodes (The "Connected Dots")
- When walls connect at corners (L-joins), intersections (T-joins), or cross-junctions, their endpoints coincide at exact mathematical coordinates.
- **Rule 4 & Principle 4**: Calculations are preserved with 1 mm nominal architectural precision. Join nodes maintain structural continuity so that room detection algorithms can traverse closed loops.

### 3. Openings (Doors & Windows)
- **Doors (`A-DOOR`)**: Drawn as a door leaf outline at a 90° open angle accompanied by a circular swing arc indicating ingress/egress clearance.
- **Windows (`A-GLAZ`)**: Drawn as an opening cutout through the host wall with frame lines and a distinct centerline representing glazing glass.

### 4. Rooms / Spaces (`A-AREA`)
- Automatically detected or explicitly defined room boundaries are filled with a subtle, translucent surface tint.
- A centered room label displays the **Room Name** (e.g., *Living Room*) and the exact live **Calculated Area** (e.g., *20.0 m²*).

### 5. Associative Dimensions (`A-DIMS`)
- Dimension lines run parallel to walls at a configurable offset (default: `-600 mm`).
- Feature extension lines, tick slashes, and clear millimetre distance labels (`5000 mm`).
- **Rule 7 (Dynamic Associativity)**: If a wall is moved, lengthened, or shortened, its associated dimension automatically updates its measurement and geometry.

### 6. Site Planning Context
- **Property Boundary (`C-PROP`)**: Displayed as an outer golden dashed line polygon (`#e6be50`).
- **Buildable Setback Envelope (`C-SETB`)**: Displayed as an inner orange dash-dot boundary (`#e66e46`) representing the legally buildable zone after subtracting front, rear, and side setbacks.
- **Road Context (`C-ROAD`)**: Pavement boundaries with curb offsets and centerlines.
- **Parking Zones (`C-SITE`)**: Outlined parking bays with stall labels and wheelchair accessibility icons (♿).
- **Outdoor Elements & Pools (`C-SITE`)**: Swimming pools with paved coping deck surrounds, deep blue water fills, water ripple line accents, and live computed area and water volume in liters.

---

# 3. Viewport Navigation & Coordinate HUD

| Action | Input / Gesture | Description |
| :--- | :--- | :--- |
| **Pan Canvas** | **Middle Mouse Button (MMB) Drag**<br>— or —<br>**Alt + Left Mouse Button (LMB) Drag** | Smoothly shifts the 2D viewport across the infinite virtual canvas. |
| **Zoom In / Out** | **Mouse Wheel Scroll** | Dynamically zooms in or out, maintaining the exact canvas point under the cursor as the focal center. |
| **Live Coordinates** | **Mouse Movement** | Bottom status bar permanently displays canonical millimetre coordinates: `X: ... mm \| Y: ... mm`. |
| **Zoom Indicator** | **Status Bar HUD** | Shows real-time magnification percentage (`Zoom: 100%`, `Zoom: 150%`, etc.). |

---

# 4. Selection, Moving, Rotating, and Deleting

### Selection Modes
1. **Single Selection**:
   - Left-click on any wall, room, door, window, furniture instance, or roof.
   - The selected entity highlights with an electric-cyan border.
   - The status bar reports the selection breakdown: `Selected: 1 entities (Walls: 1, Rooms: 0, ...)`.
2. **Multi-Selection**:
   - Hold **`Shift`** or **`Ctrl`** while clicking entities to add them to or remove them from the active selection set.
3. **Rubberband / Marquee Box Selection**:
   - Left-click on empty canvas space and drag a selection rectangle.
   - Releasing the mouse selects all entities bounded within the marquee box.

### Manipulation & Transformation
1. **Interactive Live Drag-Move**:
   - Click and drag directly on any selected entity. The selection moves smoothly under your cursor in real time.
2. **Precision 100 mm Nudging (Arrow Keys)**:
   - With entities selected, press the keyboard arrow keys:
     - **`Left Arrow`**: Shifts selection **`-100 mm`** along the X-axis.
     - **`Right Arrow`**: Shifts selection **`+100 mm`** along the X-axis.
     - **`Up Arrow`**: Shifts selection **`+100 mm`** along the Y-axis.
     - **`Down Arrow`**: Shifts selection **`-100 mm`** along the Y-axis.
3. **90° Rotation (`R` Key)**:
   - Press **`R`** on the keyboard.
   - All selected entities rotate **90° counter-clockwise** around their shared bounding box centroid.
4. **Deleting Entities (`Delete` / `Backspace`)**:
   - Select any entity (or group of entities) and press the **`Delete`** or **`Backspace`** key.
   - Walls, rooms, doors, windows, furniture instances, and annotations are instantly removed. Deleting a wall automatically cleans up its hosted doors, windows, and attached dimensions.

---

# 5. Semantic Snapping & Orthogonal Mode

KaLara Arch treats snapping as an architectural relationship, not a crude pixel grid effect (**Rule 12**).

```text
Snapping Glyphs:
┌──────────────┬────────────────────────────┬─────────────────────────────┐
│ Glyph        │ Meaning                    │ Snapping Target             │
├──────────────┼────────────────────────────┼─────────────────────────────┤
│ Green Square │ Endpoint Snap              │ Wall corners, ends, joints  │
│ Blue Triangle│ Midpoint Snap              │ Exact center of wall span   │
│ Lime Cross   │ Wall Centerline Snap       │ Longitudinal axis of a wall │
│ White Circle │ Grid Snap                  │ Background coordinate grid  │
└──────────────┴────────────────────────────┴─────────────────────────────┘
```

- **Snap Radius**: Automatically captures cursor targets within **`150 mm`** in world coordinates.
- **Orthogonal Lock Mode (`Shift` Key)**:
  - Holding **`Shift`** locks cursor movement strictly to 0°, 90°, 180°, or 270° relative to the origin or anchor point. Perfect for drafting rectilinear architectural plans.

---

# 6. Dockable Panels Reference

### 1. Architectural 2D Library Browser (Left Dock)
- **Library Content**: Organized by category:
  - *Furniture*: Double beds, 3-seat sofas, armchairs, 6-seat dining tables, office desks.
  - *Bathroom*: Standard toilets, vanity sinks, bathtubs, walk-in showers.
  - *Kitchen*: Sink counters, cooktops, refrigerators.
  - *Openings*: Standard hinged doors, sliding double glass doors, sash windows.
  - *Site*: Standard vehicle parking bays, accessible bays, deciduous/evergreen trees.
- **Clearance Envelopes**: Library items know their functional clearance (e.g., chair pull-out zone around a dining table, leg space in front of a toilet).
- **Placing an Item**: Click any item in the library list. It instantly instantiates at your current mouse cursor position on the canvas, pre-selected and ready for rotation or fine adjustment.

### 2. Levels & Stories Manager (Right Dock, Tab 1)
- **Multi-Level Hierarchy (Principle 5 & Step 12)**:
  - Manages building floors with real elevation data (e.g., *Ground Floor: 0 mm*, *First Floor: 3200 mm*, *Roof Level: 6400 mm*).
- **Active Floor Switching**: Selecting a level in the tree view switches the active editing canvas to that specific level.
- **Underlay / Ghost Reference**: Check the Underlay option to project a faint silhouette of the floor below, allowing perfect vertical alignment of plumbing stacks, stairwells, and load-bearing walls.

### 3. Site Planning & Setbacks Panel (Right Dock, Tab 2)
- **Property Geometry**: Defines property boundary coordinates, total land area in $m^2$, and perimeter.
- **Setback Envelopes**: Configure Front, Rear, Left, and Right setbacks in millimetres. The system re-computes the buildable envelope in real time.
- **Site Elements**: Configure street frontages, parking stall allocations, and swimming pools with live volume estimations.

### 4. Diagnostics & Validation Engine (Bottom Dock)
- **Live Rule Verification (Step 14)**:
  - Inspects geometry for unclosed room boundaries, overlapping opening clearances, disconnected wall endpoints, and building setback violations.
- **Interactive Issue Highlighting**: Clicking any issue in the table automatically selects and frames the offending entities in the 2D viewport.
- **Rule 16 Non-Professional Approval Disclaimer**:
  - Permanently displays the mandatory legal banner:
    > *"KaLara Arch automated validation is provided for design assistance and geometric/constraint checking only. It does not constitute or replace required professional architectural, structural, building code, or authority approval."*

---

# 7. File Operations & Native Format (`.kla`)

### 1. Saving Projects (`Ctrl+S` / `Ctrl+Shift+S`)
- Files are saved with the native **`.kla`** extension (e.g., `residence.kla`).
- **Data Integrity (Rule 4)**: The `.kla` file is a structured, human-readable JSON schema that stores the entire project hierarchy:
  - Project metadata, units, and tolerances
  - Site boundary, setbacks, roads, parking, and outdoor elements
  - Buildings, levels, and real elevation datum
  - Parametric walls, join references, and thickness preferences
  - Doors, windows, and opening swing directions
  - Rooms, calculated boundaries, and area names
  - Associative dimensions and annotations
  - Library instances and clearance envelopes
- Supports **100% round-trip semantic equivalence**: saving and reloading reproduces the exact model state without numerical drift.

### 2. Opening Projects (`Ctrl+O`)
- Restores project state, re-populates all docking managers, and frames the active floor plan in the 2D viewport.

### 3. History & Undo / Redo (`Ctrl+Z` / `Ctrl+Y`)
- Every human action (placing furniture, moving walls, modifying setbacks) is wrapped in an atomic **Transaction** (**Step 16**).
- Full before/after snapshots allow infinite undo and redo without corrupted intermediate states.

---

# 8. Professional CAD & Multi-Format Exports

KaLara Arch treats export as a core architectural deliverable (**Principle 7**), translating the canonical model into industry-standard formats:

```text
KaLara Arch Canonical Model (1 mm = 1.0)
     ├── File -> Export -> Export DXF (AutoCAD)...      [Ctrl+E] -> AutoCAD / SketchUp
     ├── File -> Export -> Export PDF (Documentation)... [Ctrl+P] -> Scaled Print Sheets
     ├── File -> Export -> Export SVG (Vector)...                -> Scalable Web Graphics
     └── File -> Export -> Export Structured JSON...             -> AI Agents & BIM Tools
```

### 1. AutoCAD DXF Export (`Ctrl+E`)
- **Unit Precision**: Inserts AutoCAD header variable `$INSUNITS = 4` (Decimal Millimetres) and `$MEASUREMENT = 1` (Metric).
- **AIA / NCS Standard Architectural CAD Layers**:
  - `A-WALL`: Wall boundary outlines (closed 2D polylines).
  - `A-WALL-CNTR`: Wall centerlines.
  - `A-DOOR`: Door leafs and circular swing arcs.
  - `A-GLAZ`: Window opening cutouts and glazing centerlines.
  - `A-AREA`: Room boundary polylines, room name tags, and area text ($m^2$).
  - `A-FLOR-FIXT`: Furniture and sanitary fixture footprints.
  - `A-DIMS`: Linear associative dimensions with extension lines and text.
  - `A-ANNO-TEXT`: Notes and leader annotations.
  - `C-PROP`, `C-SETB`, `C-ROAD`, `C-SITE`: Property boundary, setbacks, roads, and outdoor structures.
- **Workflow Compatibility**:
  - **AutoCAD / LibreCAD / QCAD**: Opens directly with 1:1 scale. Measuring a 5000 mm wall returns exactly `5000.0000`.
  - **Trimble SketchUp**: Import with units set to *Millimeters*. Wall outlines on `A-WALL` are closed loops that can be immediately extruded upward into 3D walls using the Push/Pull tool (`P`).
  - **Blender 3D**: Imports seamlessly using a 0.001 scale factor ($1000\text{ mm} = 1\text{ m}$).

### 2. PDF Documentation Sheet Export (`Ctrl+P`)
- Produces printable architectural documentation sheets.
- Supports standard paper sizes (ISO A4, A3, A2, A1) in Landscape or Portrait.
- Scales drawing output to professional architectural ratios (**1:50, 1:100, 1:200**).
- Embeds standardized title block, north arrow indicator, and scale ratio labels.

### 3. SVG Vector Graphic Export
- Outputs clean SVG markup with CSS classes (`class="wall"`, `class="room"`, `class="dimension"`).
- Suitable for high-resolution presentations, web portfolios, and technical reports.

### 4. Structured JSON Interchange Export
- Exports a complete, machine-readable JSON representation of the building data tree, purpose-built for external scripts and AI agents.

---

# 9. Master Keyboard Shortcuts & Mouse Gestures Table

| Shortcut / Gesture | Context | Action / Result |
| :--- | :--- | :--- |
| **`Middle Mouse Drag`** | Viewport | Pan canvas smoothly |
| **`Alt + Left Click Drag`** | Viewport | Pan canvas smoothly (trackpad alternative) |
| **`Mouse Wheel Scroll`** | Viewport | Zoom in / out centered on mouse cursor |
| **`Left Click`** | Viewport | Select individual entity (Wall, Room, Door, Window, Furniture) |
| **`Shift + Left Click`** | Viewport | Add/remove entity from multi-selection set |
| **`Ctrl + Left Click`** | Viewport | Add/remove entity from multi-selection set |
| **`Left Click Drag (Empty Space)`** | Viewport | Draw Marquee / Rubberband Box to multi-select entities |
| **`Left Click Drag (On Entity)`** | Viewport | Interactive live drag-move of selected entities |
| **`Arrow Keys (Left/Right/Up/Down)`** | Selection | Nudge selected entities by exactly **100 mm** |
| **`R`** | Selection | Rotate selected entities **90° counter-clockwise** |
| **`Delete`** / **`Backspace`** | Selection | Delete selected entities from the active level |
| **`Shift` (Hold)** | Viewport | Lock cursor to strict **Orthogonal Axes** (0°, 90°, 180°, 270°) |
| **`Ctrl + N`** | Application | Create a **New Project** |
| **`Ctrl + O`** | Application | **Open Project** (`.kla` file dialog) |
| **`Ctrl + S`** | Application | **Save Project** (`.kla` native file) |
| **`Ctrl + Shift + S`** | Application | **Save Project As** |
| **`Ctrl + Z`** | Application | **Undo** last operation |
| **`Ctrl + Y`** / **`Ctrl + Shift + Z`** | Application | **Redo** last undone operation |
| **`Ctrl + E`** | Application | **Export AutoCAD DXF** |
| **`Ctrl + P`** | Application | **Export PDF Documentation Sheet** |
| **`Ctrl + Q`** | Application | Exit application |

---

# 10. Developer Guide: How to Extend or Modify the GUI

If you plan to modify or add new features to the KaLara Arch GUI, follow these architectural conventions:

```text
                       ┌────────────────────────┐
                       │       MainWindow       │
                       └───────────┬────────────┘
                                   │ Owns & Coordinates
           ┌───────────────────────┼───────────────────────┐
           ▼                       ▼                       ▼
┌────────────────────┐   ┌───────────────────┐   ┌────────────────────┐
│   ViewportWidget   │   │ Docking Panels    │   │ TransactionManager │
│   (QOpenGLWidget)  │   │ (Level, Library,  │   │ (Undo/Redo Stack)  │
│                    │   │  Site, Validation)│   │                    │
└──────────┬─────────┘   └───────────────────┘   └────────────────────┘
           │ Signals: cursorCoordinatesChanged, selectionChanged
           ▼
┌────────────────────┐
│   Architectural    │
│    Project Model   │  ◄── Controlled Mutation Choke Point (Rule 3)
└────────────────────┘
```

### 1. Controlled Mutation Choke Point (Rule 3)
- **NEVER** write ad-hoc state mutations directly inside UI event handlers.
- Always wrap model edits in a `Transaction`:
  ```cpp
  m_transactionManager.beginTransaction(*m_project, "Add New Wall");
  // Perform level mutation: level->addWall(...);
  m_transactionManager.recordOperation("Created Wall");
  m_transactionManager.commitTransaction(*m_project);
  updateUndoRedoActions();
  m_viewport->update();
  ```

### 2. World Coordinates vs Screen Pixels (`ViewportState`)
- All architectural geometry is stored in **canonical millimetres (`mm`)**.
- The viewport uses [`ViewportState`](file:///c:/Users/kasun/Projects/Game%20Dev/KaLara/KaLara%20Arch/editor/include/kalara/editor/viewport_state.hpp):
  ```cpp
  Point2D worldPt = m_state.screenToWorld(mouseX, mouseY);
  QPointF screenPt = m_state.worldToScreen(worldPt);
  ```
- Always perform snapping, collision testing, and dimensioning in **world millimetres**, never in display pixels.

### 3. Adding a New Tool or Viewport Interaction Mode
1. In [`editor/include/kalara/editor/viewport_widget.hpp`](file:///c:/Users/kasun/Projects/Game%20Dev/KaLara/KaLara%20Arch/editor/include/kalara/editor/viewport_widget.hpp), add your interaction mode to `ViewportInteractionMode` (e.g., `DrawWall`, `PlaceDimension`).
2. Implement mouse handlers (`mousePressEvent`, `mouseMoveEvent`, `mouseReleaseEvent`) for that mode.
3. In `paintGL()`, render preview geometry (e.g., temporary dashed lines or dimension strings) before the operation is committed.
4. Add a `QAction` to `MainWindow`'s menu or toolbar to activate the tool.

### 4. Adding a New Dockable Panel
1. Create a `QWidget` subclass in `editor/include/kalara/editor/` and `editor/src/`.
2. In `MainWindow::setupUI()`, wrap it in a `QDockWidget`:
   ```cpp
   auto* dock = new QDockWidget("My Custom Panel", this);
   auto* customWidget = new MyCustomWidget(m_project.get(), dock);
   dock->setWidget(customWidget);
   addDockWidget(Qt::RightDockWidgetArea, dock);
   ```
3. Connect custom widget signals to `m_viewport->update()` or transaction handlers.
