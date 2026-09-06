# Step 02 — Units and robust 2D geometry

## Objective

Deliver the numerical foundation that every measurement and transform uses.

## Entry gate

Step 01 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/05_GEOMETRY_AND_TOPOLOGY.md](../../docs/05_GEOMETRY_AND_TOPOLOGY.md)

## Work package

Implement typed units, parser/formatter, points/vectors/segments/arcs/polygons, transforms, predicates, bounds; distinguish numeric/merge/screen tolerances; pin polygon dependency if used.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

Feet-inch parsing and conversion pass; 3-4-5 distances and rectangle/hole areas match fixed expectations; inverse transforms round-trip; invalid/nonfinite values reject.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_02.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

No premature viewport-specific geometry or integer rounding of canonical data.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
