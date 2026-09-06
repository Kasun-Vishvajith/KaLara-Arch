# Import, export, and print interoperability

## Export rules

Export from an immutable committed model snapshot through shared scene extraction, not the current framebuffer. Respect chosen floors/layers, crop, lineweights, annotation styles, and explicit export scale. Never include selection handles, snap markers, hover, or a pending preview.

Report unsupported/lost representation before completion. Exported CAD geometry is full-size mm. Native semantics may become linework; state this in export summary. A successful file write alone does not prove scale, content, or compatibility.

## Formats

| Format | Required contract |
|---|---|
| PDF | Vector paths/text where possible, actual paper dimensions, chosen fixed scale, multi-sheet export |
| SVG | Explicit width/height in mm, correct viewBox, vector linework, no external assets, optional text-to-path if supported |
| DXF | Declared bounded profile below, modelspace full-size, layers, units, dimensions represented explicitly |
| JSON | Document schema and units, typed entities/references; readable interchange without ZIP |
| PNG | Explicit pixel dimensions/DPI, white or transparent background, offscreen render |
| JPEG | Explicit resolution, opaque chosen background; never pretend transparency |
| CSV | Room/opening schedules with units and proper text quoting |

## DXF profile

Target ASCII AutoCAD 2000-compatible AC1015 subset. Implement LINE, LWPOLYLINE, CIRCLE, ARC, TEXT, layers, and BLOCK/INSERT for simple repeated symbols. Wall fills may export as closed boundary polylines with optional supported hatch; the required baseline is clear wall boundary linework. Dimensions export as measured linework plus text by default, with a named layer and an explicit “exploded dimensions” note. Do not claim associative native CAD dimensions unless a later implementation within this release proves them.

Set modelspace insertion units to millimetres ($INSUNITS=4) and metric measurement convention. Layer names follow project mapping; generic defaults include A-WALL, A-DOOR, A-WIND, A-FURN, A-DIMS, A-TEXT, A-SITE. These are generic profile names, not a compliance claim.

DXF entities use Z=0 as required by file representation; this does not add a 3D feature. Unsupported custom SVG artwork is flattened to polylines with documented error bound ≤0.25 mm in model coordinates. Never omit an entire fixture silently.

## DXF import

Import the same line/polyline/circle/arc/text subset plus bounded nonrecursive/limited-nesting blocks into ordinary drafting entities. Units come from supported header values; missing/unitless input requires the user to choose units. Preview bounds and a known-distance calibration before commit.

Unsupported entities produce a count/list; the user can cancel or import supported subset explicitly. Nonplanar geometry rejects or requires an explicit XY projection option with a loss warning; no 3D importer. Do not infer walls, doors, rooms, or construction meaning merely from layers.

Apply file size/entity/nesting limits, reject invalid numbers, and parse into a candidate. One accepted import is one transaction. Preserve source-layer mapping without overwriting user's layer styles silently.

## Underlays

Raster PNG/JPEG and safe SVG reference underlays: move/rotate/opacity/crop/lock; two-point calibration. Pick known segment distance d_measured, enter true distance d_target, apply uniform underlay factor d_target/d_measured around first picked point. Existing semantic geometry remains unchanged. Mark uncalibrated underlays clearly.

Raster underlay import is not image-to-wall tracing. PDF import is not required; PDF is an output format in this release.

## Printing

Print preview uses selected paper, margins, sheets, and exact model-to-paper conversion. Warn if the physical printer's printable region clips content; allow adjusted margins or explicit fit with a changed-scale label. No hidden driver “fit” assumption.

Qt PDF/print adapters remain outside core. See official Qt deployment/printing-related APIs before pinning modules. Export SVG asset behavior must follow the validated subset in the library contract.

## Independent verification

Export fixture with 6000 mm line, 900 mm door, 200 mm wall thickness, circle radius 500 mm, 30° arc, Unicode note, and separated layers. Parse DXF with an independent reader (a pinned development-only ezdxf tool is suitable) and inspect in at least one external CAD viewer. Record actual viewer/version; do not invent coverage of all CAD applications.

Measure PDF line at 1:100 as 60 mm within 0.2 mm digital coordinate tolerance, verify page size, inspect text/layers/curves, and compare SVG physical dimensions. Open PNG/JPEG and confirm selected background and no editor overlays. External consumer validation is a release gate; if unavailable, mark BLOCKED.
