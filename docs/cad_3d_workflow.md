# KaLara Arch — CAD & 3D Workflow Validation Guide

## 1. Overview & Architectural Principles

KaLara Arch is an open-source, human-first architectural planning system built on canonical millimetre precision (**Principle 3** and **Principle 4**). 

Export is a first-class product feature (**Principle 7**). KaLara Arch produces real-scale, structured exports that seamlessly transition into downstream professional documentation and 3D modeling tools:

```text
┌────────────────────────────────────────┐
│              KaLara Arch               │
│      Canonical Model (1 mm = 1.0)      │
└──────────────────┬─────────────────────┘
                   │
         ┌─────────┴─────────┐
         │                   │
         ▼                   ▼
    AutoCAD DXF             SVG
    ($INSUNITS = 4)   (Responsive Vector)
         │                   │
   ┌─────┴──────────┐        │
   │                │        │
   ▼                ▼        ▼
AutoCAD /       Trimble   Blender 3D
LibreCAD /      SketchUp  (Scale: 0.001 m/mm)
QCAD
```

---

## 2. Universal Unit Mapping & Standards

| Target System | Native Unit | DXF Unit Code (`$INSUNITS`) | Import Scale Factor | Verified Precision |
| :--- | :--- | :--- | :--- | :--- |
| **KaLara Arch** | Millimetre (`mm`) | `4` (Millimeters) | `1.0` | $\pm 0.001\text{ mm}$ |
| **AutoCAD / LibreCAD** | Millimetre (`mm`) | `4` (Millimeters) | `1.0` | Exact 1:1 |
| **Trimble SketchUp** | Millimetre (`mm`) | `4` (Millimeters) | `1.0` | Exact 1:1 |
| **Blender** | Metre (`m`) | `4` (Millimeters) | `0.001` ($1000\text{ mm} = 1.0\text{ m}$) | $\pm 0.000001\text{ m}$ |

### Standard Architectural Layers (AIA / NCS Compliant)
- `A-WALL`: Wall thickness boundary outlines (closed 2D polygons ready for 3D extrusion)
- `A-WALL-CNTR`: Wall reference centerlines
- `A-DOOR`: Door leaf outlines and circular swing arcs
- `A-GLAZ`: Window frame profiles and glass centerlines
- `A-AREA`: Room boundary polylines, space names, and computed areas ($m^2$)
- `A-FLOR-FIXT`: Furniture and fixture footprints
- `A-FLOR-STRS`: Stair steps and direction arrows
- `A-ROOF`: Roof outlines and ridges
- `A-DIMS`: Linear associative dimensions with extension lines, slash ticks, and measurement text
- `A-ANNO-TEXT`: Notes and spot elevations
- `C-PROP`: Property boundary perimeter
- `C-SETB`: Setback buildable envelope
- `C-ROAD`: Road context and centerlines
- `C-SITE`: Site parking, landscaping, and outdoor features

---

## 3. Workflow 1: KaLara Arch → AutoCAD / LibreCAD / QCAD

### Step-by-Step Procedure
1. In KaLara Arch, select **File $\rightarrow$ Export $\rightarrow$ Export DXF (AutoCAD)...** (`Ctrl+E`).
2. Open AutoCAD, LibreCAD, or QCAD.
3. Open or insert the exported `.dxf` file:
   - **AutoCAD**: `OPEN` $\rightarrow$ select `.dxf`. The header `$INSUNITS = 4` automatically sets drawing units to Decimal Millimetres.
   - **LibreCAD / QCAD**: `File $\rightarrow$ Open` $\rightarrow$ select `.dxf`.
4. Type `ZOOM` $\rightarrow$ `ALL` (or `EXTENTS`) to frame the exported floor plan.
5. In the Layer Properties Manager, verify that all architectural layers (`A-WALL`, `A-DOOR`, `A-GLAZ`, `A-DIMS`, etc.) appear with assigned ACI colors.

### Verification Checklist
- [x] Measuring a 5000 mm wall with the `DIST` command returns exactly `5000.0000`.
- [x] Wall thickness measures exactly `200.0000 mm` (or configured thickness).
- [x] Dimension text displays exact real-world millimetre values matching geometric endpoints.
- [x] Layers can be frozen/thawed independently (e.g., hiding dimensions or furniture for clean structural drafting).

---

## 4. Workflow 2: KaLara Arch → Trimble SketchUp

