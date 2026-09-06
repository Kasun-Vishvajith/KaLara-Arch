# Architectural authoring contracts

## Walls

Straight walls at any angle, orthogonal-friendly default. Chain, rectangle-enclosure helper, split, compatible merge, trim/extend, endpoint drag, full-wall translation, type and thickness editing. Defaults: exterior 200 mm, interior 150 mm, user-editable preset values. These are examples, not construction requirements.

A rectangular wall helper creates four connected semantic walls in one command. Individual wall chain segments remain separate commands. Walls show proper face joins and openings. Wall position/length inspector states whether values refer to centerline or selected face.

Wall translation of shared junctions previews incident wall changes. Thickness changes offer center/left/right face anchoring. Imported drafting lines do not become walls until Convert to Wall asks thickness and side.

## Doors and windows

Place only on eligible wall with ghost cutout, host highlight, width, and clear fit indication. Projection uses wall axis; click location is opening center. Reject overlapping intervals or end overrun. Minimum end pier distance is a configurable rule, not universal hardcoded law.

Door patterns: single swing, double swing, sliding, pocket, opening-without-leaf. Window patterns: fixed, casement, sliding. Draw procedural jamb/frame/leaf/swing geometry so no SVG is necessary. Width handles run along the host tangent; move is along host. No free rotation or screen-Y “height” handle.

Hinge end and swing side are independent; mirror and wall reversal remap both. Slider patterns expose pattern-specific parameters. Optional height/sill metadata appears only in inspector and schedules, not as an extra spatial feature.

Delete host previews hosted deletion. Splitting through opening blocks; splitting elsewhere remaps offsets. Moving wall retains attachment. Doors in room detection close the boundary virtually; swing arcs do not divide room polygons.

## Rooms and spaces

Click inside an enclosed region to create a derived room; manual polygon mode supports deliberate zoning. Derived room auto-detection is user-invoked initially and updates affected registered rooms after edits. Open boundaries produce clear “Not enclosed” diagnostics and no fabricated area.

Store user ID/name/type/label location separately from derived geometry. Match old/new room faces through boundary references plus overlap when geometry changes. On split: preserve ID on the region containing the old label anchor, create a proposed new room for others. On merge: ask which metadata survives; never randomly rename/delete room metadata.

Report net area using the documented finished-face policy, perimeter, and optional bounding dimensions clearly labeled as bounds. Irregular rooms do not have an invented rectangular width/length. Courtyard holes subtract area. Provide display in m²/ft² independent of stored mm².

## Columns and structural symbols

Rectangular and circular column entities in plan, editable physical size, rotation where applicable, tag, and optional area-exclusion behavior. They are drawing objects, not a structural sizing engine. Beams may be represented as annotated linework with a chosen dashed style; do not imply load analysis.

## Stairs and ramps

Straight, L-return, and U-return plan stairs: outline, treads, landing, direction arrow, break mark, and tag. Parameters include clear width, number of treads, tread depth, landing size, and optional rise metadata for schedule. Validation checks internal geometry consistency and configured warnings, not statutory approval.

L/U stair geometry is generated in local 2D coordinates and transformed as an ordinary plan object. Interfloor link is an explicit relationship between floor plan markers, never an automatic 3D connector. A ramp is a 2D polygon with direction and optional slope text metadata.

## Fixtures and furniture

Beds, seating, tables, cabinets, kitchen modules, sanitary fixtures, parking, landscape, and outdoor objects use library definitions. Exact physical dimensions and anchors exist even with missing artwork. Clearance zones are optional visual/validation envelopes; they are not ordinary occupied area and do not print by default.

Group/duplicate, move, rotate, mirror, align, distribute, and custom user blocks are required. Invalid negative/zero sizes reject. Object presets may lock aspect ratio; the inspector explains it. Do not resize fixed-width doors as generic sofa boxes.

## Floors

Add/rename/reorder/duplicate/delete floors. At least one floor must remain. Navigation never creates floors. New floor choices: empty, copy selected categories, copy exterior outline. Copy creates fresh entity IDs and internal reference remapping; it does not accidentally share mutable nodes across floors.

Underlay another floor at adjustable opacity, locked and noneditable; snap opt-in. Floor elevations are optional scalar labels. A floor is not a perspective level or 3D mesh.

## Site

Closed property polygon; north arrow; road reference lines; entrance, footprint, parking and landscape polygons; setbacks by inward polygon offset from configured distances. Concave offset can create multiple regions; show all resulting feasible regions or an empty result. Do not assume every offset produces one rectangle.

Site and building XY share project coordinates. Setting north rotates its symbol, not every wall. Unknown site dimensions/rules remain unknown. All objects remain 2D.

## Acceptance

Create and edit a furnished two-floor house with a courtyard and site boundary. Test floor duplicate/remapping, opening reversal, room split/merge metadata, stair parameter edits, clearance visibility, and printed symbols. The complete sample remains editable after save/reopen.
