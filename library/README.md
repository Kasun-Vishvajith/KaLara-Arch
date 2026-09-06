# Library folders — ready for your SVGs

The category folders already exist in this pack. Put plan-view SVGs in the matching symbols/ category. An SVG becomes a selectable item after its catalog definition is added; the runtime must not guess its physical dimensions from artwork alone.

Do not edit a project's embedded assets to update all instances silently. Use the application library import/update flow specified in [the library contract](../docs/10_LIBRARY_AND_SVG.md).

## Quick example

Place symbols/furniture/bed-double.svg. Create catalog/bed-double.json using the schema example in the library contract, with physical width/depth in mm and matching relative SVG path. Run the asset validator implemented in Step 14. Then inspect the thumbnail, place at 1800×2000 mm, rotate, save/reopen, and export.

Artwork should be top-down orthographic 2D linework with a valid viewBox. Remove scripts, external references, shadows/perspective effects, and embedded executable content. Keep paths self-contained. Preserve the source license in licenses/.

## The application must work before you add artwork

The code built from this pack must ship procedural architectural symbols for the complete starter catalog. The folders can stay empty except for these instructions and drafting must still work. This pack delivers specifications and directory scaffolding; the procedural runtime is implemented in Step 14, not already included here.
