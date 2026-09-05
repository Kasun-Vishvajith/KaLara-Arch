# KaLara Arch — AGENTS.md
## Controlled AI Development Protocol

**Project:** KaLara Arch  
**Purpose:** Development-time instructions for AI coding agents  
**Status:** Authoritative development controller  
**Scope:** Building the KaLara Arch architectural planning system only  
**Important:** This document is NOT the runtime AI-agent specification for users of finished KaLara Arch projects.

---

# 0. READ THIS FIRST

You are an AI coding agent working on the **KaLara Arch architectural planning system**.

Your job is to build KaLara Arch safely, incrementally, predictably, and with human review at every major checkpoint.

You MUST:

1. Work on exactly one development step at a time.
2. Read the current step before editing code.
3. Never skip a checkpoint.
4. Never silently expand scope.
5. Never rewrite completed architecture without a documented reason.
6. Keep the project buildable at every checkpoint.
7. Prefer small, reversible changes.
8. Use the repository as the source of truth.
9. Stop at every checkpoint.
10. Ask the human to review the result.
11. Do NOT continue until the human explicitly types:
   `CONTINUE`
12. If the human types anything else, treat it as review/change instructions and do not advance automatically.
13. Never claim that a step passed unless its acceptance criteria have been demonstrated with evidence.
14. When architectural behavior is user-configurable, do not hard-code one behavior where the specification requires a setting or explicit choice.

The agent must never say that a step is complete when the acceptance criteria have not been demonstrated.

---

# 1. PRODUCT IDENTITY

KaLara Arch is an **open-source, human-first, AI-native architectural planning system**.

Core goal:

> Make it possible for beginners to create accurate architectural floor plans while providing professional-grade structured data and external AI agents with a safe, machine-readable way to understand and modify the same architectural project.

KaLara Arch is primarily a **2D planning and documentation environment** in V1.

It must be suitable for workflows that begin in KaLara Arch and continue in:

- AutoCAD and other CAD applications
- SketchUp
- Blender
- future BIM/3D systems

KaLara Arch itself does NOT require an AI model.

AI is an optional external development/design assistant.

The V1 product target is:

- Windows
- 2D architectural planning
- dimensional accuracy
- CAD-oriented drafting
- multi-floor buildings
- site planning
- semantic architectural objects
- parametric/constraint-aware editing
- AI-agent integration prepared as a later phase
- Apache License 2.0

Future:

- IFC/BIM workflows
- stronger building-code rule systems
- direct 3D generation
- Blender/SketchUp bridges
- broader platform support

These are NOT early V1 implementation priorities unless explicitly opened by the current roadmap step.

---

# 2. CORE PRODUCT PRINCIPLES

KaLara Arch must be built around these principles:

## Principle 1 — Structured Architectural Model First

The architectural project model is the source of truth.

The 2D viewport is a view/editor for that model.

Do NOT make the visible drawing the only representation of project state.

Preferred architecture:

```text
Structured Architectural Model
            ↓
        2D Viewport
            ↓
      Human Interaction
```

and:

```text
External AI Agent
        ↓
Architectural Engine API
        ↓
Structured Architectural Model
        ↓
2D Viewport / Export
```

---

## Principle 2 — Geometry + Semantics + Intent

Important architectural elements must be represented by three conceptual layers:

```text
GEOMETRY
What physically exists.

SEMANTICS
What the element means.

INTENT
Why it exists / what design requirements it should satisfy.
```

Example:

```text
Wall W102

Geometry:
    start = (4200, 3500)
    end = (8400, 3500)
    thickness = 150 mm

Semantics:
    type = InteriorWall
    room_boundary = Bedroom / Corridor

Intent:
    privacy = High
    preferred_material = Brick
    must_remain_connected = true
```

The exact internal schema may evolve, but this conceptual separation must remain.

---

## Principle 3 — Millimetres Are the Canonical Internal Unit

The canonical internal unit is:

```text
millimetre (mm)
```

For example:

```text
4200 mm
4800 mm
150 mm
900 mm
```

The user may select display units in settings, including metres and other supported units.

Display conversion MUST NOT change the underlying canonical geometry.

---

## Principle 4 — 1 mm Architectural Precision

V1 architectural dimensions use a nominal 1 mm precision target.

Do not unnecessarily introduce precision that creates instability.

Internal numerical calculations must remain deterministic and geometrically stable.

---

## Principle 5 — Multi-Level From the Beginning

The project model must support multiple levels from the start.

Example:

```text
Project
 └── Site
      └── Building
           ├── Ground Floor
           ├── First Floor
           └── Roof
```

Each level must support real elevation data.

Example:

```text
Ground = 0 mm
First Floor = 3200 mm
Roof = 6400 mm
```

---

## Principle 6 — Human and AI Use the Same Mutation API

The final architecture is:

```text
Human Editor
      \\
       \\
        → Architectural API → Project State
       /
      /
AI Agent
```

Do NOT create a secret AI-only mutation path.

AI operations must ultimately go through the same controlled mutation system used by humans.

---

## Principle 7 — Export Is a Product Feature

Export is not an afterthought.

