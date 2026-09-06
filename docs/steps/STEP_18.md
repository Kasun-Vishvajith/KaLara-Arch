# Step 18 — Portable native files and recovery

## Objective

Protect real user work and make custom assets portable.

## Entry gate

Step 17 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/12_PROJECTS_AND_RECOVERY.md](../../docs/12_PROJECTS_AND_RECOVERY.md)

## Work package

ZIP .kla writer/reader with manifest/hashes; embedded definitions/assets; atomic save; failure handling; autosave generations; recovery chooser; session/file locks; legacy JSON migration.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

Round-trip custom assets on another location; disk failure preserves original; newer edit stays dirty; corrupt recovery falls back; recovered copy doesn't overwrite original.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_18.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

No single TEMP autosave or silent version loss.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
