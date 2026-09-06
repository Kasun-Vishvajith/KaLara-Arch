# Step 21 — DXF import/export and independent CAD proof

## Objective

Deliver the declared CAD exchange subset with measured interoperability.

## Entry gate

Step 20 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/13_INTERCHANGE_AND_PRINT.md](../../docs/13_INTERCHANGE_AND_PRINT.md)

## Work package

Implement AC1015 subset/layers/units; blocks with nesting limits; exploded dimensions report; import preview/unit decision/unsupported counts; independent reader and external viewer checks.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

Independent parse gives exact 6000/900/200 mm fixture values; viewer confirms scale/layers; unsupported input reports loss; accepted import is one Undo.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_21.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

No DWG, broad compatibility claim, or semantic inference from imported layers.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