The structured architectural model must be designed so that clean export to CAD/documentation formats is possible.

V1 mandatory export targets:

```text
DXF
SVG
PDF
JSON
```

Future targets:

```text
IFC
DWG
Dedicated SketchUp bridge
Dedicated Blender bridge
```

Do NOT make the native project format equal to any single export format.

---

## Principle 8 — 2D First, 3D Later

KaLara Arch V1 is a 2D planning/documentation system.

However, the data model MUST preserve enough information to support future 3D generation.

Walls, openings, stairs, levels, fixtures, site elements, and other important objects should have extensible dimensional/elevation information where appropriate.

Do not build a full 3D editor during early V1 steps merely because the data can support one later.

---

# 3. AUTHORITATIVE ARCHITECTURE

Use this architecture unless the human explicitly changes it.

## Core technology

### C++

C++ is the primary engine/core language.

Responsible for:

- geometry
- numerical foundations
- project model
- architectural objects
- constraints
- serialization
- history/transactions
- validation
- rendering infrastructure
- engine/application API
- export infrastructure
- performance-sensitive systems

### Python

Python is the tooling layer.

Responsible for:

- asset/library processing
- import/export helpers
- project tooling
- automation
- validation utilities where appropriate
- test/build helpers
- development utilities
- future AI/development utilities

Python is NOT the authoritative runtime model for the architectural application.

### Qt

Qt is the primary desktop UI framework.

Responsible for:

- desktop application shell
- panels
- docking
- hierarchy
- inspector
- property editing
- dialogs
- settings
- library browser
- documentation-oriented UI

Do not introduce another primary desktop UI framework without an architecture-change request.

---

# 4. CONCEPTUAL LAYER SEPARATION

The repository MUST maintain clean conceptual layers from the beginning.

Recommended structure:

```text
core/
    Math
    Geometry
    Units
    IDs
    Logging
    Serialization primitives
    Utilities

architecture/
    Project
    Site
    Building
    Levels
    Rooms
    Walls
    Openings
    Doors
    Windows
    Stairs
    Furniture
    Fixtures
    Site Elements
    Dimensions
    Semantics
    Intent
    Constraints

runtime/
    Scene/Project Runtime
    Selection
    Commands
    Transactions
    History
    Validation
    API

editor/
    Viewport
    Grid
    Snapping
    Hierarchy
    Inspector
    Library Browser
    Properties
    Annotation Tools
    Drawing Modes
    Editor State

exporters/
    DXF
    SVG
    PDF
    JSON
    Future IFC/DWG adapters

tools/
    Python tools

library/
    2D architectural library data

tests/

docs/
```

Preferred dependency direction:

```text
editor
   ↓
runtime
   ↓
architecture
   ↓
core
```

Exporters may depend on the structured model and core/runtime interfaces as required, but must not become the source of truth for the architectural model.

`core` must not depend on Qt, rendering, or editor systems.

The architectural model must not depend on editor-only state.

The exported project must not require the editor to remain installed merely to interpret the exported result.

---

# 5. NON-NEGOTIABLE ARCHITECTURAL RULES

## Rule 1 — Stable Entity IDs

Every persistent architectural entity gets a stable unique ID.

Do NOT use:

- raw pointer identity
- vector index as identity
- temporary display name as identity
- viewport selection index as identity

Entity names may change.

Entity IDs must remain stable.

Example:

```text
entity_a81f32
```

---

## Rule 2 — Stable Project / Level / Room / Element Identity

Persistent identity must exist at appropriate levels of the model.

At minimum, identity must be stable for important entities such as:

- project
- site
- building
- level
- room/space
- wall
- door
- window
- stair
- library instance
- dimension
- annotation

---

## Rule 3 — One Mutation Choke Point

All meaningful project/model mutations must pass through the controlled mutation/API layer.

Do NOT scatter direct state writes throughout the editor.

Preferred pattern:

```text
UI / External Agent
        ↓
Architectural Command / API
        ↓
Mutation
        ↓
Validation / constraint handling
        ↓
Transaction / history
        ↓
Project State
```

This rule must exist BEFORE AI integration.

---

## Rule 4 — Serialize Early

Important project state must be serializable from the beginning.

The first native representation may use JSON internally where appropriate, but the long-term native project format must remain separate from export formats.

Serialization must support:

```text
state
 ↓
save
 ↓
load
 ↓
semantically equivalent state
```

No major feature should depend on undocumented hidden state where practical.

---

## Rule 5 — Parametric Architectural Objects

Architectural objects are not merely lines.

Examples:

```text
Wall
Door
Window
Room
Stair
Column
Furniture
Fixture
Pool
Site Element
```

must be represented semantically wherever applicable.

A wall should understand that it is a wall, not merely four line segments.

---

## Rule 6 — User-Preference Controlled Parametric Behavior

The human explicitly chose **user preference** for how certain connected geometry behaves.

Therefore, KaLara Arch MUST NOT hard-code one global behavior for changes such as:

- wall thickness
- dimension changes
- connected geometry propagation

The system should support an explicit preference/configuration model where practical.

Conceptually:

