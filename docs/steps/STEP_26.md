# Step 26 — LOCAL_2D_RELEASE_GATE

## Objective

Close the complete local 2D product with traceable evidence.

## Entry gate

Step 25 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/16_TESTING_AND_RELEASE.md](../../docs/16_TESTING_AND_RELEASE.md)
- [docs/18_REQUIREMENTS_MATRIX.md](../../docs/18_REQUIREMENTS_MATRIX.md)

## Work package

Audit every requirement and prior gate; run final golden walkthrough; fix remaining critical/high issues; generate release notes, supported limits, known minor issues, checksums and user quick-start.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

All requirements and G01–G14 have evidence; performance/Windows/CAD/visual gates pass; no mandatory placeholder; signed-off release record names tested build.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_26.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

If any mandatory evidence is unavailable, report BLOCKED release candidate. Do not start AI or 3D work.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
