# Step 13 — Scoped resize and scaling dialog

## Objective

Deliver the signature wall-referenced X/Y/uniform editing workflow completely.

## Entry gate

Step 12 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/08_SCALING_AND_SMART_EDITING.md](../../docs/08_SCALING_AND_SMART_EDITING.md)

## Work package

Implement all dialog scopes/anchors/policies, exact diagonal math, ghost preview, locks/hidden counts, position-only architectural resize, uniform geometry scale, explicit lossy conversion for unsupported affine semantics.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

All scaling-table examples pass; 900 mm opening preserved in layout resize; impossible target blocks; one Undo; Cancel hash unchanged; selected floors/site scope honored.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_13.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

This step is incomplete if only a factor field or wall-only resize exists.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
