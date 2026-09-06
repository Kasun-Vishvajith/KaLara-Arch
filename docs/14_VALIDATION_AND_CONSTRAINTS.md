# Validation and supported constraint editing

## Separate responsibilities

Geometric invariants protect data integrity. Explicit user constraints encode design choices. Configured advisory rules surface planning issues. None of these is automatic professional approval.

| Class | Examples | Behavior |
|---|---|---|
| Structural data invariant | Invalid ID, NaN, missing host, zero-length wall | Always block commit/load |
| Hard geometric constraint | Fixed junction, required coincidence, fixed length | Block incompatible edit |
| Configured planning rule | Clearance, setback, door width target | Warning by default; explicit user hard setting allowed |
| Preference | Prefer alignment or target room area | Suggest/diagnose; never silently force |

## Minimum supported constraints

Fixed point, coincident junction, horizontal, vertical, parallel, perpendicular, fixed segment length, and wall-host attachment. Persistent IDs and referenced features are explicit. Arbitrary generalized parametric solving is not promised.

Implement connected-component solving of supported equations using a pinned established solver or a bounded local solver with analytic residuals/Jacobian and deterministic variable ordering. Step 12 records the selected method in an ADR. Solver outcome must expose solved, underconstrained, overconstrained, or failed. A timeout is failure with no mutation, not a best-effort commit.

Default interactive solve budget ≤50 ms for an affected component; longer solves run cancellably on a snapshot. Final constraints must satisfy configured numerical residual tolerance before commit. Underconstrained layouts retain nearest-to-baseline configuration rather than jumping to an arbitrary solution.

If a combination is unsupported, reject with “This constraint combination is not supported” and identify the combination. Do not quietly disable constraints. Supported fixture combinations remain required regardless of implementation choice.

## Diagnostic record

Code, severity, entity IDs, title, explanation, numerical evidence, rule source, suggested fix, base revision. Clicking a diagnostic highlights affected geometry. Auto-fix is always a previewable command and is not available unless its behavior is implemented and tested.

Rules panel separates Geometry, Constraints, Rooms, Clearances, Site, and Documentation. Filters never delete the underlying issues. Diagnostics on older revisions are marked stale or replaced, not shown as current.

## Required geometric checks

Dangling references, duplicate IDs, invalid bounds, self-intersection, invalid opening interval, overlapping openings, inconsistent joins, room closure, invalid ring winding, orphan dimensions, unknown library asset, illegal group cycle, hidden/locked change conflict, unsupported transform loss.

Clearance checks use actual transformed envelopes and broad-phase spatial indexing, then exact overlap predicates. Door swing may overlap a clearance envelope; default warning, not universal prohibition. Setback checks use the actual configured polygon regions and distances.

## Rule provenance

Each configured rule stores name, value/unit, scope, source note, version/date if supplied, and severity. Built-ins say Generic planning default. Unknown local rules stay absent/undecided; do not populate legal limits from guesses.

## Fixtures

A constrained rectangle resizes with perpendicularity preserved; a fixed node plus incompatible length change rejects; two conflicting fixed lengths report relevant constraints; a free rectangle is underconstrained but stable; opening overrun blocks; concave site inset returns valid regions; orphan dimension warns visibly; Undo restores constraints and diagnostics for the restored revision.
