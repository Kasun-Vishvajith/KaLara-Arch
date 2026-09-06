# UI and visual quality specification

## Design direction

A precise architectural workbench: warm light canvas, neutral graphite chrome, restrained blue selection, crisp measured linework, generous local spacing, and no decorative clutter. Dark mode is equally finished. Avoid a collection of default Qt controls with unrelated spacing.

The plan owns the visual hierarchy. Dense professional tools remain discoverable through grouped menus and contextual options. Users should recognize the active tool, floor, snap state, selection, and edit consequence without searching.

## Shell composition

| Region | Default size and content |
|---|---|
| Top menu | File, Edit, Draw, Architecture, Annotate, View, Tools, Help |
| Project/context bar | 44 DIP; project title/dirty marker, active floor, active tool settings |
| Left rail | 48 DIP; Select, Wall, Opening, Draft, Place, Measure, Dimension |
| Left dock | 240 DIP; hierarchy/layers tabs; collapsible |
| Main canvas | Remaining area; plan and lightweight overlays |
| Right dock | 300 DIP; Inspector/Library tabs; resizable |
| Bottom status | 28 DIP; prompt, coordinates, units, snap/ortho/grid, zoom, drawing scale |
| Document strip | Open project tabs; close/dirty state; no many-document support through singleton state |

At 1366×768, collapse the left dock by default and keep the canvas usable. At 1920×1080, show both docks without squeezing numeric fields. Minimum supported window 1100×700 DIP with scrollable panels. Save layouts per user, provide Reset Workspace, recover off-screen windows after monitor changes.

## Tokens

| Token | Light | Dark |
|---|---|---|
| Chrome | #F3F4F6 | #181C22 |
| Panel | #FFFFFF | #222832 |
| Canvas | #FBFAF7 | #151A20 |
| Primary text | #202833 | #E8EDF4 |
| Muted text | #586575 | #ADB8C7 |
| Accent/selection | #2563EB | #6AA6FF |
| Warning | #9A5B00 | #F4C15D |
| Error | #B42332 | #FF7B87 |
| Divider | #D9DFE7 | #394352 |

These are proposed tokens; verify contrast in actual rendered states. Use a Windows system UI font such as Segoe UI with fallback, 13–14 DIP body, 12 DIP minimum ordinary labels, 16–20 DIP section titles. Numeric fields use tabular digits where available. Spacing scale: 4/8/12/16/24 DIP. Buttons 30–34 DIP high, main click targets at least 32 DIP; icons 18–20 DIP. Use readable text labels in menus and tooltips, never emoji-only tool icons.

## Inspector

Sections: Identity, Geometry, Placement, Style, Relationships. Mixed selection shows a genuine Mixed state. Editing a common field applies only to eligible selected entities and states the count. Locked selection explains why controls are disabled. Unit suffix remains visible.

Numeric edit: type value/unit → Enter commits once → Escape restores field → focus leaving validates visibly; no silent clamp. Invalid values keep input and show a reason. Continuous spinbox changes coalesce into one logical interaction. Geometry changes that affect others open the scoped change preview.

## Canvas overlays

Selected outline is blue; hover is weaker; preview uses dashed blue; invalid preview uses red plus an icon and message. Snaps display distinct glyph and text, not color alone. Grips stay 8 DIP with 12 DIP hit targets. Only selected/hovered connected elements show grips; do not render every wall's handles on a large plan.

Keep preview dimensions near but offset from cursor; avoid covering the placement point. Labels stay upright. Show temporary guides with lighter lineweight. Auto-hide only nonessential transient clutter during fast pan, never the active tool preview.

## Interaction feedback

Tool hints live in status bar, not a toast on every mouse movement. Toasts for save/export completion and meaningful warnings last 2–4 seconds, do not steal focus, and are also recorded when important. Long operations show progress and cancel where cancellation can be safe.

No modal for ordinary pan, select, or single wall placement. Delete Floor and broad dependency changes summarize the affected content. Scaling preview uses the dedicated dialog specified in the scaling contract.

## Library

Search at top, category filter, 2–3 column cards depending on width, favorites, local/custom filter. Card shows plan thumbnail, name, nominal size, and provenance/placeholder badge when relevant. Click selects an item for preview placement; it does not insert at a hidden canvas center. Double-click also enters placement; only canvas click commits.

## Accessibility and keyboard

Native shell controls expose accessible names, roles, checked/disabled states, and focus order. No color-only information. Numeric inspector and command search offer a keyboard alternative to essential canvas edits. F1 opens active-tool instructions. Screen-reader-friendly property summary describes selected objects and dimensions; do not claim full nonvisual CAD authoring from labeled toolbar buttons alone.

Verify 100%, 150%, 200% scaling, light/dark, keyboard-only open/save/property edit, high-contrast visibility, and reduced-motion preference. Animations are optional 100–160 ms panel/hover transitions, never geometry lag.

## Required visual evidence

Capture empty project, furnished plan, selected wall/inspector, scaling preview with warning, library placement, layer panel, sheet preview, recovery chooser. Capture both themes and 1366×768/1920×1080 layouts. Pass only with no clipping, overlapping controls, unreadable labels, missing icons, or lost focus traps.
