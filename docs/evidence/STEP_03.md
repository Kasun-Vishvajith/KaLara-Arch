# Step 03 — PASS (2026-09-06)

Source follows 30a4357. Fixture family `model-json-v1`.

## Implemented and verified

- Qt-free authoritative `Project` with opaque stable IDs and typed site, building, floor, layer, junction, and straight-wall records. Entity headers carry owner, optional layer, type, and revision metadata.
- Explicit typed references and validation for existence, referenced type, floor/building ownership, wall scope, distinct/noncoincident junctions, positive thickness, revisions, display precision, design-envelope geometry, polygon validity, and intent state/value consistency.
- Intent distinguishes `known` zero, `undecided`, and `omitted`. Project extension metadata is inert namespaced string data and survives a round trip.
- Deterministic UTF-8 JSON writer ordered by stable entity ID, candidate reader that never mutates an active project, structured diagnostics, and an explicit migration entry point. Version 1 is the first editable version; unsupported versions reject rather than being guessed.
- JSON Schema draft 2020-12 at `persistence/schema/project.schema.json`; codec validates the same required root structures and domain references before activation.

## Commands and results

- `tools/build.ps1 core-tests`: PASS 2/2. This configuration compiles `kalara_architecture` without Qt and runs core/geometry regression tests (`step03-core.log`).
- `tools/build.ps1 windows-debug`: PASS 4/4 (`step03-debug.log`).
- `build/windows-debug/model_codec_tests.exe tests/fixtures`: PASS 24 checks; 2965-byte valid JSON output (`step03-model.txt`).
- `tools/build.ps1 windows-release`: PASS 4/4 (`step03-release.log`).

The round trip preserves exact IDs, `6000.125` mm, Unicode title text, extension metadata, and all three intent states. Tests reject malformed JSON, duplicate IDs, missing references, wrong typed references, unknown root properties, wrong Boolean types, invalid same-junction walls, and schema version 99. The schema file is parsed as JSON during the test and its draft declaration is checked.

Fixture SHA-256 values:

- valid v1: `E3EFA86E74B88A996A8790B124FC1FF54A92453C30555CF2A8A2436399C585E4`
- missing reference: `8C4E1685F25067F6D57AEA50A7CAB3A9E45737BA5BED881E6B830EA7705FE607`
- unsupported v99: `2C5256E361ED3239C4BD144633AB44234C32A2201C84736F382360209512F1A0`
- schema: `F153E949D0F399B61D989736C8498B6BD2FE7494F59D98425CA1ED2B0D2C0DDA`

## Limits and next work

This JSON codec is the early semantic codec. Atomic files, archive limits, embedded assets, recovery, and `.kla` packaging remain MISSING until Step 18. The migration framework intentionally contains no invented pre-v1 migration. Openings and later entity families enter with their owning feature steps. Step 04 adds the sole mutation path, immutable snapshots, transactions, history, and dirty tracking.
