# KaLara Arch — Codex Implementation Controller

## Authority and mission

Implement the complete local 2D application specified here. Current user instructions and platform rules take precedence. Within the adopted pack: this controller → explicitly approved decisions → domain contracts → current step → historical attachments. Existing code is evidence of behavior, not permission to preserve a defect.

For an existing repository, adopt this controller using START_HERE and record the diff. Never silently replace applicable instructions or user work.

## Fixed decisions

- Windows x64, C++20, Qt 6 Widgets, GPU-capable 2D viewport with painter fallback. Python is development tooling only.
- Canonical millimetres; stable entity IDs; explicit topology; semantic model; atomic commands; early undo and serialization.
- No 3D, IFC, runtime AI, login, cloud backend, reserved AI controls, or Python runtime requirement.
- No release-required behavior may remain a stub. Placeholder SVG artwork is allowed only with a functional procedural fallback.
- Keep validated existing subsystems. Do not duplicate the model, transaction manager, UI framework, or persistence stack.

## Every working session

1. Read docs/PROGRESS.md, active step, relevant domain contracts, repository instructions, and Git status.
2. Inspect existing source/tests before edits. Label claimed features VERIFIED, PARTIAL, MISSING, or BROKEN with evidence.
3. State the next concrete result; implement the active step as a usable vertical slice.
4. Build and test relevant behavior. Use numerical assertions for geometry, real UI evidence for interactions.
5. Fix mandatory failures before advancing. Never downgrade a requirement to fit unfinished code.
6. Record commands, results, fixture version, screenshots where useful, issues, and commit in docs/evidence/STEP_XX.md.
7. Update progress and make a focused commit when authorized; never stage unrelated changes.
8. Continue through authorized work. This adopted pack defaults to autonomous progression; it does not require CONTINUE after every step. Honor user-selected review mode.
9. Pause only for missing access, an irreconcilable requirement, or a material decision outside this specification. Explain the evidence and smallest necessary choice.

The old CONTINUE controller is superseded only through explicit adoption. The startup prompt authorizes adoption and routine repository work. Never claim platform approvals have been granted merely because a document says so.

## Code rules

Geometry is Qt-free. Convert Qt types at adapters. No MainWindow pointer in domain/tool services. Editor state belongs to a document session, not a project-wide mutable singleton. Model objects are not viewport items.

Mouse-move previews operate on a draft from a captured base revision. Release commits one validated delta; Esc, focus loss, document switch, or invalid release discards the draft. Never mutate live model objects and begin history afterwards. Rendering never writes the model. Persistent settings changes also use commands.

Use typed millimetres/radians, doubles for computation, explicit rounding and tolerance policies. Do not equate 1 mm architectural display precision with a floating-point epsilon. Never infer permanent connectivity from nearly equal coordinates.

## Evidence and completion

PASS means demonstrated. NOT RUN and BLOCKED are distinct. A compiling toolbar is not a functioning tool. A generated export is not a verified export. A screenshot is not an area test. A non-Windows build is not a Windows release test.

Do not call performance smooth without measurements. Do not call symbols certified from appearance. No fake success toast, inert menu, or hidden failure. Build logs and evidence must survive session changes.

Steps 00–26 are the full plan. Do not append runtime AI or 3D milestones. Close each run with outcome, step status, material changes, tested behavior, blockers, and next work.
