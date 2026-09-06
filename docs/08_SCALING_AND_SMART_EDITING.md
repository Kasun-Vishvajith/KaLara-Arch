# Change Dimensions — resize, scale, and constraint-aware editing

This is a release-critical feature. Do not implement it as a raw multiplication hidden behind a wall-length field.

## Five separate concepts

| Operation | Changes |
|---|---|
| Edit wall length | Changes selected endpoint geometry using an explicit anchor/propagation choice |
| Resize Layout | Transforms layout positions while preserving architectural sizes by default |
| Scale Geometry | Applies a mathematical 2D scale to supported geometry with explicit semantic consequences |
| Calibrate Underlay | Changes only the reference image/vector underlay's transform |
| Drawing scale / display units | Paper presentation / numeric formatting; neither changes model geometry |

Editing a wall length offers **This wall** or **Resize layout using this wall**. The latter opens the dialog below. A user can select an “Always ask” preference; it is default. Remembering choices must not bypass new conflicts or broaden scope silently.

## Dialog layout and defaults

Title: **Change dimensions**.

- Reference: Wall W-014, current length 6000 mm, target length field.
- Operation: Resize layout (default) or Scale geometry.
- Scope: This selection; Connected wall network; Active floor; Chosen floors.
- Axes: X only; Y only; Uniform X and Y.
- Anchor: reference start, reference end, reference midpoint, selection center, project origin, picked point.
- Size policy: preserve wall thickness and opening widths (default for layout); object positions only by default; explicit alternate choices below.
- Affected count: walls, openings, objects, dimensions, floors, locked items.
- Before/after ghost overlay, reference target measurement, bounding width/depth, factors.
- Diagnostics list: blockers, warnings, reference/size consequences.
- Buttons: Cancel and Apply. Apply disabled when blockers exist.

Nothing changes until Apply. Changing any option recomputes from baseline. One Apply is one undoable command. The viewport stays available for inspecting the preview but unrelated edits are suspended.

“Entire thing” is presented as **Active floor** by default. **Chosen floors** requires visible explicit floor selection, with an optional Include site toggle off by default. Hidden entities inside selected scope are included and counted; locked entities are never silently skipped. No invisible scale of another floor or site.

## Exact transform

For anchor a=(ax,ay), point p=(x,y):

p' = a + diag(sx,sy)(p-a).

X only sets sy=1; Y only sets sx=1; uniform sets sx=sy=s. This release's axes are project X/Y, not screen axes or arbitrary local axes. Camera orientation must not change the operation.

For reference vector (dx,dy), current length L and requested positive length Lt:

- Uniform: s=Lt/L.
- X-only: sx=sqrt((Lt²-dy²)/dx²), sy=1.
- Y-only: sy=sqrt((Lt²-dx²)/dy²), sx=1.

X-only is undefined if dx is numerically zero. It is impossible if Lt≤abs(dy), because zero/collapsing and imaginary factors are rejected. Y-only has the symmetric rule. Do not use Lt/L for a diagonal wall in axis-only mode.

Explicit factor entry is also allowed and updates the achieved reference length. Require finite positive factors; negative scaling belongs to Mirror. Validate coordinate envelope and nondegenerate entities before Apply.

## Mandatory examples

| Reference | Mode and target | Expected |
|---|---|---|
| (0,0)→(6000,0) | X only to 7500 | sx=1.25, sy=1 |
| (0,0)→(6000,0) | Uniform to 7500 | sx=sy=1.25 |
| (0,0)→(0,4000) | X only to 5000 | Undefined reference; explain use Y/uniform |
| (0,0)→(3000,4000) | X only to 6000 | sx=sqrt(20,000,000)/3000 ≈1.490711985 |
| (0,0)→(3000,4000) | X only to 3500 | Impossible; fixed Y component is already 4000 |
| (1000,2000)→(7000,2000) | X only 1.25 anchored at start | start unchanged, end=(8500,2000) |

