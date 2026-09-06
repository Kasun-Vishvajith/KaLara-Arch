# Step 12 — Constraints and connected edit policies

## Objective

Enforce explicit constraints and explain conflicts without hidden changes.

## Entry gate

Step 11 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/14_VALIDATION_AND_CONSTRAINTS.md](../../docs/14_VALIDATION_AND_CONSTRAINTS.md)
- [docs/08_SCALING_AND_SMART_EDITING.md](../../docs/08_SCALING_AND_SMART_EDITING.md)

## Work package

Implement minimum constraint set and bounded deterministic solve; fixed/under/overconstrained outcomes; endpoint anchor policies; driving dimensions; diagnostics panel and revision handling.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

Constrained rectangle resizes predictably; conflicting fixed data rejects unchanged; underconstrained layout stays near baseline; supported combinations pass.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_12.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

No universal solver claim or automatic constraint deletion.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
