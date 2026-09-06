# Step 10 — Transforms, clipboard, groups, and layers

## Objective

Support daily editing and plan organization consistently.

## Entry gate

Step 09 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/04_COMMANDS_AND_HISTORY.md](../../docs/04_COMMANDS_AND_HISTORY.md)
- [docs/07_TOOLS_AND_INPUT.md](../../docs/07_TOOLS_AND_INPUT.md)

## Work package

Move/rotate/mirror gizmos; duplicate/copy/paste remapping; grouping; align/distribute; layer/category hide/lock/print; multi-selection inspector mixed state.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

Paste wall+opening remaps host; mirror keeps door semantics; rotated grips work in local axes; locked mixed selection diagnoses conflict; one Undo per operation.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_10.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

Do not treat nonuniform scale as ordinary rotation/width editing.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
