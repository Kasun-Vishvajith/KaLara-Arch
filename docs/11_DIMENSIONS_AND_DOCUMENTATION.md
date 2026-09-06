# Measurement, annotation, schedules, and sheets

## Temporary measurement tools

Distance: pick two points; show true distance, ΔX, ΔY, and angle relative to project X. Snap to meaningful geometry. Keep result visible until next measurement or Esc; optional click-drag shortcut gives the same result. No document mutation until Pin as Dimension.

Angle: pick vertex and two ray points, or select two nonparallel segments. Compute atan2(cross,dot) and expose interior 0–180° plus optional directed/reflex angle. Zero-length rays reject. Parallel segments report 0°/180° as applicable; do not invent an intersection at infinity. A vertex need not be a real wall junction for temporary measurement.

Area/perimeter: select closed region or trace polygon, including holes where supplied. Show units and whether result is manual region, centerline enclosure, or net room. Self-intersections reject with offending edges highlighted.

## Persistent dimensions

Kinds: aligned, horizontal, vertical, angular, radius, diameter, chain/baseline. Refer to stable semantic features. Witness lines and labels derive from geometry. A measured dimension never stays a stale number after its references move.

Default dimension is reporting only. “Use as driving dimension” explicitly creates a supported constraint and opens the applicable anchor/scope preview. Editing a label is not geometry editing: custom prefix/suffix is allowed; numerical overrides must be visibly marked as overridden and generate a validation warning.

If a source disappears, dimension is deleted through the dependency plan or explicitly retained as orphan. Orphan displays warning and last-known value crossed/marked stale; exclude it from valid schedule values.

## Dimension style

Project styles contain paper text height, arrow/tick size, witness gap/extension, dimension-line offset, unit format, precision, trailing-zero policy, color and lineweight. Default text height 2.5 mm on paper is a generic starting point. Styles are editable and not declared universally standard.

At sheet scale 1:N, paper-size annotation is converted to N×size in model rendering for that viewport. At 1:100, 2.5 mm paper text corresponds to 250 mm in model units, but measured lengths do not change. Each sheet viewport can use its own annotation scale; do not multiply native coordinates twice.

Chain dimensions preserve referenced order and prevent overlapping labels through layout suggestions. Automatic room dimension placement is user-invoked and previewable. It does not clutter every wall by default.

## Text and symbols

Room tags, room areas, door/window tags, north arrow, grid bubbles, leader notes, level labels, scale bars, legends, and revision clouds. Rotated text remains legible by style rule. Text uses a stored font family with fallback; missing font shows substitution before export. Prefer embedding a distributable font in release assets only after license verification.

Grid lines/bubbles are 2D annotation objects, not a hidden structural model. Section-reference symbols are manually labeled references without generated sections.

## Schedules

Generate room schedule: ID/tag, name, floor, net area, perimeter, status. Generate door/window schedule: tag, floor, type, width, optional metadata, host. Missing/undecided height/sill remains blank or “Undecided”, never zero. Select schedule row highlights source entity.

Refresh from committed snapshot. Read-only computed columns; editable tags/names dispatch normal entity commands. Export UTF-8 CSV with quoted fields and unit headers. Sheet tables paginate with repeated headings and no clipped rows.

## Sheets and drawing scale

A4/A3/A2/A1 sizes, portrait/landscape, explicit printable margins and custom paper size. A sheet holds title block, one or more 2D viewports, notes, legends, and schedules. Viewports refer to a floor/site named view, crop, visible layers, style overrides, and scale ratio.

Support 1:20, 1:50, 1:100, 1:200 and custom positive ratio. Display exact scale and scale bar. Viewport rotation is optional only if fully tested; default project-up. A crop changes visibility, never model geometry.

Sheet-space move/resize uses paper mm. Model-space drafting stays full-size mm. Do not use canvas zoom as print scale. Fit-to-page is explicit and shows “Not to fixed scale” if it changes the chosen ratio.

## Evidence

6000 mm line produces 60 mm on PDF at 1:100. The same line produces 120 mm at 1:50; paper annotation stays 2.5 mm. Angular measurement on perpendicular walls reads 90°. A 3000–4000–5000 triangle measures 5000 mm and 53.130102° at the appropriate ray. Moving a referenced endpoint updates dimension and schedule after commit and Undo.
