# Step 07 — IN PROGRESS (2026-09-07)

## Verified partial implementation

- `WallAuthoring` creates one semantic chain segment per atomic command with explicit junction reuse only when the caller supplies a matching junction ID. It rejects zero length, invalid thickness, locked layers, missing scope, and coordinates outside ±10,000,000 mm.
- The rectangle helper creates four walls and four shared junctions in one command. The fixed fixture proves an exact 6000 × 4000 mm centerline enclosure with 24,000,000 mm² signed area and 200 mm default thickness.
- Connected junction movement, full-wall endpoint translation, and type/thickness/reference-line edits use the existing command service. Undo restores exact junction identity and position. A 0.001 mm long wall with 0.0001 mm positive thickness proves there is no invented 300 mm planning minimum.
- New documents contain a valid Building, Ground Floor, and unlocked Walls layer. W activates visible wall authoring; the first click only anchors, each later click commits one wall, preview linework shows true length/angle, endpoint snapping can reuse stable junction IDs, and Escape cancels the pending anchor without undoing committed segments.
- The inspector exposes wall type, thickness in millimetres, and center/left/right face preservation for a single selected wall. Each accepted edit is an atomic wall-properties command.
- The wall inspector now accepts a length, absolute `X,Y`, or relative `@dX,dY`. It commits through the same segment command, reports invalid input without mutation, and carries the newly allocated endpoint junction ID into the next chain segment. The UI test proves a 6000 mm segment followed by a relative segment shares one explicit junction.

## Checks run

- `tools/build.ps1 windows-debug`: PASS 8/8 after numeric entry and chain-ID propagation. `wall_authoring_tests` passes its exact rectangle, explicit topology, zero-length rejection, junction edit/Undo, thickness policy, and tiny-valid-wall assertions. `render_tests::wallToolInteraction` proves first-click nonmutation, exact 6000 mm numeric entry, invalid-input preservation, relative coordinate entry, shared-node incidence two, Escape preservation, and no double-click duplicate.
- Native Windows Step 07 interaction capture: NOT RUN after the latest inspector change.
- Windows release build: NOT RUN for the current partial revision.

## Remaining gate work

Derived corner joins still use overlapping wall rectangles and must suppress internal seams and prove L, diagonal, near-parallel, T, and X behavior. Rectangle UI entry, shared-node/segment drag previews, final native interaction evidence, release tests, and the focused passing commit remain. Step 07 is not PASS.