### Step-by-Step Procedure
1. In KaLara Arch, export your floor plan via **File $\rightarrow$ Export $\rightarrow$ Export DXF (AutoCAD)...**.
2. Launch Trimble SketchUp (Pro / Studio).
3. Select **File $\rightarrow$ Import...**.
4. In the file dialog, set format filter to **AutoCAD Files (*.dwg, *.dxf)**.
5. Click **Options** before opening:
   - **Units**: Select **Millimeters** (do NOT select Model Units or Inches).
   - Check **Preserve drawing origin**.
   - Check **Merge coplanar faces**.
   - Click **OK**, then click **Import**.
6. SketchUp imports the drawing as a group:
   - Right-click the imported group $\rightarrow$ **Explode**.
   - Because KaLara Arch exports wall boundaries as **strictly closed, planar 2D polylines** (`A-WALL`), SketchUp automatically recognizes closed boundary loops.
7. Use the **Push/Pull Tool (`P`)**:
   - Click inside any wall profile.
   - Pull upwards and type `3000` (or your wall height in mm) $\rightarrow$ press **Enter**.
   - Double-click other wall sections to repeat the extrusion instantly.
8. Window and door openings on `A-DOOR` and `A-GLAZ` align with wall openings for cutting openings or placing 3D components.

### Verification Checklist
- [x] Import scale is 1:1; a 5000 mm wall measures 5000 mm using SketchUp's Tape Measure Tool (`T`).
- [x] Wall outlines form closed faces without needing to re-trace lines.
- [x] Levels match their configured elevations when exporting multi-level projects.

---

## 5. Workflow 3: KaLara Arch → Blender 3D

### Method A: DXF Import
1. In Blender, enable the built-in add-on: **Edit $\rightarrow$ Preferences $\rightarrow$ Add-ons $\rightarrow$ Import-Export: AutoCAD DXF Format (.dxf)**.
2. Select **File $\rightarrow$ Import $\rightarrow$ AutoCAD DXF (.dxf)**.
3. In the import settings sidebar:
   - Set **Scale** to `0.001` (to convert millimetres to Blender standard metric metres: $1000\text{ mm} \rightarrow 1.0\text{ m}$).
   - Set **Merge Vertices** to `Enabled` (threshold `0.0001`).
4. Press **Import DXF**.
5. Select the imported `A-WALL` curves or mesh:
   - Switch to **Edit Mode (`Tab`)**.
   - Press **`A`** (select all vertices) $\rightarrow$ **`E`** (extrude) $\rightarrow$ **`Z`** (lock Z axis) $\rightarrow$ type `3.0` $\rightarrow$ press **Enter**.
   - The building walls are now 3D geometry extruded to exactly 3.0 metres.

### Method B: SVG Import
1. In KaLara Arch, select **File $\rightarrow$ Export $\rightarrow$ Export SVG (Vector Graphic)...**.
2. In Blender: **File $\rightarrow$ Import $\rightarrow$ Scalable Vector Graphics (.svg)**.
3. The SVG curves appear grouped by layer in the Outliner.
4. Scale by `0.001` to match metric scene units.
5. In the Curve properties panel, adjust **Extrude** under Geometry to create 3D walls directly.

### Verification Checklist
- [x] Wall length measures $5.000\text{ m}$ in Blender's Item Transform panel.
- [x] Wall thickness measures $0.200\text{ m}$ ($200\text{ mm}$).
- [x] Door swing curves align with door openings.

---

## 6. Geometric Integrity Benchmarks

Automated unit tests ([`test_workflow_validation.cpp`](file:///c:/Users/kasun/Projects/Game%20Dev/KaLara/KaLara%20Arch/tests/unit/test_workflow_validation.cpp)) and standalone validator script ([`tools/validate_cad_3d_workflow.py`](file:///c:/Users/kasun/Projects/Game%20Dev/KaLara/KaLara%20Arch/tools/validate_cad_3d_workflow.py)) certify:
- **Loop Closure**: 100% of wall boundary polygons are strictly closed loops.
- **Planarity**: $Z = 0.0$ for all 2D entities.
- **Shoelace Polygon Area Consistency**: $\Delta \le 0.01\%$ between model room area and polygon vertices.
- **Unit Precision**: DXF coordinates retain $\le 0.001\text{ mm}$ fidelity with zero precision loss.
