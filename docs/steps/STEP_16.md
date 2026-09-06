# Step 16 — Floors, site, and calibrated underlays

## Objective

Support coherent multi-floor plans and site context strictly in 2D.

## Entry gate

Step 15 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/09_ARCHITECTURAL_OBJECTS.md](../../docs/09_ARCHITECTURAL_OBJECTS.md)
- [docs/13_INTERCHANGE_AND_PRINT.md](../../docs/13_INTERCHANGE_AND_PRINT.md)

## Work package

Explicit floor CRUD/copy categories; reference-only underlays; opt-in snap; site boundary/north/parking/landscape; configurable inset; raster/SVG calibration.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

Navigation never creates floor; copy remaps IDs; other floors remain unchanged on local edits; concave inset handles multi-regions; calibration changes only underlay.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_16.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

No cross-floor node sharing, GIS, or terrain model.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
