# Step 09 — General 2D drafting and editing

## Objective

Provide professional basic drafting beyond semantic walls.

## Entry gate

Step 08 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/07_TOOLS_AND_INPUT.md](../../docs/07_TOOLS_AND_INPUT.md)
- [docs/05_GEOMETRY_AND_TOPOLOGY.md](../../docs/05_GEOMETRY_AND_TOPOLOGY.md)

## Work package

Line/polyline/rectangle/circle/arc/polygon; text; guides; offset; trim/extend on supported lines/arcs; polygon hatch; command-backed geometry and inspector.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

Numeric primitives match dimensions; offset/trim fixtures pass; open versus closed entities behave correctly; Cancel and Undo work on each tool.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_09.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

No spline or curved-semantic-wall promise; raw lines remain raw.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
