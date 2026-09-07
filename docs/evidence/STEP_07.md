# Step 07 — PASS (2026-09-07)

Source commits `107569f`, `855df77`, and the completing commit that follows. Fixtures `wall-authoring-v1` and render join networks. SHA-256: `tests/wall_authoring_tests.cpp` `5B3AA3CAD45EFB907F428D2707F08C0457DC386C2E1E82E295E5B77D442BD7A6`; `tests/render_tests.cpp` `7793581DEFDD6CE752F7E037159EDBA9B8406FFC8754FE33E57DBE4ADC7F76A3`.

## Implemented and verified

- `WallAuthoring` creates one semantic chain segment per atomic command. It reuses a junction only when the caller supplies its matching stable ID, and rejects zero length, invalid thickness, locked layers, missing scope, and coordinates outside ±10,000,000 mm.
- The rectangle helper creates four walls and four shared junctions in one command. The fixture proves an exact 6000 × 4000 mm centerline enclosure, 24,000,000 mm² centerline area, and 200 mm default thickness.
- Connected junction movement, full-wall translation, and type/thickness/reference-line edits use `CommandService`. Undo restores exact IDs and positions. A 0.001 mm wall with 0.0001 mm positive thickness proves there is no invented 300 mm minimum.
- New documents contain a valid Building, Ground Floor, and unlocked Walls layer. W activates wall authoring; first click anchors without mutation, later clicks commit individual walls, the preview shows true length/angle, explicit endpoint snaps carry the shared ID, and Escape keeps committed segments.
- Numeric input accepts length, absolute `X,Y`, and relative `@dX,dY`; invalid input does not mutate the model. A 6000 mm segment followed by relative entry shares one explicit junction. The rectangle dialog accepts four numeric coordinates and creates its enclosure in one command.
- The inspector exposes wall type, thickness, and center/left/right reference preservation. Each accepted change is one atomic properties command.
- Qt-free scene extraction derives joined outlines by bounded endpoint extension, edge intersection splitting, and filled-union boundary classification. L, diagonal, near-parallel, T, and X fixtures prove emitted outline segments separate fill from empty space and remain inside the four-thickness miter bound.
- Selected wall and shared-junction movement renders a dashed preview from committed state. Mouse movement leaves the snapshot pointer unchanged; Escape discards the draft; release commits one command.

## Commands and results

- `tools/build.ps1 windows-debug`: PASS 8/8 (`step07-debug.log`). `wall_authoring_tests` passes 20 checks; render tests additionally cover five join networks and all wall UI behaviors.
- Native Windows `render_tests.exe`, Qt 6.7.2 on PATH with `QT_QPA_PLATFORM=windows`: PASS 12/12 (`step07-render-native.txt`).
- Native Windows `shell_tests.exe`: PASS 5/5 (`step07-shell-native.txt`), including W routing, rectangle action availability, and editor focus isolation.
- `tools/build.ps1 windows-release`: PASS 8/8 (`step07-release.log`).
- `step07-wall-tool.png` was visually inspected: two joined semantic segments have a continuous external outline with no internal seam. SHA-256 `55573DC19423FE6E154DB7759F15DBD0CA6B7977A13505DF67233B0CF4198563`.

The native shell run requested 1920 × 1080 DIP but the 125% desktop constrained it to 1539 × 844 DIP. Its mixed capture/resize samples are instrumentation only and are not a Step 24 performance claim.

## Limits and next work

Boundary derivation currently compares wall shapes quadratically; Step 24 must use spatial neighborhoods and prove performance. Interior split, compatible merge, hosted opening interval remapping, and real opening cutouts belong to Step 08.
