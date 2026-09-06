# Step 08 — Topology editing and hosted openings

## Objective

Complete wall split/merge and editable doors/windows with actual host semantics.

## Entry gate

Step 07 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/05_GEOMETRY_AND_TOPOLOGY.md](../../docs/05_GEOMETRY_AND_TOPOLOGY.md)
- [docs/09_ARCHITECTURAL_OBJECTS.md](../../docs/09_ARCHITECTURAL_OBJECTS.md)

## Work package

Split/remap references; compatible merge; host preview/fit; procedural door/window types; along-wall move/width grips; hinge/swing commands; delete dependency plan.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

Split before opening remaps correctly; split through opening rejects; opening follows wall and reverses consistently; overrun/overlap rejects; Undo restores all.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_08.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

No background-color fake cutouts or free-rotating hosted openings.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
