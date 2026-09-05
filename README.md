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
- project initialization and template-system direction

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
- Step 00 scope boundary for later product features

## Current product-design addition

The controlled documents now also define a **Project Initialization + Template System** direction. KaLara Arch may begin a project from an empty canvas or from a structured template. The template workflow can ask a compact set of questions about building type, site shape, terrain, floors, measurement system, target building area, and programme requirements, while allowing each unknown item to remain explicitly `Not decided yet`.

The answers are intended to become structured project intent and initial project context. This should reduce repetitive human setup and later reduce unnecessary external AI-agent discovery, token usage, and API calls. Templates are structured starting configurations, not image-only presets, and must converge into the same architectural model and controlled mutation system used by normal human operations.

This is a product-design decision recorded for future roadmap steps; it does not authorize implementation during Step 00.

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
