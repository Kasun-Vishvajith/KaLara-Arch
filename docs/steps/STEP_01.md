# Step 01 — Layered application foundation

## Objective

Establish a buildable, well-organized desktop shell and dependency boundaries.

## Entry gate

Step 00 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/02_ARCHITECTURE.md](../../docs/02_ARCHITECTURE.md)
- [docs/06_UI_AND_VISUAL_DESIGN.md](../../docs/06_UI_AND_VISUAL_DESIGN.md)

## Work package

Create or map CMake targets; implement logging/error types; per-document session ownership; ActionRegistry; empty themed main window with docks/status bar; add Reset Workspace.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

Qt-free core compiles independently; application opens/closes without errors; actions use one registry; dock layout persists and resets.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_01.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

No runtime AI, alternative UI stack, or architectural editing stubs presented as complete.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
