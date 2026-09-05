# KaLara Arch — Development Control Documents

This folder contains the initial controlled-development documents for KaLara Arch.

## Files

### `AGENTS.md`

The authoritative development controller for AI coding agents.

It defines:

- product identity
- architecture
- architectural data model principles
- units and precision
- semantic objects
- mutation rules
- export strategy
- AI-agent integration philosophy
- development safety rules
- checkpoint protocol
- controlled roadmap
- V1 acceptance workflows

### `KA_LARA_ARCH_SAFE_ENV.md`

The safe development environment specification.

It defines:

- recommended repository layout
- Windows-first development environment
- filesystem safety
- Git rules
- CMake/toolchain rules
- Qt dependency policy
- Python virtual environment
- dependency reproducibility
- Step 00 environment acceptance criteria

## Starting point

Place both documents at the root of the KaLara Arch repository:

```text
KaLaraArch/
├── AGENTS.md
├── KA_LARA_ARCH_SAFE_ENV.md
└── ...
```

The first development instruction is:

```text
STEP 00 — SAFE ENVIRONMENT
```

The agent must stop at the Step 00 checkpoint and wait for:

```text
CONTINUE
```
