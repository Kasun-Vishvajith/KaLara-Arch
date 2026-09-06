# Step 19 — Sheets, styles, and schedules

## Objective

Turn plans into composed professional drawing documents.

## Entry gate

Step 18 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/11_DIMENSIONS_AND_DOCUMENTATION.md](../../docs/11_DIMENSIONS_AND_DOCUMENTATION.md)

## Work package

Paper presets/margins/title blocks; viewport crop/scale; model versus paper coordinates; notes/legends/clouds; room/opening schedules and CSV; paginated sheet tables.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

Two viewports at 1:50 and 1:100 retain model truth and paper label size; schedules update after change/Undo; no clipped table rows.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_19.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

Viewport zoom must not control print scale.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
