# Step 22 — Integrated validation and full workflow QA

## Objective

Find cross-feature failures and make diagnostics actionable.

## Entry gate

Step 21 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/14_VALIDATION_AND_CONSTRAINTS.md](../../docs/14_VALIDATION_AND_CONSTRAINTS.md)
- [docs/16_TESTING_AND_RELEASE.md](../../docs/16_TESTING_AND_RELEASE.md)

## Work package

Run all golden workflows; complete geometry/room/clearance/site/documentation checks; fix topology/reference/cache integration defects; verify revision-safe diagnostics.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

G01–G12 pass numerically and interactively; no stale room/dimension result shown as current; diagnostic jump targets correct; fixes are undoable.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_22.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

Do not downgrade failed gates to advisory to advance.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
