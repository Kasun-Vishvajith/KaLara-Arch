# Step 23 — Visual polish, accessibility, and onboarding

## Objective

Reach the specified finished desktop quality.

## Entry gate

Step 22 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/06_UI_AND_VISUAL_DESIGN.md](../../docs/06_UI_AND_VISUAL_DESIGN.md)
- [docs/07_TOOLS_AND_INPUT.md](../../docs/07_TOOLS_AND_INPUT.md)

## Work package

Review all required screenshots; refine light/dark tokens, spacing, icon states, panel density, focus, help, empty/error states, high DPI, keyboard paths, reduced motion.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

No clipped controls at supported sizes; G13 passes; empty/furnished/scaling/sheet/recovery screens are consistent; essential inspector flow works by keyboard.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_23.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

No decorative redesign that breaks tool consistency or adds 3D/AI tabs.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
