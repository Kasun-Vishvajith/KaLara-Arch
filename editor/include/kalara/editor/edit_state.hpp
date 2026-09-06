#pragma once

namespace kalara::editor {

/// Primary user-facing tool modes for the 2D viewport (Arcada §3.1 / KALARA_ARCADA_GUI_ADDITION_PLAN §3.1).
/// Persistent modes are exposed on the tool rail and remain active between clicks.
/// Transient sub-states are managed internally by ViewportWidget and never appear on the tool rail.
enum class ViewportInteractionMode {
    // --- Persistent tool modes (user-selectable via tool rail / keyboard) ---
    Select,            ///< Default: click-select, drag-move, marquee-select
    DrawWall,          ///< Chain wall drawing: click to add nodes, double-click or Esc to end (S20-B)
    AddDoor,           ///< Door placement: click on a wall to insert; RMB to flip orientation (S20-D)
    AddWindow,         ///< Window placement: click on a wall to insert; RMB to flip orientation (S20-D)
    Measure,           ///< Overlay distance measurement — pure preview, zero model mutation (S20-A/F)

    // --- Internal transient sub-states (never set via tool rail) ---
    RubberbandSelect,  ///< Active while LMB-dragging empty canvas in Select mode
    DragMove           ///< Active while LMB-dragging a selected entity in Select mode
};

/// Lightweight editor-layer state (mirrors Arcada's EditorStore / Zustand singleton).
/// Owned by MainWindow, shared with ViewportWidget via non-owning pointer.
/// Holds UI-layer toggles and the active persistent tool — NOT architectural model state.
struct EditState {
    ViewportInteractionMode activeTool = ViewportInteractionMode::Select;
    bool snapEnabled    = true;   ///< Grid + semantic geometry snapping (Rule 12)
    bool gridVisible    = true;   ///< Grid line display on canvas
    bool labelsVisible  = true;   ///< Furniture W×H size labels on canvas
};

} // namespace kalara::editor