```text
Dimension Change Behavior
    Ask Me
    Propagate Connected Geometry
    Preserve Selected Reference
```

and:

```text
Wall Thickness Change
    Ask Me
    Preserve Outer Face
    Preserve Inner Face
    Preserve Centerline
```

The exact options are to be finalized in the relevant roadmap step.

---

## Rule 7 — Associative Dimensions

Dimensions should be associated with their referenced geometry.

Example:

```text
Wall = 4000 mm
Dimension = 4000 mm

Wall changes → 4500 mm
Dimension updates → 4500 mm
```

A dimension must not become a stale text label after its reference geometry changes.

---

## Rule 8 — Constraints Are First-Class Data

Where constraints exist, they must be represented explicitly rather than hidden inside editor behavior.

Constraints may include:

```text
alignment
coincidence
parallelism
perpendicularity
fixed distance
minimum distance
containment
attachment
room boundary closure
clearance
site boundary
setback
host relationship
```

The full constraint system may be developed incrementally.

---

## Rule 9 — Hard Constraints vs Soft Preferences

The architecture should distinguish:

```text
HARD CONSTRAINT
```

from:

```text
SOFT PREFERENCE
```

Example:

```text
Pool must remain within site boundary
= HARD

Master bedroom should face garden
= SOFT
```

This distinction is important for later AI planning and validation.

---

## Rule 10 — Room Representation

Rooms/spaces must support both:

- automatically derived room boundaries where possible
- explicit room objects when a human needs direct control

Room data should be able to expose at minimum where computable:

```text
area
perimeter
width
length
level
boundary references
room type
```

---

## Rule 11 — Semantic Library Objects

The 2D library is a structured architectural library, not a collection of decorative pictures.

Every important library item should support, where applicable:

```text
semantic type
canonical dimensions
clearances
anchor points
orientation
connection/hosting information
metadata
```

Example:

```text
BED_DOUBLE

Width: 1800 mm
Length: 2000 mm
Category: Furniture
Semantic type: SleepingFurniture
```

The library contains **2D planning representations**, not full 3D asset packs, unless a future phase explicitly introduces 3D assets.

---

## Rule 12 — Snapping Is Semantic

Snapping should eventually support meaningful relationships such as:

```text
wall-to-wall
wall-to-door
door-to-wall
window-to-wall
endpoint
center
midpoint
grid
alignment
equal spacing
```

Do not treat snapping as a purely visual pixel effect.

---

## Rule 13 — Coordinate Systems

KaLara Arch should use a stable local project coordinate system internally.

The architecture must leave room for future real-world/site coordinates.

Do not introduce GIS complexity during early steps unless required by the current acceptance criteria.

---

## Rule 14 — Orthogonal-First, Arbitrary-Angle Capable

V1 workflows should be optimized for common orthogonal architectural layouts.

However, the geometry foundation must not prevent future arbitrary angles.

```text
90°-friendly
+
arbitrary-angle-capable
```

Do not build a geometry engine that assumes every object can only exist at 0°/90°/180°/270°.

---

## Rule 15 — Curves Are Future-Capable

Curved geometry should be architecturally possible but does not need to dominate V1.

Straight architectural walls and rectilinear planning are the initial priority.

---

## Rule 16 — Validation Is Distinct From Professional Approval

KaLara Arch may provide:

```text
Design assistance
Geometric validation
Constraint validation
Configured rule validation
```

It must not represent its automated validation as a substitute for required professional architectural, structural, planning, or authority approval.

Country-specific building rules are pluggable/future architecture, not one hidden universal rule set.

---

# 6. PROJECT MODEL HIERARCHY

The conceptual hierarchy is:

```text
Project
 └── Site
      └── Building(s)
           ├── Level(s)
           │    ├── Room(s)
           │    ├── Wall(s)
           │    ├── Door(s)
           │    ├── Window(s)
           │    ├── Stair(s)
           │    ├── Furniture
           │    ├── Fixtures
           │    ├── Dimensions
           │    └── Annotations
           └── Shared / building-level data
```

The exact schema may evolve, but relationships and stable identity must remain explicit.

---

# 7. SITE + BUILDING FOUNDATION

The project should be able to represent, as the roadmap reaches the relevant steps:

```text
property boundary
building footprint
setbacks
road context
entrances
parking
landscape zones
pool / outdoor elements
north direction
site annotations
```

Do not add full GIS functionality merely because site information exists.

---

# 8. DIMENSIONS + DOCUMENTATION

KaLara Arch must support real architectural documentation concepts.

Target concepts include:

```text
dimensions
room names
room areas
door/window tags
level markers
north arrow
grid
notes
symbols
scale
```

Drawing scale should support professional-style ratios such as:

```text
1:50
1:100
1:200
```

The internal geometry is always stored at real size; scale is a documentation/presentation property.

---

# 9. EXPORT ARCHITECTURE

Do NOT make exporters responsible for architectural intelligence.

Preferred pattern:

```text
Native Architectural Model
           ↓
       Export Adapter
           ↓
      Target Format
```

V1:

```text
DXF
SVG
PDF
JSON
```

Future:

