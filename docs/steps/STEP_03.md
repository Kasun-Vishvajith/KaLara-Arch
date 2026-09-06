# Step 03 — Model identity and early serialization

## Objective

Create authoritative typed project state with stable identity and a tested codec.

## Entry gate

Step 02 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/03_MODEL_AND_SCHEMA.md](../../docs/03_MODEL_AND_SCHEMA.md)
- [docs/12_PROJECTS_AND_RECOVERY.md](../../docs/12_PROJECTS_AND_RECOVERY.md)

## Work package

Implement project/site/building/floor/layer/junction/wall base types; typed references; JSON Schema and validation; deterministic JSON writer/reader; valid and invalid fixtures; migration framework.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

Save/load produces semantic equality with identical IDs; missing references and unsupported schema reject; user intent distinguishes undecided from zero.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_03.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

JSON is an early codec, not completion of final portable archive/recovery.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
