# Step 07 — Architectural wall authoring

## Objective

Draw and edit semantic connected walls as a complete vertical slice.

## Entry gate

Step 06 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/05_GEOMETRY_AND_TOPOLOGY.md](../../docs/05_GEOMETRY_AND_TOPOLOGY.md)
- [docs/09_ARCHITECTURAL_OBJECTS.md](../../docs/09_ARCHITECTURAL_OBJECTS.md)

## Work package

Wall chain and rectangle helper; numeric input; junction reuse; derived faces/joins; type/thickness inspector; shared-node and segment move previews; one segment per command.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

Draw enclosure with exact dimensions; no duplicate double-click endpoint; Esc keeps committed segments; joins render correctly; Undo restores topology.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_07.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

No 300 mm hard minimum or proximity-only permanent connectivity.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