```text
IFC
DWG
SketchUp bridge
Blender bridge
```

The internal model must not become constrained by the lowest-common-denominator limitations of any one target format.

---

# 10. NATIVE PROJECT FORMAT

KaLara Arch should have a dedicated native project representation.

Conceptually:

```text
house.kla
```

The exact storage implementation is a roadmap decision, but the native project must be capable of preserving structured data such as:

```text
project metadata
site
buildings
levels
rooms
walls
openings
stairs
furniture
fixtures
dimensions
annotations
semantics
intent
constraints
history metadata
settings
```

Do not use DXF, SVG, or PDF as the native source of truth.

---

# 11. EXTERNAL AI ARCHITECTURE

KaLara Arch itself does NOT contain an AI model.

The later architecture is:

```text
Human
  \
   \
    → KaLara Arch → Project State
   /
  /
External AI Agent
```

The external agent should use a structured API rather than screen scraping.

Possible semantic operations include:

```text
create_project
create_site
create_level
create_room
resize_room
move_wall
set_wall_thickness
add_door
add_window
add_stair
add_library_object
move_entity
rotate_entity
measure
select
inspect
set_semantic_type
set_constraint
validate
export
undo
redo
```

These are conceptual examples. Do not implement future API operations before their roadmap step.

---

# 12. AI CONTEXT

Eventually the external AI agent should be able to retrieve structured context including:

```text
current project
current site
current building
current level
current selection
entity metadata
room information
dimensions
constraints
recent changes
validation errors
library objects
project summary
```

The agent should not be required to infer basic project state from a screenshot when structured state exists.

---

# 13. AI TRANSACTIONS + HISTORY

AI changes must be grouped into logical transactions.

Example:

```text
Transaction #1042

Actor:
AI Agent

Intent:
Create master bedroom

Operations:
1. Create room
2. Create four connected walls
3. Add door
4. Add window
5. Add dimensions
6. Apply relevant constraints

Result:
SUCCESS
```

A logical AI task should be undoable as one transaction where appropriate.

Human operations should also support appropriate grouping.

---

# 14. AI PERMISSIONS

AI must NOT automatically receive unrestricted control merely because the API exists.

Later AI phases must support permissions such as:

```text
read
select
inspect
write
export
history
```

and approval requirements for potentially dangerous operations such as:

```text
delete entire building
delete floor
mass geometry rewrite
major structural-layout modification
large-scale destructive operation
```

The permission model must be explicit and auditable.

---

# 15. HUMAN + AI LIVE CONTEXT

Eventually, the system should support a live connection in which:

```text
Human changes project
        ↓
Committed project change
        ↓
AI receives/retrieves updated context
```

and:

```text
AI changes project
        ↓
Committed project change
        ↓
Human UI updates
```

Continuous drag events should be coalesced where appropriate.

Committed state is authoritative.

Do not implement real-time synchronization during an earlier step merely because the architecture anticipates it.

---

# 16. DEVELOPMENT SAFETY ENVIRONMENT

The AI agent MUST work inside an isolated project workspace.

Recommended host layout:

```text
D:\Dev\KaLaraArch\
```

or another user-owned development directory.

Do NOT develop inside:

```text
C:\
C:\Windows\
C:\Program Files\
C:\Program Files (x86)\
Windows system directories
```

except where a normal installer/toolchain requires system installation.

---

# 17. REPOSITORY BOUNDARY

The Git repository is the primary writable boundary.

The agent MUST NOT modify files outside the repository unless the current step explicitly requires it.

The agent MUST NOT:

- delete unrelated files
- modify registry settings
- disable antivirus/firewall
- change OS security settings
- modify boot settings
- modify unrelated environment variables
- install random software
- execute downloaded unknown binaries
- run destructive shell commands

---

# 18. PRIVILEGE RULE

Never request or use administrator privileges unless the human explicitly approves it for a specific installation.

Prefer:

- user-level installs
- project-local dependencies
- repository-local tools
- virtual environments

---

# 19. PYTHON ENVIRONMENT

Use a project-local virtual environment:

```text
tools/python/.venv/
```

Never install project Python dependencies globally.

Preferred:

```text
python -m venv tools/python/.venv
```

then activate/use that environment.

---

# 20. DEPENDENCY RULE

Prefer pinned, reproducible dependencies.

Do not casually update all dependencies.

A dependency change must be treated as an architectural/development decision and recorded in checkpoint notes and/or `docs/dependencies.md`.

---

# 21. C++ DEPENDENCIES

Use a repository-controlled package/dependency strategy.

The selected package manager and exact dependency versions must be recorded in:

```text
docs/dependencies.md
```

Do not download arbitrary third-party source code during an unrelated task.

---

# 22. BUILD DIRECTORY

Build outputs must be isolated:

```text
build/
out/
dist/
```

Generated files must not pollute source directories.

---

# 23. SECRETS

Never store:

- API keys
- passwords
- access tokens
- private credentials
- personal secrets

inside the repository.

Use:

```text
.env
```

only when necessary and ensure it is ignored by Git.

KaLara Arch itself has no AI API key requirement.

---

# 24. GIT SAFETY

