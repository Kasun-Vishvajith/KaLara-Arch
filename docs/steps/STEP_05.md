# Step 05 — Smooth 2D viewport and instrumentation

## Objective

Deliver readable model rendering, navigation, and performance measurement.

## Entry gate

Step 04 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/02_ARCHITECTURE.md](../../docs/02_ARCHITECTURE.md)
- [docs/06_UI_AND_VISUAL_DESIGN.md](../../docs/06_UI_AND_VISUAL_DESIGN.md)
- [docs/15_PERFORMANCE.md](../../docs/15_PERFORMANCE.md)

## Work package

Implement shared scene description, GL backend and painter fallback, camera-local coordinates, pan/zoom/fit/grid, DIP mapping, culling/index baseline, frame recorder, and visual tokens.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

Cursor-anchored zoom preserves the world point; grid and text are clear at 100/150/200%; fallback renders the same fixture; frame samples are recorded.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_05.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

No 3D camera or viewport object as persistent model truth.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
