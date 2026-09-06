# Structured library and SVG artwork contract

## Principle

A library item is semantic data with physical dimensions, parameters, anchors, and a 2D representation. SVG is optional artwork; it never supplies the only size or host relationship.

The delivered library/ folder contains category directories and placement instructions. During Step 14, implement the runtime catalog and procedural generators. Do not ship an empty library waiting for the user to search for images.

## Folder layout

- library/catalog/: versioned definitions and category index.
- library/symbols/doors/, windows/, furniture/, kitchen/, sanitary/, stairs/, structural/, electrical/, site/, annotations/: optional SVG artwork.
- library/styles/: lineweights, hatch and dimension profiles.
- library/templates/: editable project seeds.
- library/licenses/: source licenses/attributions.
- library/user/: repository sample user-pack definitions; the installed app writes user packs to its user-data directory.
- library/thumbnails/: generated preview cache in development; installed app cache belongs outside its program directory.

Artwork instructions in every category describe what goes there. Runtime asset lookup must not assume the working directory is the repository.

## Definition schema example

~~~json
{
  "schemaVersion": 1,
  "id": "kalara.furniture.bed.double",
  "version": 1,
  "name": "Double bed",
  "category": "furniture",
  "semanticType": "bed",
  "nominalSizeMm": {"width": 1800, "depth": 2000},
  "resizePolicy": "independentAxes",
  "anchors": [{"id": "center", "uv": [0.5, 0.5]}],
  "clearancesMm": {"left": 600, "right": 600, "front": 750, "rear": 0},
  "representation": {
    "svg": "symbols/furniture/bed-double.svg",
    "fallback": "bed",
    "svgOrigin": "topLeft",
    "svgPositiveY": "down",
    "fit": "contain"
  },
  "provenance": {"kind": "original-procedural", "license": "Apache-2.0"}
}
~~~

Clearances and sizes above are editable planning defaults, not certified minimums. Metadata without verified provenance must say unknown; do not invent a license for user artwork.

## SVG normalization

Require a valid viewBox, finite bounded dimensions, self-contained geometry, no external URL references, scripts, event handlers, foreignObject, or executable content. Restrict supported artwork to basic paths/shapes, transforms, fills, and strokes. Reject unsupported effects or offer an explicit rasterized appearance import as an underlay, not a pretend semantic symbol.

Normalize viewBox to the item's physical bounding box. Account for SVG Y-down versus project Y-up once at the adapter. Physical dimensions always come from the definition. Fit=contain preserves aspect ratio; fit=stretch requires an explicit definition. Original stroke width is mapped to the selected symbol style; do not let scaling make furniture strokes thicker than cut walls.

An SVG viewBox does not establish millimetres by itself. The import dialog asks nominal width/depth and anchor when metadata is absent. Thumbnails and in-plan representation use the same normalization.

## Asset identity and updates

Pin definition ID/version and asset content hash into each placed instance/document asset table. Replacing a library SVG affects newly placed instances by default, not existing saved projects. Provide explicit Update Instances with before/after preview and a single undoable transaction. Embedded document assets take precedence, so moving a project to a second machine preserves appearance.

Missing or invalid SVG uses the declared procedural generator and records a nonblocking diagnostic. Missing custom definition preserves embedded last-known definition. If neither exists, show a labeled dimensioned rectangle with preserved bounds and an explicit missing-item indicator.

## Starter catalog minimum

| Category | Required defaults |
|---|---|
| Doors | Single swing, double, sliding, pocket, open passage |
| Windows | Fixed, casement, sliding |
| Furniture | Single/double bed, sofa, chair, armchair, dining table, desk, wardrobe, shelf |
| Kitchen | Base cabinet, wall cabinet plan, sink, hob, refrigerator, island |
| Sanitary | WC, washbasin, shower, bath |
| Stairs | Straight, L, U; spiral plan symbol only |
| Structural | Rectangular/circular column, grid marker |
| Electrical | Light, switch, socket as plain 2D symbols |
| Site | Car, tree, shrub, bench, gate, pool outline |
| Annotations | North arrow, scale bar, level marker, section-reference marker, revision cloud |

Provide at least these 45 identifiable items (count programmatically from catalog rather than claiming a fixed count from this table). Electrical symbols and section-reference markers are annotation capabilities, not full MEP or section-generation systems.

Procedural drawing primitives: lines, polygons, arcs, circles, repeat patterns, labels. A toilet fallback should resemble a toilet; a bed should show mattress/pillows. Category initials alone are acceptable only for unknown custom assets, not the built-in catalog.

## User blocks

Select eligible 2D entities → Create Library Item → name/category → anchor → preview → store as a versioned local pack. Remap IDs on each placement. Prevent recursive block cycles; limit nesting to eight. Host-dependent openings cannot become free-standing blocks without explicit host-template semantics.

## Quality gates

With every SVG folder empty, all built-in items remain placeable/editable/exportable. Add one valid SVG and verify correct size/orientation/anchor. Malicious/oversized/unsupported SVG rejects safely. Save with custom assets, remove external pack, reopen successfully. Update asset instances, then Undo restores previous version and appearance.
