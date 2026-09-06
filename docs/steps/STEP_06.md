# Step 06 — Selection, tool lifecycle, and snapping

## Objective

Make input predictable and selection reliable across the editor.

## Entry gate

Step 05 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/07_TOOLS_AND_INPUT.md](../../docs/07_TOOLS_AND_INPUT.md)

## Work package

ToolController states; selection/marquee/cycling; screen-space hit targets; semantic snap candidates/hysteresis; ortho/polar; keyboard focus routing; preview cancellation.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

Focus loss/tool switch cancels draft; crossing/enclosed marquee differs correctly; locks/hidden filters work; snap target stays stable across zoom.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_06.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

Do not wire separate shortcuts with inconsistent handlers.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
