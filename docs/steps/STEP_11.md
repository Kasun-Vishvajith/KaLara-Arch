# Step 11 — Distance, angle, dimensions, and annotations

## Objective

Make measurement and documentation numerically trustworthy.

## Entry gate

Step 10 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/11_DIMENSIONS_AND_DOCUMENTATION.md](../../docs/11_DIMENSIONS_AND_DOCUMENTATION.md)

## Work package

Temporary distance/angle/area tools; Pin; reporting aligned/horizontal/vertical/angular/radius/diameter/chain dimensions; stable references; styles/tags/notes.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

3-4-5 and 90-degree fixtures pass; referenced movement updates values; orphan state is visible; paper-sized labels render correctly at two scales.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_11.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

Driving dimension UI waits for supported constraint commands in Step 12.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
