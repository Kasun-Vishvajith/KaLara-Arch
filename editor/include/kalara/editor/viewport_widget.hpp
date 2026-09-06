#pragma once
#include "kalara/editor/edit_state.hpp"          // ViewportInteractionMode + EditState (S20-A)
#include "kalara/editor/viewport_state.hpp"
#include "kalara/editor/grid_settings.hpp"
#include "kalara/runtime/selection_manager.hpp"
#include "kalara/runtime/snapping_engine.hpp"
#include "kalara/runtime/model_manipulator.hpp"
#include "kalara/architecture/project.hpp"
#include <QOpenGLWidget>
#include <QPoint>
#include <optional>
#include <vector>

namespace kalara::editor {

/// GPU-accelerated 2D Viewport Widget for architectural planning.
class ViewportWidget : public QOpenGLWidget {
    Q_OBJECT

public:
    explicit ViewportWidget(QWidget *parent = nullptr);
    ~ViewportWidget() override = default;

    [[nodiscard]] ViewportState& viewportState() noexcept { return m_state; }
    [[nodiscard]] const ViewportState& viewportState() const noexcept { return m_state; }

    [[nodiscard]] GridSettings& gridSettings() noexcept { return m_grid; }
    [[nodiscard]] const GridSettings& gridSettings() const noexcept { return m_grid; }

    [[nodiscard]] kalara::runtime::SelectionManager& selectionManager() noexcept { return m_selection; }

    void setProject(kalara::architecture::Project* project) noexcept { m_project = project; update(); }

    /// Provide the shared editor state (owned by MainWindow, non-owning pointer).
    void setEditState(EditState* state) noexcept;

    /// Switch tool mode: cancels any in-progress operation, updates cursor, emits toolChanged.
    /// Mirrors Arcada's resetTools() + setTool() pattern (§3.1).
    void setActiveTool(ViewportInteractionMode mode);

    /// Enable / disable orthogonal constraint mode (Shift key or toggle).
    void setOrthogonalMode(bool enabled) noexcept { m_orthogonalMode = enabled; }
    [[nodiscard]] bool isOrthogonalMode() const noexcept { return m_orthogonalMode; }

    /// Current cursor position in world millimetres.
    [[nodiscard]] kalara::core::geometry::Point2D cursorWorld_mm() const noexcept { return m_cursorWorld; }

    /// Manipulation commands (used by MainWindow and keyboard shortcuts)
    void moveSelection(const kalara::core::geometry::Vector2D& delta_mm, bool propagateConnected = false);
    void rotateSelection(kalara::core::geometry::Angle angle);
    void alignSelection(kalara::runtime::AlignmentType alignment);
    void deleteSelection();

signals:
    void cursorCoordinatesChanged(double x_mm, double y_mm);
    void zoomChanged(double scale);
    void selectionChanged();
    /// Emitted whenever the active tool mode changes (tool rail click, keyboard shortcut, or Escape).
    /// MainWindow connects to this to sync the tool rail checked state and display a toast hint.
    void toolChanged(ViewportInteractionMode mode);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    // --- Rendering helpers ---
    void drawGrid(QPainter& painter);
    void drawOrigin(QPainter& painter);
    void drawSiteAndBuildings(QPainter& painter);
    void drawSitePlan(QPainter& painter, const kalara::architecture::Site& site);
    void drawNorthArrow(QPainter& painter, double northAngle_deg = 0.0);
    void drawMarquee(QPainter& painter);
    void drawSnapIndicator(QPainter& painter);
    /// Preview overlay: rubber-band line + live length label for DrawWall and Measure modes (S20-A).
    void drawPreviewLayer(QPainter& painter);

    // --- Level access ---
    kalara::architecture::Level* activeLevel() const;

    // --- Tool state helpers ---
    /// Cancel any in-progress tool operation (wall chain, measure drag) and clear preview state.
    /// Called by setActiveTool() and Escape — mirrors Arcada's resetTools().
    void cancelInProgressOperation();

    // --- Core state ---
    ViewportState m_state;
    GridSettings  m_grid;
    kalara::runtime::SelectionManager m_selection;
    kalara::architecture::Project*    m_project   = nullptr;
    EditState*                        m_editState = nullptr; ///< Non-owning; owned by MainWindow

    bool   m_isPanning = false;
    QPoint m_lastMousePos;
    kalara::core::geometry::Point2D m_cursorWorld{0.0, 0.0};
    bool   m_orthogonalMode = false;

    // --- Interaction mode and drag state ---
    ViewportInteractionMode m_mode = ViewportInteractionMode::Select;
    QPoint                  m_pressScreenPos;
    kalara::core::geometry::Point2D m_pressWorldPos{0.0, 0.0};
    kalara::core::geometry::Point2D m_dragCurrentWorldPos{0.0, 0.0};
    kalara::runtime::SnapResult     m_currentSnap;

    // --- Preview / in-progress state (editor-only overlay — never persisted, never in model) ---
    /// Live cursor endpoint used by drawPreviewLayer() for rubber-band animation.
    kalara::core::geometry::Point2D m_previewEndPoint{0.0, 0.0};

    /// DrawWall chain nodes accumulated since the user entered DrawWall mode.
    /// S20-A: data structure in place; commit logic implemented in S20-B.
    bool m_drawingWall = false;
    std::vector<kalara::core::geometry::Point2D> m_wallChainNodes;

    /// Measure overlay state (S20-A: fully functional — pure preview, no model mutation).
    bool m_measuring = false;
    kalara::core::geometry::Point2D m_measureStart{0.0, 0.0};
};

} // namespace kalara::editor