Git must be initialized before substantial implementation begins.

The agent should create small logical commits.

Recommended format:

```text
step-00: establish safe development environment
step-01: establish project foundation
step-02: establish geometry and units
...
```

Never rewrite or squash history without human approval.

Before starting a step:

```text
git status
```

At checkpoint:

```text
git status
git diff
```

The agent must report:

- changed files
- added files
- deleted files
- build result
- tests
- known issues

---

# 25. WORKFLOW FOR EVERY STEP

Every development step follows exactly this process.

```text
READ
 ↓
UNDERSTAND
 ↓
INSPECT CURRENT STATE
 ↓
PLAN
 ↓
IMPLEMENT
 ↓
BUILD
 ↓
TEST
 ↓
CHECK ACCEPTANCE CRITERIA
 ↓
DOCUMENT
 ↓
COMMIT
 ↓
CHECKPOINT
 ↓
STOP
```

Do not automatically start the next step.

---

# 26. BEFORE MODIFYING ANY CODE

The agent MUST first inspect:

1. `AGENTS.md`
2. current roadmap/checkpoint
3. repository status
4. current project structure
5. relevant source files
6. relevant tests
7. current build result if needed
8. relevant documentation/architecture decisions

The agent must understand the existing implementation before editing it.

Do not replace existing files blindly.

---

# 27. STEP SCOPE CONTROL

Each step has:

- objective
- inputs
- allowed changes
- implementation tasks
- tests
- acceptance criteria
- checkpoint

The agent MUST NOT perform future-step work “because it is convenient.”

Example:

If the current step is:

```text
Establish geometry and units
```

do not additionally implement:

- complete room editor
- DXF exporter
- AI bridge
- multi-agent synchronization

unless needed to satisfy the current acceptance criteria.

---

# 28. CHECKPOINT PROTOCOL

At the end of every step, the agent MUST stop.

The checkpoint response must use this structure:

```text
CHECKPOINT: STEP XX

Status:
PASS / PASS WITH NOTES / BLOCKED

Implemented:
- ...

Files changed:
- ...

Build:
PASS / FAIL

Tests:
- ...

Acceptance criteria:
- PASS ...
- PASS ...

Known issues:
- ...

Commit:
<commit hash>

Review required.

Type CONTINUE to start the next step.

Or describe any changes you want before continuing.
```

The agent MUST NOT continue after displaying this message.

---

# 29. CONTINUE RULE

Only the instruction:

```text
CONTINUE
```

moves the project to the next step.

Case-insensitive matching is acceptable.

Examples that do NOT automatically authorize continuation:

```text
looks good
nice
okay
yes
fine
go ahead
```

Unless the human explicitly uses:

```text
CONTINUE
```

the agent must remain at the current checkpoint.

If the human provides change instructions, implement those changes first and create a new checkpoint.

---

# 30. REVIEW CHANGE PROTOCOL

If the human says:

> Change how wall thickness is handled.

The agent must:

1. remain on the current checkpoint
2. interpret the requested change
3. modify the project
4. rebuild/test
5. produce another checkpoint
6. wait for `CONTINUE`

Do not silently advance.

---

# 31. BLOCKED PROTOCOL

If a step cannot be completed safely:

```text
CHECKPOINT: STEP XX

Status:
BLOCKED

Reason:
...

What was attempted:
...

Evidence:
...

Recommended choices:
A. ...
B. ...

No next step has been started.
```

Do not hide failures.

Do not fake a PASS.

---

# 32. ERROR POLICY

If a build fails:

1. diagnose the failure
2. fix only what belongs to the current step
3. rebuild
4. test again

If fixing it would require future architecture work:

STOP and report it.

Do not create a large speculative rewrite.

---

# 33. CODE QUALITY RULES

Prefer:

- readable code
- small modules
- explicit ownership
- clear interfaces
- deterministic geometry behavior
- deterministic serialization
- testable logic
- descriptive names
- comments only where they add real value

Avoid:

- premature abstraction
- unnecessary template metaprogramming
- giant classes
- global mutable state
- hidden side effects
- magic constants
- undocumented geometry assumptions
- editor-only state leaking into the model
- untracked generated files

---

# 34. TESTING RULE

Every completed step needs evidence.

Depending on the step, evidence may be:

- unit test
- geometry test
- serialization round-trip test
- integration test
- executable run
- visual/manual test
- export/open verification
- API test
- performance measurement

For visual/editor features, a manual test is acceptable when automated testing is not practical.

Record what was tested.

For geometry-related changes, include explicit measurements or invariants wherever practical.

---

# 35. GEOMETRY TESTING RULES

Geometry tests should prefer measurable assertions over screenshots alone.

Examples:

```text
wall length == expected value
room area == expected tolerance
boundary is closed
wall endpoints coincide within tolerance
attached door remains on host wall
```

A visually plausible result is not sufficient evidence when a numerical assertion can be tested.

---

# 36. EXPORT TESTING RULES

An exporter is not considered working merely because a file is produced.

Where practical, test that:

```text
native model
 ↓
export
 ↓
parse/import or inspect result
 ↓
expected geometry / units / layers / metadata preserved
```

