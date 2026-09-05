# KaLara Arch — Safe Development Environment

## Purpose

This document defines the recommended isolated development environment for building KaLara Arch with AI coding agents.

It is separate from the final KaLara Arch runtime architecture.

KaLara Arch is a Windows-first V1 desktop architectural planning system, but the development environment should avoid unnecessary platform lock-in where practical.

## Recommended layout

```text
D:\Dev\KaLaraArch\
├── .git\
├── AGENTS.md
├── CMakeLists.txt
├── CMakePresets.json
├── core\
├── architecture\
├── runtime\
├── editor\
├── exporters\
├── library\
├── tests\
├── tools\
│   └── python\
├── docs\
├── third_party\
├── build\
├── out\
└── .gitignore
```

Use another user-owned directory when necessary. The important rule is that the project has a clear filesystem boundary.

## Safety rules

The AI development agent should have permission to work inside the KaLara Arch repository.

It should not have unrestricted permission to modify the operating system.

Avoid administrator shells for normal development.

Do not allow development scripts to:

- delete files outside the repository
- modify the Windows registry
- change security settings
- disable antivirus/firewall
- install arbitrary software
- execute unknown downloads
- modify unrelated projects
- modify boot configuration
- make destructive filesystem changes outside the repository

## Repository boundary

The repository is the primary writable boundary.

Any operation outside the repository must be explicitly required by the current development step or separately approved by the human.

## Reproducibility

Keep build configuration in the repository.

Use:

- CMake
- CMake presets
- a reproducible C++ compiler/toolchain
- Qt through an approved reproducible dependency strategy
- a repository-controlled dependency strategy
- project-local Python virtual environments

Avoid global Python package installation.

## Source control

Initialize Git before substantial implementation.

Every checkpoint should leave the repository in a recoverable state.

Recommended commit pattern:

```text
step-00: establish safe development environment
step-01: establish project foundation
step-02: establish geometry and units
```

## Build isolation

Keep generated files under:

```text
build/
out/
dist/
```

Do not commit generated binaries unless the release process explicitly requires them.

Do not generate temporary build artifacts inside source directories.

## Dependency policy

Record direct dependencies and versions in:

```text
docs/dependencies.md
```

Do not update dependencies merely to resolve unrelated warnings.

Treat dependency changes as controlled development changes.

## Qt policy

Qt is the approved primary desktop UI framework for KaLara Arch.

The specific supported Qt version must be pinned and recorded in `docs/dependencies.md` when the project is initialized.

Do not silently switch desktop UI frameworks because of an isolated convenience issue.

If a framework change is believed necessary, use the architecture-change protocol in `AGENTS.md`.

## C++ compiler/toolchain policy

The selected Windows compiler/toolchain must be explicitly recorded during Step 00.

The exact:

- compiler family
- compiler version
- architecture
- CMake version
- generator/build preset

must be documented sufficiently for another developer to reproduce the environment.

Do not casually switch compilers after the project foundation is established.

## Python environment

Use:

```text
tools/python/.venv/
```

for project Python tools.

Create with:

```text
python -m venv tools/python/.venv
```

Install project dependencies only into that environment.

Do not modify the system Python installation for normal project development.

## Native geometry requirements

The project must establish and document:

```text
canonical unit = millimetre
nominal precision = 1 mm
local coordinate system = project-defined
```

User display units may differ from the canonical internal unit.

The display unit setting must not silently rescale native project geometry.

## Filesystem-safe development commands

Prefer commands whose effects are clearly contained inside the repository.

Before running a destructive or broad command, verify the current working directory.

Especially avoid unrestricted commands such as:

```text
recursive delete from a parent directory
system-wide package removal
registry edits
security configuration changes
```

Do not use destructive commands as shortcuts for normal build cleanup when a project-local cleanup is sufficient.

## AI-agent safety policy

During development, the AI agent should use a permission policy equivalent to:

### Allowed automatically

- read repository files
- inspect repository state
- inspect Git history
- edit repository files
- create repository-local directories
- run approved project-local build/test commands
- run approved project-local Python tooling
- create Git commits for completed checkpoints

### Approval required

- install system software
- administrator privileges
- commands that modify files outside the repository
- changing operating-system configuration
- changing security configuration
- deleting large directory trees
- downloading and executing unknown programs
- changing firewall/antivirus behavior
- changing developer machine credentials or secrets
- modifying unrelated repositories

## Secrets

Never put the following into the repository:

```text
API keys
passwords
access tokens
private credentials
personal secrets
```

Use environment variables or a properly ignored `.env` file only when a specific development feature requires them.

KaLara Arch itself does not require an AI API key for its core engine/application.

## Recovery

Before every major step:

```text
git status
```

At every checkpoint:

```text
git status
git diff
```

Keep the previous known-good commit available.

Do not rewrite history without human approval.

## Environment milestone — STEP 00

The environment is ready only when:

1. Git works.
2. CMake works.
3. The selected C++ compiler works.
4. The selected architecture/toolchain preset works.
5. The selected Qt dependency strategy works.
6. Python virtual environment works.
7. A tiny C++ test program builds.
8. A minimal Qt application can compile/run or the accepted initial Qt verification is documented.
9. The repository contains no accidental generated files.
10. The agent can operate entirely within the repository boundary.
11. The selected dependency versions are recorded.
12. The first Git checkpoint has been committed.

## STEP 00 checkpoint requirements

At the end of Step 00, the development agent must provide:

```text
CHECKPOINT: STEP 00

Status:
PASS / PASS WITH NOTES / BLOCKED

Environment:
- OS: Windows
- Compiler: ...
- CMake: ...
- Qt: ...
- Python: ...
- Dependency strategy: ...

Files changed:
- ...

Build:
PASS / FAIL

Tests:
- ...

Acceptance criteria:
- PASS Git
- PASS CMake
- PASS Compiler
- PASS Qt/toolchain
- PASS Python venv
- PASS Tiny test program
- PASS Repository boundary
- PASS No accidental generated files
- PASS First checkpoint commit

Known issues:
- ...

Commit:
<commit hash>

Review required.

Type CONTINUE to start STEP 01.
```

The agent must stop here and wait for the exact `CONTINUE` instruction.
