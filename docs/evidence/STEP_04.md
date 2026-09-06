# Step 04 — PASS (2026-09-07)

Source follows `0b7cd6f`. Fixture family `commands-v1`.

## Implemented and verified

- `ProjectStore` exposes shared immutable revision snapshots. A commit creates and validates a private candidate, then replaces the live snapshot once; readers holding an older snapshot retain an immutable view.
- `CommandService.prepare` checks project/revision metadata, captures before/after entity deltas, removes effective no-ops, rejects duplicate targets and ID mismatches, and produces a plan with read-only public access.
- Commit rechecks project/revision and each captured baseline, applies to a candidate, stamps entity/project revisions, runs dependency hooks and full domain validation, then publishes one committed event and one history entry.
- Undo/redo apply exact recorded deltas and preserve stable entity IDs. An effective edit after Undo clears redo; an identical edit does not. Saved semantic content is tracked independently of monotonically increasing revisions, so Undo back to saved content clears dirty state.
- History defaults to 256 MiB, estimates retained deltas, prunes oldest complete transactions, retains at least the newest transaction, and exposes its retained boundary.

## Commands and results

- `tools/build.ps1 core-tests`: PASS 3/3, including Qt-free architecture/runtime (`step04-core.log`).
- `build/core-tests/command_service_tests.exe`: PASS 141 checks (`step04-commands.txt`).
- `tools/build.ps1 windows-release`: PASS 5/5 with geometry, codec, command, and shell regressions (`step04-release.log`).

Numerical/behavior evidence: a junction moves from 6000 to 7500 mm in one commit/event/history entry; Undo restores 6000 and the exact `j2` ID; Redo restores 7500. One hundred plans prepared from the same base and discarded leave the snapshot pointer, revision, history, and events unchanged. An injected failure after member one of a five-entity candidate leaves all five live entities, revision, history, and event count untouched. A stale plan and dependency-hook error also leave the exact live snapshot untouched. A one-byte test budget prunes whole entries and exposes a nonzero retained boundary.

## Limits and next work

The service provides atomic entity deltas and hooks; operation-specific dependency expansion is added with each semantic feature. No editor tool can mutate persistent state yet. Opening/room/dimension dependency restoration belongs to their entity steps and is not claimed here. Save completion wiring is deferred to persistence steps; `markSaved(R)` already rejects any noncurrent revision. Step 05 adds scene extraction, painter/GPU-capable viewport selection, navigation, and measured frame instrumentation without rendering writes.