For each supported format, record the exact test method used.

---

# 37. PERFORMANCE RULE

Do not optimize blindly.

First make the architecture correct and measurable.

Use:

- frame-time measurements
- geometry operation timing
- memory measurements
- targeted benchmarks

Do not sacrifice architecture for unmeasured theoretical performance.

---

# 38. ARCHITECTURE CHANGE RULE

If the agent discovers that the current architecture must change:

DO NOT silently change it.

Use:

```text
ARCHITECTURE CHANGE REQUEST

Current decision:
...

Problem:
...

Evidence:
...

Proposed change:
...

Affected systems:
...

Migration risk:
...

Human approval required.
```

Wait for the human's decision.

---

# 39. SOURCE-OF-TRUTH DOCUMENTS

Use this priority order:

1. `AGENTS.md`
2. human-approved architecture decisions
3. current implementation
4. current roadmap/checkpoint
5. older draft/spec documents

If documents conflict, do not silently choose.

The current approved architecture takes priority.

---

# 40. DEVELOPMENT PHASES

The development process is split into controlled phases.

## STEP 00 — SAFE ENVIRONMENT

Prove:

- Git
- CMake
- selected C++ compiler/toolchain
- dependency management
- Qt availability
- Python virtual environment
- local build
- repository boundary
- tiny test program

No architecture feature work yet.

---

## STEP 01 — PROJECT FOUNDATION

Create:

```text
core
architecture
runtime
editor
exporters
tests
tools
library
docs
```

Establish:

- CMake
- build presets
- Qt application target
- logging
- configuration
- basic application target
- empty but compileable layer boundaries

---

## STEP 02 — GEOMETRY + UNITS

Implement:

- points
- vectors
- lines/segments
- rectangles
- transforms
- angles
- tolerances
- canonical millimetre units
- unit conversion
- basic geometric operations

Acceptance must include measurable numerical tests.

---

## STEP 03 — ARCHITECTURAL DATA MODEL

Implement the first structured model for:

- project
- site
- building
- level
- entity IDs
- basic semantic typing
- extensible metadata

Do NOT yet build every architectural object.

---

## STEP 04 — 2D CANVAS + GRID

Implement:

- GPU-accelerated 2D viewport foundation
- pan
- zoom
- grid
- coordinates
- origin
- orthogonal-friendly interactions
- basic selection foundation

---

## STEP 05 — WALL SYSTEM

Implement:

- wall entity
- wall centerline/boundary representation as appropriate
- thickness
- length
- joins
- connected endpoints
- wall selection
- wall editing
- initial preference-controlled thickness behavior

---

## STEP 06 — ROOMS + SPACES

Implement:

- room entities
- room boundaries
- room type
- room dimensions
- area
- perimeter
- automatic boundary detection where practical
- explicit room definitions
- room labeling

---

## STEP 07 — DOORS + WINDOWS

Implement:

- semantic door objects
- semantic window objects
- wall-host relationships
- opening geometry
- dimensions
- placement
- editing

---

## STEP 08 — DIMENSIONS + ANNOTATIONS

Implement:

- associative dimensions
- room labels
- room areas
- door/window tags
- notes
- levels
- north arrow
- basic grid annotations
- scale-aware documentation

---

## STEP 09 — CONSTRAINTS + SMART EDITING

Implement:

- geometric constraints
- hosting relationships
- clearances
- hard vs soft constraints
- preference-controlled propagation
- conflict reporting

The exact constraint system may grow incrementally.

---

## STEP 10 — SELECTION + MANIPULATION

Implement:

- robust selection
- multi-selection
- move
- rotate
- resize
- alignment
- snapping
- semantic selection context
- editor-to-model mutation pathway

---

## STEP 11 — ARCHITECTURAL 2D LIBRARY

Implement:

- library browser
- categories
- reusable 2D objects
- furniture
- bathroom fixtures
- kitchen objects
- doors/windows
- stairs
- site/outdoor objects
- dimensions
- semantic metadata
- clearance metadata
- anchor points
- user-created library objects

The library is 2D-first and structured, not a generic 3D asset marketplace.

---

## STEP 12 — MULTI-LEVEL BUILDINGS

Implement:

- multiple levels
- real level elevations
- level visibility
- floor switching
- cross-level references
- roof level foundation
- vertical relationships where needed

---

## STEP 13 — SITE PLANNING

Implement:

- property boundary
- setbacks
- road context
- entrances
- parking
- landscape zones
- pool/outdoor objects
- north direction
- site annotations

---

## STEP 14 — VALIDATION ENGINE

Implement:

- geometry validation
- connection validation
- room validation
- clearance validation
- site/setback validation foundation
- constraint conflicts
- severity levels
- human-readable diagnostics

Do not represent validation as professional approval.

---

## STEP 15 — SERIALIZATION + NATIVE PROJECT

Implement:

- structured project serialization
- save/load
- native `.kla` project representation or the approved equivalent
- schema/version metadata
- migration foundation
- round-trip tests

Acceptance:

```text
state
 ↓
save
 ↓
load
 ↓
semantically equivalent state
```

---

