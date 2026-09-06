# Step 00 — Environment and baseline audit

## Objective

Prove the toolchain and classify the existing repository before feature edits.

## Entry gate

No predecessor; start with actual repository/toolchain audit. Read AGENTS.md and docs/PROGRESS.md. Inspect actual source, Git status, relevant prior evidence, and the linked contracts before modifying code.

## Required reading

- [KA_LARA_ARCH_SAFE_ENV.md](../../KA_LARA_ARCH_SAFE_ENV.md)
- [docs/00_SOURCE_RECONCILIATION.md](../../docs/00_SOURCE_RECONCILIATION.md)

## Work package

Inspect Git and applicable instructions; record exact compiler/Qt/CMake/Ninja/SDK versions; build a minimal C++ test and Qt window; create dependency manifest; audit existing features against the requirement matrix.

Implement all listed tasks through the approved layer boundaries. Keep the application buildable; wire visible actions to functioning commands and record incomplete internal work explicitly while the step remains active.

## Acceptance evidence

Native Qt window opens; minimal test passes; source tree is clean of generated output; all existing feature claims have evidence or are marked unverified.

Record actual command lines, platform/build version, test results, numerical outputs, fixture IDs/hashes, and relevant UI screenshots in docs/evidence/STEP_00.md. Use fixed expected values from domain contracts. Distinguish PASS, FAIL, and NOT RUN.

## Scope guard

Do not rewrite working code or claim a non-Windows environment proves native UI.

## Failure handling and handoff

If a mandatory criterion fails, reproduce and fix it before advancing. If access prevents verification, leave this step BLOCKED with the precise missing environment/action; do not report it passed from code inspection. Update docs/PROGRESS.md with implemented versus remaining tasks, changes, risks, and next action. Commit focused changes under authorization. Continue to the next step only after this gate passes, unless the user's chosen review mode requires a pause.