On the 6000×4000 centerline rectangle: X-only 1.25 produces 7500×4000, with default 200 mm walls preserved. Net room area becomes 7300×3800=27.74 m². Uniform Scale Geometry 1.25 with thickness scaling produces 7500×5000 axes and 250 mm walls; clear area=7250×4750=34.4375 m².

## Resize Layout semantics

Transform junction positions. Preserve scalar wall thickness by default, then rebuild joins. Preserve door/window widths. Map opening center positions by normalized host fraction for this operation, independent of the normal endpoint-edit anchoring preference; show this temporary policy in the dialog.

An opening originally centered at 3000 on a 6000 wall becomes centered at 3750 on a 7500 wall and remains 900 wide. Bounds/collision checks still apply. Optional keep distance from start/end is available and explicitly changes this policy.

Free objects default to position-only: transform anchor positions; retain physical width/depth and rotation. This preserves beds, toilets, appliances, and standard-size doors. Explicit **Resize eligible symbols** scales local width/depth by the chosen object parameters, not by silently shearing furniture. Nonuniform scaling of a rotated rectangular symbol cannot generally be represented by width/depth/rotation alone; exact geometric transformation requires conversion as described below.

Rooms derived from walls are recomputed. Manual room polygons transform. Dimension references stay associated; annotation text size stays paper-based. Site boundary changes only when explicitly included. Underlays remain unchanged unless included explicitly, preventing accidental recalibration.

## Scale Geometry semantics

Raw points/lines/polygons receive exact affine transforms. Uniform scale applies to circle radii, arc radii, wall thickness, opening width and offset, and free symbol sizes. Paper text/lineweight does not scale; model-space custom text follows its declared style.

Nonuniform scale of a circle creates an ellipse; this release's semantic circle/arc types cannot represent it. Before Apply offer **Convert affected curves to polylines** with maximum deviation 0.25 mm in transformed model space, or cancel/change operation. Conversion creates raw drafting geometry and makes loss of semantic circle/radius references explicit. Block unresolved driving dimensions.

Nonuniform scale of an oblique wall transforms its rectangular section to a parallelogram; it is not exactly representable as an ordinary perpendicular-thickness semantic wall. Offer Resize Layout to preserve wall semantics, or explicitly convert affected semantic walls AND hosted opening render geometry to a grouped raw drafting representation. Show loss of room/host intelligence and dependent references; never silently approximate and call it exact. Uniform scaling preserves wall semantics.

Nonuniform rotated symbols use the same explicit raw-outline conversion for exact geometry, or position-only/layout policy. Conversion is optional and entirely undoable. The dialog should guide architectural users to Resize Layout, not force conversion as the default.

## Connected editing, selection boundaries, and locks

Connected scope is the wall junction graph within selected floor(s), plus all hosted dependents and dimensions referring to changed features. It does not jump floors, traverse an underlay snap, or scale every furniture item merely because it touches a wall. Active-floor scope additionally includes ordinary plan entities, with size policies stated above.

Selection-only changes touching a shared junction show three choices: include connected geometry, stretch incident boundary walls, or detach selected connections. Default is include connected geometry after preview; never detach automatically. Detach duplicates junction IDs and diagnoses orphan constraints.

Locked geometry or a hard constraint blocks incompatible changes. Offer change scope or an explicit separate unlock/constraint edit; do not skip locked nodes, delete constraints, or silently apply a partial scale. Hidden content remains counted.

## Single-wall endpoint editing

Anchor start/end/midpoint. Keep joined endpoints uses shared junction movement, stretching incident walls; preserve a connected network invokes scoped layout resize. Normal opening anchoring supports distance-from-start, distance-from-end, or proportional position as persisted preference. A wall shortened below its opening interval rejects without moving/clamping the opening silently.

## Required tests

All numeric examples above; X-only/y-only at arbitrary angles; nonorigin anchor; zero/negative/NaN factor; mixed locks; cross-floor scope; hidden entities; excluded site; preserved 900 mm opening; opening fit failure; manual/derived room behavior; circle conversion tolerance; nonuniform oblique wall warning; one Undo restoring semantic types after conversion; Cancel preserving semantic hash; driving-dimension conflict; repeated preview changes without cumulative drift.