## STEP 16 — HISTORY + TRANSACTIONS + UNDO

Implement:

- logical transactions
- grouped human operations
- AI-style future transaction compatibility
- undo
- redo
- before/after state tracking
- stable history IDs

---

## STEP 17 — DXF EXPORT

Implement a professional-oriented DXF export foundation.

Goals include:

- real-scale geometry
- units
- layers
- architectural categories
- linework
- dimensions where supported
- text/annotations where supported
- clean organization

Do not begin DWG implementation merely because DXF is complete.

---

## STEP 18 — SVG + PDF + JSON EXPORT

Implement:

- SVG floor plan export
- PDF documentation output
- structured JSON interchange export
- export settings
- scale-aware documentation

---

## STEP 19 — CAD / 3D WORKFLOW VALIDATION

Validate actual workflows involving:

```text
KaLara Arch → DXF → CAD
KaLara Arch → DXF → SketchUp
KaLara Arch → SVG/DXF → Blender
```

Record measured scale/geometry verification.

A file merely opening is not enough; geometry and units must be verified.

---

## STEP 20 — IFC FOUNDATION

Begin structured IFC/BIM interoperability.

Do not compromise the native model to fit IFC.

Implement only the subset justified by current requirements.

---

## STEP 21 — ARCHITECTURAL ENGINE API

Finalize the public semantic API.

Initial categories should include concepts such as:

```text
project
site
building
level
room
entity
component/property
library
selection
dimension
constraint
validation
history
context
export
```

The API must be versioned and documented.

---

## STEP 22 — AI BRIDGE

ONLY NOW begin the external AI development interface.

Implement:

- agent protocol
- MCP compatibility where appropriate
- local bridge
- authentication/authorization model
- tool discovery
- structured responses
- safe mutation path

No embedded model.

---

## STEP 23 — LIVE CONTEXT

Implement:

- current project
- current level
- current selection
- inspector context
- recent changes
- dimensions
- constraints
- validation errors
- relevant library objects
- project summary

---

## STEP 24 — REAL-TIME EVENT CHANNEL

Implement lightweight local real-time synchronization.

Rules:

- meaningful changes are emitted
- continuous drag events are coalesced
- committed state is authoritative
- agents can retrieve changes since their last known version

---

## STEP 25 — AI PERMISSIONS

Implement:

- per-agent permissions
- per-session/task permissions
- read permissions
- write permissions
- dangerous-operation approval
- audit records

---

## STEP 26 — MULTI-AGENT FOUNDATION

Allow multiple external agents to connect safely.

Do not prioritize advanced collaboration until single-agent reliability is excellent.

---

## STEP 27 — ARCHITECTURAL AI ACCEPTANCE TESTS

Required demonstrations:

1. AI reads a project.
2. AI reads the current level.
3. AI reads the current selection.
4. AI understands semantic architectural entities.
5. AI reads dimensions.
6. AI understands constraints.
7. AI creates a room.
8. AI modifies a wall.
9. AI modifies room dimensions.
10. AI places a door/window.
11. AI uses a library object.
12. Human modification appears to AI.
13. AI modification appears to human.
14. AI transaction can be undone.
15. AI can produce a small structured residential floor plan.
16. Exported geometry preserves expected scale/measurements.
17. Multiple agents can eventually connect without corrupting state.

---

# 41. V1 “DO NOT BUILD” LIST

Until the human explicitly opens a later phase, do not prioritize:

- full 3D editor
- full 3D asset library
- structural engineering analysis
- full building-code coverage for every country
- cloud platform
- embedded AI model
- cloud LLM dependency
- console/mobile deployment
- advanced GIS platform
- direct support for dozens of proprietary formats
- automatic generative 3D asset creation
- unnecessary distributed architecture
- arbitrary shell execution through AI

---

# 42. V1 ACCEPTANCE WORKFLOWS

The project is not considered successful because it has many features.

It succeeds when these workflows work.

# 41A. PROJECT INITIALIZATION + TEMPLATE SYSTEM

KaLara Arch should support two first-class project creation paths:

```text
Create Project
    ├── Empty Canvas
    └── Start from Template
```

The template path is not merely a visual preset. A template is a structured architectural starting configuration that reduces repetitive human setup and reduces unnecessary agentic AI discovery, prompting, and API calls.

## Project Initialization Wizard

When starting from a template, KaLara Arch should provide a guided initialization questionnaire. The questionnaire must capture known project context without forcing the user to invent information they do not yet know. Every applicable field should support an explicit `Not decided yet` / `Undecided` state.

The initial questionnaire should be designed around categories such as:

```text
Project / Building Type
Site / Land
    - site shape
    - site dimensions where known
Terrain
    - flat / sloped / irregular where known
Levels
    - planned floor count
Measurement / Display Units
    - metric / imperial / undecided
Building
    - target building area where known
Programme / Requirements
    - bedrooms
    - bathrooms
    - parking
    - other template-specific requirements
```

The exact questionnaire may evolve with the template library. Do not make every field mandatory. The system must distinguish between a known value, an explicitly undecided value, and an omitted value where appropriate. `Undecided` must never be represented as a misleading numeric value such as zero.

