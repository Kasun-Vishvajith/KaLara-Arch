#pragma once
#include "kalara/editor/viewport_state.hpp"
#include "kalara/editor/grid_settings.hpp"
#include "kalara/runtime/selection_manager.hpp"
#include "kalara/runtime/snapping_engine.hpp"
#include "kalara/runtime/model_manipulator.hpp"
#include "kalara/architecture/project.hpp"
#include <QOpenGLWidget>
#include <QPoint>
#include <optional>

namespace kalara::editor {

/// Mode of user interaction on the 2D viewport.
enum class ViewportInteractionMode {
    Select,
    RubberbandSelect,
    DragMove
};

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

    /// Enable / disable orthogonal constraint mode (Shift key or toggle).
    void setOrthogonalMode(bool enabled) noexcept { m_orthogonalMode = enabled; }
    [[nodiscard]] bool isOrthogonalMode() const noexcept { return m_orthogonalMode; }

    /// Current cursor position in world millimetres.
    [[nodiscard]] kalara::core::geometry::Point2D cursorWorld_mm() const noexcept { return m_cursorWorld; }

    /// Manipulation commands
    void moveSelection(const kalara::core::geometry::Vector2D& delta_mm, bool propagateConnected = false);
    void rotateSelection(kalara::core::geometry::Angle angle);
    void alignSelection(kalara::runtime::AlignmentType alignment);

signals:
    void cursorCoordinatesChanged(double x_mm, double y_mm);
    void zoomChanged(double scale);
    void selectionChanged();

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
    void drawGrid(QPainter& painter);
    void drawOrigin(QPainter& painter);
    void drawSiteAndBuildings(QPainter& painter);
    void drawNorthArrow(QPainter& painter);
    void drawMarquee(QPainter& painter);
    void drawSnapIndicator(QPainter& painter);

    kalara::architecture::Level* activeLevel() const;

    ViewportState m_state;
    GridSettings m_grid;
    kalara::runtime::SelectionManager m_selection;
    kalara::architecture::Project* m_project = nullptr;

    bool m_isPanning = false;
    QPoint m_lastMousePos;
    kalara::core::geometry::Point2D m_cursorWorld{0.0, 0.0};
    bool m_orthogonalMode = false;

    // Manipulation and interactive drag state
    ViewportInteractionMode m_mode = ViewportInteractionMode::Select;
    QPoint m_pressScreenPos;
    kalara::core::geometry::Point2D m_pressWorldPos{0.0, 0.0};
    kalara::core::geometry::Point2D m_dragCurrentWorldPos{0.0, 0.0};
    kalara::runtime::SnapResult m_currentSnap;
};

} // namespace kalara::editor
