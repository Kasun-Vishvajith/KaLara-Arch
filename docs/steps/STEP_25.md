# Step 25 — Windows distribution and offline validation

## Objective

Package a usable application independent of the development machine.

## Entry gate

Step 24 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [KA_LARA_ARCH_SAFE_ENV.md](../../KA_LARA_ARCH_SAFE_ENV.md)
- [docs/16_TESTING_AND_RELEASE.md](../../docs/16_TESTING_AND_RELEASE.md)
- [docs/19_REFERENCES_AND_DEPENDENCIES.md](../../docs/19_REFERENCES_AND_DEPENDENCIES.md)

## Work package

Release build; portable package then installer; Qt/runtime dependencies; notices; checksums; standard-user data paths; clean-machine test disconnected from internet.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

G14 passes without Python/build tools/developer PATH; assets/help/printing work; non-ASCII path passes; uninstall preserves projects; dependency notices correct.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_25.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

A Linux build or developer-machine launch cannot pass this step.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
