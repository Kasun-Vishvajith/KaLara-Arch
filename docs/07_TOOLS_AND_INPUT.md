# Tool system and input contracts

## Tool state machines

Every tool implements enter, pointerDown/Move/Up, key, cancel, and exit through a document ToolController. Persistent tool identity and transient state are separate. A captured drag cannot accidentally become a selection marquee. Deactivate cancels pending work before activating the next tool.

| Tool | States | Commit point | Esc |
|---|---|---|---|
| Select | Idle, Marquee, Drag, GripDrag | Valid pointer release | Discard preview |
| Wall | AwaitStart, AwaitNext | Each distinct valid next point | End chain; keep committed segments |
| Door/window | SeekHost, Preview | Canvas placement click | Exit placement |
| Library | Chosen, Preview, RotatePreview | Canvas click | Discard unplaced instance |
| Distance | AwaitFirst, AwaitSecond, Result | No model write; optional Pin | Clear result |
| Angle | Vertex, FirstRay, SecondRay, Result | No model write; optional Pin | Clear current construction |
| Dimension | References, Offset, Preview | Final placement click | Discard pending dimension |
| Draft line/polyline | AwaitStart, AwaitNext | Segment/polyline finish per tool contract | Discard unfinished entity |
| Scale/resize | SelectScope, PreviewDialog | Apply | Restore unchanged baseline |

For polyline drafting, one finished polyline is one command; wall chains intentionally commit per segment. Backspace during an unfinished polyline removes its last pending vertex; it must not delete a selected unrelated entity.

## Navigation and selection

Wheel zooms around the cursor. Middle drag or Space+left drag pans. Pan suspension preserves the active drawing tool. Zoom Fit considers visible printable geometry but excludes distant accidental guides by explicit option. Zoom Selection uses selected bounds. Empty fit shows origin at sensible default zoom.

Left-to-right marquee selects fully enclosed eligible entities; right-to-left selects intersecting entities and shows a dashed boundary. Shift adds/removes according to the displayed selection operation. Alt-click cycles overlapping eligible objects in a stable order. Hidden entities do not participate; locked entities can be inspected but cannot move.

Hit priority: active grip → selected object control → opening → library/fixture → draft annotation → wall → room fill → underlay. Cycle selection allows reaching lower objects. All hit tolerances are screen/DIP based.

## Snapping

Candidates: endpoint, junction, midpoint, intersection, perpendicular foot, nearest point, center, alignment, grid, and host projection. Query a spatial index around cursor. Apply explicit tool constraints first, then rank compatible candidates by semantic priority, distance, and stable ID. Ortho must not be silently defeated by an off-axis endpoint snap.

Default radius 8 DIP and release hysteresis 12 DIP prevent jitter. Tab cycles candidates; show candidate label. Grid display is independent of grid snapping. Snap to underlay floors is opt-in and visibly marks reference-only snaps; it never joins geometry across floors.

Numeric input: length, angle, absolute X/Y, and relative delta. Tab switches active numeric field. Input focus suppresses single-letter shortcuts. Enter accepts a valid value or commits according to tool hint. Do not evaluate arbitrary expressions as code; a bounded arithmetic parser may support +,-,*,/ and unit literals.

## Shortcut defaults

| Input | Action |
|---|---|
| V / W / D / I | Select / Wall / Door / Window |
| L / P / C | Line / Polyline / Circle |
| M / A | Distance measure / Angle measure |
| N | Dimension tool |
| Ctrl+Shift+S | Save As |
| Ctrl+S / Ctrl+O / Ctrl+N | Save / Open / New |
| Ctrl+Z / Ctrl+Y | Undo / Redo |
| Ctrl+C / Ctrl+V / Ctrl+D | Copy / Paste / Duplicate |
| Delete | Delete with dependency policy |
| R / Shift+R | Rotate selected free objects +90° / -90° |
| Ctrl+0 / Ctrl+2 | Fit / Fit selection |
| F3 / F8 / F9 | Snap / Ortho / Grid display |
| PageUp / PageDown | Previous/next existing floor |
| F1 | Context help |
| Ctrl+K | Command search |

Right-click opens context menu in Select. During opening preview it cycles preview orientation; after placement it must not secretly edit the last opening. Selected opening context menu offers explicit Flip Hinge and Flip Swing commands. Provide equivalent visible controls.

## Wall drawing details

First click anchors without creating a zero-length wall. Preview displays true length and angle. Next click uses the actual resolved snap/numeric endpoint. Connected placement reuses stable junction IDs. Double-click must not commit the same endpoint twice; test Qt's press/double-click sequence. Clicking the start junction closes and ends a loop. Escape does not erase earlier walls.

## Tool-specific drafting

Rectangle supports corner-corner and numeric width/depth. Circle supports center/radius. Arc supports center/start/end with explicit clockwise/counterclockwise sweep. Polygon closes on start or Enter. Offset shows side and distance; trim/extend requires selected boundary and target. Every semantic wall operation uses wall commands instead of treating walls as raw segments.

## Acceptance

Exercise keyboard focus in inspector, tool switch mid-drag, window focus loss, floor switch, overlapping selection, snap at all zooms, zoom while drawing, double-click termination, invalid numeric entry, and cancellation after pan suspension.
