# Execution roadmap — complete local 2D release

Execute steps 00–26 in order. Later numbers do not mean optional. Each stage includes relevant UI, command, persistence and tests for its scope; a model-only feature is not complete when its step requires interaction.

For existing repositories, demonstrated work may satisfy parts of a step. Record evidence and run affected regressions instead of rebuilding it. Stubs or historical checkmarks do not satisfy steps.

Default mode is autonomous after explicit pack adoption. Optional review mode pauses after a passing stage. A long stage can span sessions or focused commits; progress must preserve its unfinished criteria.

| Step | Outcome | Main gate |
|---|---|---|
| [00](steps/STEP_00.md) | Environment and baseline audit | Prove the toolchain and classify the existing repository before feature edits. |
| [01](steps/STEP_01.md) | Layered application foundation | Establish a buildable, well-organized desktop shell and dependency boundaries. |
| [02](steps/STEP_02.md) | Units and robust 2D geometry | Deliver the numerical foundation that every measurement and transform uses. |
| [03](steps/STEP_03.md) | Model identity and early serialization | Create authoritative typed project state with stable identity and a tested codec. |
| [04](steps/STEP_04.md) | Transactions and undo before authoring | Make persistent editing atomic and cancelable before rich tools depend on it. |
| [05](steps/STEP_05.md) | Smooth 2D viewport and instrumentation | Deliver readable model rendering, navigation, and performance measurement. |
| [06](steps/STEP_06.md) | Selection, tool lifecycle, and snapping | Make input predictable and selection reliable across the editor. |
| [07](steps/STEP_07.md) | Architectural wall authoring | Draw and edit semantic connected walls as a complete vertical slice. |
| [08](steps/STEP_08.md) | Topology editing and hosted openings | Complete wall split/merge and editable doors/windows with actual host semantics. |
| [09](steps/STEP_09.md) | General 2D drafting and editing | Provide professional basic drafting beyond semantic walls. |
| [10](steps/STEP_10.md) | Transforms, clipboard, groups, and layers | Support daily editing and plan organization consistently. |
| [11](steps/STEP_11.md) | Distance, angle, dimensions, and annotations | Make measurement and documentation numerically trustworthy. |
| [12](steps/STEP_12.md) | Constraints and connected edit policies | Enforce explicit constraints and explain conflicts without hidden changes. |
| [13](steps/STEP_13.md) | Scoped resize and scaling dialog | Deliver the signature wall-referenced X/Y/uniform editing workflow completely. |
| [14](steps/STEP_14.md) | Complete semantic library and SVG pipeline | Ship useful built-in objects without any external artwork dependency. |
| [15](steps/STEP_15.md) | Rooms, columns, stairs, and fixtures | Complete semantic space planning and remaining architectural plan objects. |
| [16](steps/STEP_16.md) | Floors, site, and calibrated underlays | Support coherent multi-floor plans and site context strictly in 2D. |
| [17](steps/STEP_17.md) | Templates, settings, and first-run workflow | Make starting and returning to a project effortless. |
| [18](steps/STEP_18.md) | Portable native files and recovery | Protect real user work and make custom assets portable. |
| [19](steps/STEP_19.md) | Sheets, styles, and schedules | Turn plans into composed professional drawing documents. |
| [20](steps/STEP_20.md) | Vector and image export plus printing | Produce high-quality documents from committed state. |
| [21](steps/STEP_21.md) | DXF import/export and independent CAD proof | Deliver the declared CAD exchange subset with measured interoperability. |
| [22](steps/STEP_22.md) | Integrated validation and full workflow QA | Find cross-feature failures and make diagnostics actionable. |
| [23](steps/STEP_23.md) | Visual polish, accessibility, and onboarding | Reach the specified finished desktop quality. |
| [24](steps/STEP_24.md) | Performance and stability hardening | Prove and achieve responsiveness and bounded resource use. |
| [25](steps/STEP_25.md) | Windows distribution and offline validation | Package a usable application independent of the development machine. |
| [26](steps/STEP_26.md) | LOCAL_2D_RELEASE_GATE | Close the complete local 2D product with traceable evidence. |

## Milestones

- Steps 00–06: safe foundations and responsive input.
- Steps 07–13: precise authoring, measurement, and complete scoped scaling.
- Steps 14–18: semantic planning, libraries, projects, and protection of user work.
- Steps 19–22: documentation, exchange, and integrated correctness.
- Steps 23–26: visual finish, measured performance, distribution, and release.

Serialization and history appear early deliberately. The polished local application is the finish line; there is no appended AI/3D phase.
