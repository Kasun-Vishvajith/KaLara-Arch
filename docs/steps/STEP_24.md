# Step 24 — Performance and stability hardening

## Objective

Prove and achieve responsiveness and bounded resource use.

## Entry gate

Step 23 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/15_PERFORMANCE.md](../../docs/15_PERFORMANCE.md)
- [docs/16_TESTING_AND_RELEASE.md](../../docs/16_TESTING_AND_RELEASE.md)

## Work package

Run fixed-seed fixtures in Release; capture frame/input/save/open/memory data; profile bottlenecks; optimize culling/cache/deltas; stress undo/open/close; graphics fallback recovery.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

Required p95/p99 budgets and memory limits pass on recorded hardware; no unbounded growth; fallback remains functional; raw evidence stored.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_24.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

No lowering fixture size or hiding expensive content to fabricate success.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