## Template Semantics

A template should establish reusable structured architectural intent and, where appropriate, an initial arrangement of semantic architectural objects. It must not create a separate or opaque representation that bypasses the normal architectural model.

Conceptually:

```text
Template + Initialization Answers
             ↓
    Initial Project Intent
             ↓
  Initial Structured State
             ↓
 Normal Architectural API
```

Templates may provide sensible defaults for rooms, circulation, object types, dimensions, relationships, and other starting structure, but the resulting entities must become ordinary KaLara architectural entities with stable IDs and normal editability.

## Project Intent as Structured State

Initialization answers should be represented as structured project intent rather than only as natural-language notes. The native project format must be capable of preserving this information alongside geometry, semantics, constraints, settings, and history metadata.

A conceptual representation is:

```json
{
  "intent": {
    "building_type": "residential",
    "site_shape": "trapezoid",
    "terrain": "moderately_sloped",
    "floor_count": 2,
    "target_building_area_mm2": null,
    "status": {
      "target_building_area": "undecided"
    }
  }
}
```

This is conceptual only. Do not implement a final schema before the relevant roadmap step.

## AI Context Efficiency

The initialization system should reduce unnecessary interaction with external AI agents by making basic project context available through structured state from the beginning. The agent should be able to retrieve a compact project summary rather than repeatedly asking the human for information already provided during initialization.

For example, a template-created project might expose:

```text
Building type: Residential
Site shape: Trapezoid
Terrain: Moderately sloped
Floors: 2
Target building area: Undecided
Measurement system: Metric
Bedrooms: 4
Bathrooms: 3
Parking: 2 cars
Template: Residential 4-Bedroom
```

This is an architectural state/context optimization, not an AI-specific secret path. Both human editing and later AI operations continue to use the same controlled architectural mutation system.

## Template API Direction

When the roadmap reaches project initialization and external AI API implementation, the architecture may expose operations conceptually similar to:

```text
create_project
create_project_from_template
instantiate_template
update_project_intent
get_project_context
```

These are architectural direction examples only. Do not implement them prematurely. Template creation and modification must ultimately converge on the same validated project state and mutation/transaction model used by normal human operations.

## Initialization Design Rules

1. Empty Canvas must remain a complete and useful project-creation path.
2. Templates are structured starting points, not image-only presets.
3. Initialization questions should minimize unnecessary human effort.
4. Known information should be stored as structured data.
5. Unknown information must remain explicitly unknown/undecided rather than being guessed.
6. Template defaults must remain editable by the human and external AI through the normal architectural API.
7. Template initialization must not bypass validation, stable IDs, transactions, history, or the architectural model.
8. The initialization system must not become an excuse to build a full generative-design engine in early V1.
9. Template selection and initialization should reduce agentic token/API overhead by providing compact structured context.
10. Do not infer architectural decisions from missing answers without explicit human intent or an approved later planning operation.

---

## Workflow A — Beginner Floor Planning

```text
Open KaLara Arch
 ↓
Create project
 ↓
Choose Empty Canvas or Template
 ↓
[Template] Complete initialization questionnaire
 ↓
Initial structured project state
 ↓
Define/refine site
 ↓
Set units/preferences
 ↓
Create/refine levels
 ↓
Draw walls
 ↓
Create rooms
 ↓
Place doors/windows
 ↓
Drag 2D library objects
 ↓
Configure dimensions
 ↓
Validate
 ↓
Save
 ↓
Export
```

No programming required.

---

## Workflow B — CAD/3D Preparation

```text
Create accurate plan
 ↓
Validate dimensions
 ↓
Export DXF/SVG
 ↓
Open in CAD / SketchUp / Blender
 ↓
Preserve expected scale
 ↓
Continue modeling externally
```

---

## Workflow C — AI-Assisted Planning

```text
Open project
 ↓
Connect external AI
 ↓
Describe requirements
 ↓
AI reads structured project state
 ↓
AI creates/modifies architectural objects
 ↓
Validation runs
 ↓
Human reviews
 ↓
Transaction can be undone
 ↓
Export
```

---

## Workflow D — Selection-Aware AI

```text
Select 3 wall segments
 ↓
Tell AI:
“These are external walls.”
 ↓
AI reads selection
 ↓
AI assigns semantic interpretation
 ↓
AI applies approved operation
 ↓
One transaction
 ↓
Undo works
```

---

# 43. FINAL RULE

The agent's purpose is not to finish KaLara Arch as fast as possible.

The agent's purpose is to build KaLara Arch:

- safely
- correctly
- transparently
- incrementally
- reversibly
- with dimensional integrity
- with machine-readable architectural structure
- with human review at every major checkpoint

When uncertain:

STOP.

Explain the uncertainty.

Request review.

Do not guess on architectural decisions.

---

# 44. FIRST COMMAND

When the repository is ready, the first task is:

```text
STEP 00 — SAFE ENVIRONMENT
```

Do not start architectural implementation before Step 00 passes.

At the end of Step 00:

STOP.

Ask for:

```text
CONTINUE
```

Only then begin Step 01.
