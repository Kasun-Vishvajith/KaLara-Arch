# Source reconciliation

## Reviewed inputs

| Input | Retained value | Treatment |
|---|---|---|
| arcada_analysis.md | Wall chains, previews, direct manipulation, thumbnail library | UX inspiration; current implementation claims unverified |
| implementation_plan_Arch.md | Candidate code locations and missing-tool analysis | Audit leads, not proof APIs exist |
| KALARA_ARCADA_GUI_ADDITION_PLAN.md | Tool rail, contextual input, inspector, floor workflow | Re-specified using semantic model contracts |
| README(1).md | Controlled development and templates | Replaced by pack index and execution prompts |
| AGENTS(2).md | C++/Qt layers, IDs, units, constraints, evidence | Keep foundations; correct scope and sequence |
| KA_LARA_ARCH_SAFE_ENV(1).md | Local build isolation and Windows checks | Retain and clarify |

## Binding resolutions

| Earlier conflict | Resolution |
|---|---|
| 2D-first, 3D-later, IFC roadmap | Strictly 2D product; remove those milestones |
| AI dominates later roadmap | End at fully usable local 2D release |
| Same feature called missing and done | Verify actual repository behavior |
| History added after authoring | Establish transactions before mutable tools |
| Drag mutates model before transaction | Draft preview; atomic commit on release |
| Shared node identity by coordinates | Stable junction IDs, explicit join operation |
| Universal 300 mm wall minimum | Reject degeneracy; permit intentional small details |
| Window handle rules contradict editable width | Host-local width grips; no plan drag for sill/height |
| Esc ambiguously erases whole wall chain | Discard pending preview only; committed walls remain |
| Core/runtime/tests forbidden despite new operations | Permit needed domain/test changes; preserve layer direction |
| Global mutable editor singleton | Per-document session state |
| Single TEMP autosave | Per-project/session atomic recovery generations |
| Floor navigation creates floors | Explicit Add Floor; navigation only navigates |
| One switch hides every label | Separate aids, annotations, and print layers |
| Saving appears late | Early model serialization; final portable archive |
| Export uses framebuffer screenshot | Model-derived exports; separate optional capture |
| Mandatory CONTINUE token | Autonomous default after explicit adoption; optional review mode |

Create docs/evidence/BASELINE_AUDIT.md for an existing repo: requirement, source files, actual behavior, automated/manual evidence, VERIFIED/PARTIAL/MISSING/BROKEN, and action. Preserve the baseline commit.

For coordinate-endpoint models, migration produces junction mappings and a candidate-merge report. Do not merge nearby endpoints silently. Raw imported linework remains raw until promoted explicitly.

The Arcada repository itself was not independently audited here. Its supplied descriptions are inspiration, not a claim of current parity or direct code reuse rights.
