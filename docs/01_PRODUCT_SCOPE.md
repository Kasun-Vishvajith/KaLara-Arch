# Product scope — local 2D architectural studio

KaLara Arch combines approachable room planning, precise CAD-style editing, and drawing documentation. Its central workspace is a calm 2D plan with contextual tools and a numeric inspector. The signature Change Dimensions preview shows what moves, what keeps its size, which scope changes, and why an edit may be impossible.

## Complete release

| Family | Required features |
|---|---|
| Projects | Empty/template start, metadata, units, site, floors, recent files, portable saving, recovery |
| Draft | Line/polyline, rectangle, circle, arc, polygon, guides, offset, trim/extend, hatch, editable text |
| Architecture | Arbitrary-angle straight walls, joins, thickness, doors/windows, columns, plan stairs/ramps, rooms, fixtures |
| Input | Numeric length/angle/coordinates, semantic snaps, ortho/polar, pan/zoom, marquee, cycling, keyboard workflows |
| Edit | Move, grips, rotate, mirror, duplicate, copy/paste, groups, align/distribute, lock/hide, scoped resize/scale |
| Measure | Distance, projections, angle, area/perimeter; linear/aligned/angular/radius/diameter/chain dimensions |
| Organization | Layers/categories, hierarchy, floor underlays, named views, filters, room/opening schedules |
| Library | Search, thumbnails, categories, favorites, parameters, anchors, clearances, user blocks, SVG replacement |
| Site | Boundary, roads as reference lines, north, footprints, parking/landscape/outdoor symbols, configured setbacks |
| Documentation | Sheets, title blocks, viewport crops/scales, lineweights, notes, legends, clouds, tags, printing |
| Exchange | Native archive, JSON, SVG/PDF/PNG/JPEG export, bounded DXF import/export, raster/SVG underlays |
| Quality | Complete undo/cancel, atomic save, recovery, accessible shell, frame budgets, packaging, offline acceptance |

Foundation steps may expose fewer tools; final release may not ship these required features as placeholders. No account, AI model, Python runtime, embedded browser, or internet is required.

## Outside this bounded release

3D views/meshes, BIM/IFC, generated elevations/sections, structural analysis, photorealism, terrain modeling, GIS projection, DWG codecs, cloud collaboration, runtime AI, and universal building-code certification. No inactive toolbar for these concepts.

Curved semantic walls, parametric spiral stairs, splines, full MEP network design, survey adjustment, and proprietary CAD compatibility beyond the declared DXF subset are excluded. Arc drafting and spiral-stair symbols are still ordinary 2D features. Scalar level elevations or opening sill values may appear in schedules; they do not justify a spatial engine.

## Professional conventions

Lineweight, line type, layer names, annotation sizes, and symbols are editable profiles. Ship KaLara Generic Metric and an imperial display profile. Typical defaults are not universal construction rules. Do not label a profile ISO/AIA compliant without a verified specification and tests. Site rules start undecided rather than assuming local regulations.

## First-session acceptance

Create Empty → metric → 6000 × 4000 mm centerline enclosure → 900 mm door → 1200 mm window → room label → diagonal measure → change horizontal wall to 7500 mm with X-only connected layout resize → Undo → furniture → A3 sheet at 1:100 → save/reopen → measured PDF and DXF.

Every step works without programming, internet, or downloaded SVGs. Templates use optional known/undecided fields and ordinary editable geometry. They do not promise automatic generative design.
