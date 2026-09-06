# Step 20 — Vector and image export plus printing

## Objective

Produce high-quality documents from committed state.

## Entry gate

Step 19 passed, or all its requirements verified in the baseline audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [docs/13_INTERCHANGE_AND_PRINT.md](../../docs/13_INTERCHANGE_AND_PRINT.md)

## Work package

PDF/SVG/JSON/PNG/JPEG exporters; print preview/settings; offscreen scene generation; font substitution; progress/cancel; warnings/loss report; atomic output where practical.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

6000 mm line is 60 mm in 1:100 PDF; SVG physical size correct; raster resolution/background correct; no editor overlays; printing clipping detected.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_20.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

No framebuffer screenshot passed off as drawing export.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
