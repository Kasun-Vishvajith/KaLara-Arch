# Step 15 — Rooms, columns, stairs, and fixtures

## Objective

Complete semantic space planning and remaining architectural plan objects.

## Entry gate

Step 14 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/09_ARCHITECTURAL_OBJECTS.md](../../docs/09_ARCHITECTURAL_OBJECTS.md)
- [docs/05_GEOMETRY_AND_TOPOLOGY.md](../../docs/05_GEOMETRY_AND_TOPOLOGY.md)

## Work package

Derived/manual rooms with holes; area/perimeter; split/merge identity; columns; parameterized straight/L/U stairs and ramps; clearance envelopes; integrated inspector.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

6000×4000 centerline room gives 22.04 m²; door gaps don't break room; courtyard subtracts area; room metadata survives edits; stairs update predictably.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_15.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

No engineering compliance or 3D interpretation.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
