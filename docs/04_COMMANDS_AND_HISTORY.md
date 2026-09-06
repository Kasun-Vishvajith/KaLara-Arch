# Commands, previews, history, and atomic change

## Single mutation path

Every persistent change uses CommandService: creation, deletion, property edits, topology, templates, library placement, settings, constraints, sheets, and imports. Selection, pan, hover, temporary measurement, and UI theme preference are session operations and do not pollute document undo.

Request → resolve base revision → expand dependency closure → draft → solve supported constraints → validate → show preview when needed → atomic commit → invalidate derived data → history entry → committed event.

## Request and result contracts

CommandRequest: command ID, project ID, expected revision, actor=human, label, targets, typed parameters, scope, policy choices.

ChangePlan: base revision, affected entity IDs, before/after deltas, created/deleted IDs, dependency remaps, warnings/errors, bounds before/after, policy summary. A plan is immutable once offered for commit; changing a dialog option prepares a new plan.

Result: committed revision/history ID and changed IDs, or structured failure. Zero effective delta produces no history item. Stale revision rejects; UI may prepare again and ask only if the user-visible result changes materially.

## Required operation families

| Family | Representative operations |
|---|---|
| Project | CreateProject, UpdateIntent, AddFloor, DeleteFloor, SetDisplayUnits |
| Geometry | CreateDraftEntity, MoveEntities, RotateEntities, MirrorEntities, Offset, TrimExtend |
| Walls | DrawWallSegment, MoveJunction, SplitWall, MergeJunctions, SetWallThickness |
| Openings | PlaceOpening, MoveOpening, ResizeOpening, FlipHinge, FlipSwing |
| Smart edit | ResizeLayout, ScaleGeometry, SetDrivingDimension |
| Library | PlaceInstance, SetInstanceParameters, CreateUserBlock |
| Documentation | PlaceDimension, ChangeStyle, CreateSheet, UpdateSheetViewport |
| Organization | SetLayerState, GroupEntities, DuplicateEntities, ImportEntities |

Do not expose every function as a public network API. These are in-process application commands.

## Drag lifecycle

At press, capture base revision and immutable affected state. Pointer moves compute candidate geometry from the same baseline, not from the previous preview. This avoids cumulative drift and rotation compounding. Preview rendering is separate from ProjectStore.

At release, prepare/validate the final operation, commit once, and dispose preview. Esc, focus loss, tool change, floor switch, modal close, or document close cancels an uncommitted drag. Do not autosave a preview. While a dialog awaits decision, background jobs cannot mutate its baseline.

## History policy

One wall segment is one command. Double-click finishes the chain without a duplicate terminal wall. Esc discards its pending segment and keeps previously committed segments. One drag, one numeric inspector commit, one paste, one template insertion, and one scoped scaling apply are each one history item.

Use before/after deltas and topology mappings. Redo restores exact recorded state rather than re-solving against new rounding. Undo restores identities and references. A new effective edit after Undo clears redo. Setting identical values does not clear redo.

Default history budget: 256 MiB per open document, with a visible retained-history boundary. Prune oldest complete transactions, never half of a grouped command. Do not promise unlimited undo. Save does not clear history; opening a file starts a new session history unless explicit persisted history is implemented later outside requirements.

## Save and dirty state

Track a saved semantic content identity plus history position. Undo back to saved state clears dirty only when semantic equality holds. A save finishing on revision R does not mark newer R+1 clean. Failed save preserves dirty state.

## Failure and test cases

- Inject failure after one member of a five-entity change: no partial state, no history entry, no committed event.
- Drag 100 move events then Esc: semantic hash and history unchanged.
- Move connected junction plus openings: one Undo restores all IDs, positions, offsets, room metadata.
- Undo deletion of host restores host/openings/dimension references.
- Stale ChangePlan rejects instead of clobbering a newer edit.
- Copy a wall with its opening: paste remaps host ID and selects the new instances.
