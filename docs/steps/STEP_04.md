# Step 04 — Transactions and undo before authoring

## Objective

Make persistent editing atomic and cancelable before rich tools depend on it.

## Entry gate

Step 03 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/04_COMMANDS_AND_HISTORY.md](../../docs/04_COMMANDS_AND_HISTORY.md)

## Work package

Implement ProjectStore snapshots, ChangePlan preparation/commit, revision checks, deltas, history, dirty tracking, dependency hooks, and failure injection tests.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

One command yields one event/history entry; injected partial failure rolls back; Undo/Redo restores IDs; stale plans reject; no-op leaves history unchanged.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_04.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

No direct editor writes or whole-project serialization on pointer movement.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
