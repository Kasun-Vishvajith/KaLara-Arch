# Step 14 — Complete semantic library and SVG pipeline

## Objective

Ship useful built-in objects without any external artwork dependency.

## Entry gate

Step 13 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/10_LIBRARY_AND_SVG.md](../../docs/10_LIBRARY_AND_SVG.md)
- [library/README.md](../../library/README.md)

## Work package

Implement catalog schema/loader; minimum starter items and procedural generators; thumbnails/search/favorites; placement/gizmos; SVG validation/normalization; user blocks; pinned instance versions.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

Empty SVG directories still yield usable recognizable library; valid replacement has exact size; hostile SVG rejects; user block remaps IDs; asset update Undo works.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_14.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

Category initials are not acceptable built-in furniture artwork.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
