# Model, identity, and native schema

## Coordinates and identity

Project XY is local 2D millimetres; +X right/east by convention, +Y up/north before user north rotation. Floors share this XY frame. Store angles as radians; show degrees. Every persistent entity has an opaque stable ID, type, owner scope, layer ID, and revision metadata. UUIDs are suitable; pointer addresses, vector indices, and names are not identities.

Cross-references use IDs with explicit expected type. IDs survive save/load, transforms, and undo. Duplicates receive new IDs; remap internal references as one operation.

## Persistent versus derived versus session

| Persistent | Derived, rebuildable | Session only |
|---|---|---|
| Junction positions and wall definitions | Joined wall outlines and spatial index | Active tool and mouse capture |
| Opening host/offset/width | Opening render position and cutouts | Drag preview and snap candidate |
| Room boundary source and user metadata | Detected room polygon and area | Selected IDs and hover |
| Dimension references/style | Measured text and witness geometry | Camera and active panel |
| Library definition/version and instance parameters | SVG thumbnail and tessellation | Temporary measurement |
| Layer and sheet definitions | Print scene cache | Undo view navigation |
| Explicit constraints, units, intent | Validation result cache | Window placement |

Named views are persistent project entities; the current camera is session state. Recovery may save useful session state separately without making it architectural truth.

## Entity inventory and mandatory fields

| Entity | Required domain fields |
|---|---|
| Project | schemaVersion, ID, title, buildings, site, styles, unitDisplay, intent, asset index |
| Site | ID, boundary polygon or unknown, north angle, reference annotations |
| Building | ID, name, floor IDs |
| Floor | ID, name, order, optional elevation scalar, entity IDs |
| Junction | ID, floor ID, position XY |
| Wall | ID, start/end junction IDs, positive thickness, referenceLine, side convention, wallType |
| Opening | ID, kind, hostWallID, centerOffsetMm, widthMm, anchoring rule, symbolType |
| Door | Opening fields plus hinge end, swing side, leaf count, optional schedule metadata |
| Window | Opening fields plus frame pattern, optional sill/height schedule metadata |
| Room | ID, mode derived/manual, boundary references or polygon, name/type, label anchor |
| Draft entity | Typed line/polyline/arc/circle/polygon/text/hatch geometry |
| Library instance | ID, definition ID/version, transform, physical dimensions, parameter overrides |
| Dimension | ID, kind, typed references, offset, style ID, optional driving constraint ID |
| Layer | ID, name, visible/locked, print flag, style override |
| Group | ID and members, no cyclic membership |
| Constraint | ID, kind, referenced IDs/features, parameters, hard/soft, enabled |
| Sheet | ID, paper size mm, orientation, margins, title block, placed viewports and tables |
| Underlay | ID, asset hash, floor ID, 2D transform, opacity, calibrated flag, lock |

Do not persist a decorative door SVG as the only evidence a wall has an opening.

## Wall orientation and references

Wall tangent points from start to end. Left is its positive perpendicular; right is negative. Opening centerOffsetMm measures along tangent from start. Width occupies the symmetric interval around that center. Reversing wall direction maps center offset to L minus offset and remaps hinge/swing so world appearance remains equivalent.

Feature references are typed: JunctionPoint(id), WallAxisPoint(wallID, offset), WallFace(wallID, left/right), OpeningJamb(openingID, start/end), ArcCenter(id), ExplicitPoint(xy). Never store an arbitrary render-vertex index as a persistent dimension reference.

## Illustrative JSON record shape

This fragment specifies fields, not a complete loadable project. Step 03 must create executable JSON Schema and complete fixtures.

~~~json
{
  "schemaVersion": 1,
  "projectId": "project-example",
  "canonicalUnit": "mm",
  "revision": 12,
  "display": {"lengthUnit": "mm", "lengthDecimals": 0},
  "entities": [
    {"id": "node-a", "type": "junction", "floorId": "floor-ground", "position": [0, 0]},
    {"id": "node-b", "type": "junction", "floorId": "floor-ground", "position": [6000, 0]},
    {"id": "wall-a", "type": "wall", "floorId": "floor-ground",
     "startNodeId": "node-a", "endNodeId": "node-b",
     "thicknessMm": 200, "referenceLine": "center", "layerId": "layer-walls"},
    {"id": "door-a", "type": "opening", "kind": "door", "hostWallId": "wall-a",
     "centerOffsetMm": 3000, "widthMm": 900,
     "anchor": {"mode": "distanceFromStart"},
     "hingeEnd": "start", "swingSide": "left"}
  ],
  "intent": {"targetArea": {"state": "undecided", "value": null}}
}
~~~

## Invariants

References resolve with correct types and scope. Walls have distinct junctions and nondegenerate length. Openings fit host intervals and do not overlap other openings. Dimensions either resolve or visibly report orphan status. Rooms do not contain self-intersecting outer rings. Layers exist. All numbers are finite. Assets resolve by hash or have a declared fallback. Group/host graphs have no illegal cycles.

Deleting a host expands into a visible dependency plan; it cannot silently strand openings. For deleting dimensions' geometry, default is delete dependent dimensions after preview, with explicit keep-as-orphan option. Orphan dimensions retain last value only as visibly stale data, never as valid measurement.

## Schema evolution

Reject unknown major schema versions for editing; offer safe inspection/export only if supported without losing fields. Migrations are pure old-to-new transformations with fixtures and backup. Unknown extension metadata is preserved in a namespaced extensions field, never executed. A save from an older app must not silently erase newer semantic entities.

Serialization ordering is deterministic by ID; semantic equality ignores timestamps, cache, and insertion ordering. Store enough significant digits to round-trip double values; presentation rounding does not quantize the model.
