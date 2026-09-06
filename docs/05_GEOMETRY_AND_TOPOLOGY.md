# Geometry, units, and wall topology

## Numeric policies

Use double-precision real coordinates in mm. Nominal display precision is 1 mm by default; allow decimal millimetres for detail work. Do not round stored geometry to integers after every transform.

Default geometric comparison epsilon: max(0.000001 mm, 1e-12 × local operation extent), with robust predicates for intersection/orientation. A separate user-visible merge suggestion tolerance defaults to 1 mm. Screen snap radius defaults to 8 device-independent pixels. These values have different purposes.

Supported design envelope: coordinates within ±10,000,000 mm per axis; reject out-of-range operations with a message. Rebase to camera-local coordinates before float GPU conversion. Zero-length rejection uses numerical tolerance, not a 300 mm planning rule. Extremely short walls may produce a configurable warning, not an automatic prohibition.

Parse mm, cm, m, in, ft and feet-inch fractions. Bare numbers use current display units. Exact conversions: inch=25.4 mm, foot=304.8 mm. Require finite values and explicit error indication. Unit changes only change formatting and input interpretation.

## Geometry primitives

Point2, Vector2, Segment2, Arc2(center,radius,startAngle,sweep), Circle2, Polyline2, Polygon2(outer, holes), AABB2, Transform2, Angle and Length. Polygon outer rings are counterclockwise, holes clockwise in project coordinates. Reject self-intersections for filled semantic regions; raw drafting polylines may remain open.

Implement projection, nearest point, segment intersections, point-in-polygon, offset, signed area, bounds, line/arc intersections for snapping and trim, transform inversion, and robust orientation. Use a pinned tested polygon library if custom boolean operations would be unreliable.

## Explicit junction topology

Walls connect by shared Junction ID. Proximity only proposes a join. Draw to existing endpoint reuses its junction; draw onto wall interior explicitly splits the host at that point before creating the new wall in the same transaction. Draw through a crossing asks Join or Keep Crossing when semantics are ambiguous; show a persistent connection marker.

Index incident walls by junction ID. Moving a junction updates every incident wall and dependent opening validation. Coincident independent junctions may exist deliberately and must not become silently welded.

## Wall representation and joins

The semantic source is straight reference axis plus positive thickness, direction, and reference-line policy. Render outlines are derived. At a standard two-wall corner, intersect offset faces; use a configurable miter limit of four times wall thickness and bevel beyond that to avoid spikes. T/X joins derive a union for visual fill while retaining original semantic wall IDs.

Wall faces and opening cutouts are actual derived geometry; do not paint background-colored rectangles that erase neighboring hatches or underlays. Render/detect rooms using the same committed wall boundary rules. Recompute only affected connected neighborhoods.

Thickness changes ask preserve centerline, left face, or right face. UI may label interior/exterior only when the wall has explicit side metadata; do not infer it from screen direction. Moving reference axes to preserve faces must update topology consistently or reject with a conflict preview.

## Split and merge

Split wall at distance s: retain old wall ID for start segment, allocate ID for end segment, create junction, remap references by interval. Opening entirely before/after split is assigned accordingly; one straddling the split blocks until user changes split or opening. Dimension references and constraints map by feature semantics, not array position.

Merge is allowed only for collinear compatible walls with consistent thickness/type and no conflicting hosted/reference semantics. Preview the retained ID and remapping. Do not erase a significant junction hosting a third wall.

## Room boundary basis

Automatic rooms follow finished inner wall faces with doorway gaps treated as virtual boundaries for space enclosure. Columns inside a room are holes when marked area-excluding. Furniture is not subtracted. Explicit polygon rooms use their own geometry and are labeled manual.

For a closed 6000 × 4000 centerline rectangle with 200 mm centered walls, clear inner rectangle is 5800 × 3800; net area=22.04 m² and inner perimeter=19200 mm. The centerline polygon area is 24 m² and must not be mislabeled net room area.

## Required numerical evidence

Rectangle, L-shape, diagonal junction, near-parallel corner, T/X joins, wall reversal, tiny valid segment, wall split near opening, ring with a courtyard hole, unit parser, camera transform round-trip, and topology-preserving Undo.

Use absolute/relative tolerances appropriate to quantities; area tolerances are in mm². A 2D point comparison epsilon is not an area tolerance.
