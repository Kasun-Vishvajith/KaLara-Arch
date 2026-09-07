# Implementation progress

**Mode:** Autonomous after explicit pack adoption; review mode only if user requests it.
**Current step:** 09 — IN PROGRESS.
**Implementation status:** Steps 00–08 verified. Hosted doors/windows now have true wall cutouts, procedural symbols, fit previews, tangent grips, split/merge/reversal remapping, dependency deletion, persistence, and atomic Undo.
**Baseline repository:** 868dc8b; no tracked application source.
**Last verified build/test:** Step 08 debug/release CTest 9/9; focused topology 23 checks; codec 24 checks; native render/interaction 14/14.
**Release status:** NOT VERIFIED.

## Next action

Implement Step 09 general 2D drafting and editing as a usable command-backed vertical slice.

## Session handoff — update after each run

- Active step and status: Step 09 IN PROGRESS.
- Latest implementation commit: Step 08 commit follows `2ecd8ca`; use git log.
- Changes actually implemented: Step 08 hosted opening model, atomic topology commands, actual cutouts, procedural door/window symbols, hover fit preview, tangent grips, dependency deletion, and JSON/schema persistence.
- Tests actually run and results: Step 08 debug/release 9/9; topology 23 checks; codec 24 checks; native render/interaction 14/14.
- Windows UI evidence: docs/evidence/step08-opening-preview.png.
- Open defects/blockers: larger native capture constrained by display; no active geometry blocker.
- Files with uncommitted user changes: none detected; supplied pack adopted in baseline.
- Next concrete action: implement Step 09 general drafting entities, tools, editing, and tests.
- Relevant specification/ADR: docs/steps/STEP_09.md and its linked drafting/editing contracts.
- User-selected workflow changes: autonomous, ordinary implementation/build/test/commits authorized.

## Step ledger

| Step | Status | Evidence |
|---|---|---|
| 00 — Environment and baseline audit | PASS | [STEP_00](evidence/STEP_00.md) |
| 01 — Layered application foundation | PASS | [STEP_01](evidence/STEP_01.md) |
| 02 — Units and robust 2D geometry | PASS | [STEP_02](evidence/STEP_02.md) |
| 03 — Model identity and early serialization | PASS | [STEP_03](evidence/STEP_03.md) |
| 04 — Transactions and undo before authoring | PASS | [STEP_04](evidence/STEP_04.md) |
| 05 — Smooth 2D viewport and instrumentation | PASS | [STEP_05](evidence/STEP_05.md) |
| 06 — Selection, tool lifecycle, and snapping | PASS | [STEP_06](evidence/STEP_06.md) |
| 07 — Architectural wall authoring | PASS | [STEP_07](evidence/STEP_07.md) |
| 08 — Topology editing and hosted openings | PASS | [STEP_08](evidence/STEP_08.md) |
| 09 — General 2D drafting and editing | IN PROGRESS | Pending |
| 10 — Transforms, clipboard, groups, and layers | NOT STARTED | Pending |
| 11 — Distance, angle, dimensions, and annotations | NOT STARTED | Pending |
| 12 — Constraints and connected edit policies | NOT STARTED | Pending |
| 13 — Scoped resize and scaling dialog | NOT STARTED | Pending |
| 14 — Complete semantic library and SVG pipeline | NOT STARTED | Pending |
| 15 — Rooms, columns, stairs, and fixtures | NOT STARTED | Pending |
| 16 — Floors, site, and calibrated underlays | NOT STARTED | Pending |
| 17 — Templates, settings, and first-run workflow | NOT STARTED | Pending |
| 18 — Portable native files and recovery | NOT STARTED | Pending |
| 19 — Sheets, styles, and schedules | NOT STARTED | Pending |
| 20 — Vector and image export plus printing | NOT STARTED | Pending |
| 21 — DXF import/export and independent CAD proof | NOT STARTED | Pending |
| 22 — Integrated validation and full workflow QA | NOT STARTED | Pending |
| 23 — Visual polish, accessibility, and onboarding | NOT STARTED | Pending |
| 24 — Performance and stability hardening | NOT STARTED | Pending |
| 25 — Windows distribution and offline validation | NOT STARTED | Pending |
| 26 — LOCAL_2D_RELEASE_GATE | NOT STARTED | Pending |



