#include "kalara/editor/viewport_widget.hpp"
#include "kalara/core/geometry/ops.hpp"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>

namespace kalara::editor {

ViewportWidget::ViewportWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void ViewportWidget::initializeGL() {
    // QOpenGLWidget initialization
}

void ViewportWidget::resizeGL(int w, int h) {
    m_state.viewportWidth = w;
    m_state.viewportHeight = h;
}

void ViewportWidget::paintGL() {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Dark architectural drafting canvas background
    painter.fillRect(rect(), QColor(32, 34, 38));

    drawGrid(painter);
    drawOrigin(painter);
    drawSiteAndBuildings(painter);
}

void ViewportWidget::drawGrid(QPainter& painter) {
    if (!m_grid.enabled) return;

    auto bounds = m_state.visibleWorldBounds();
    double spacing = m_grid.secondarySpacing_mm;
    if (spacing * m_state.scale < 10.0) {
        // If minor grid lines are too dense on screen, show primary grid
        spacing = m_grid.primarySpacing_mm;
    }
    if (spacing * m_state.scale < 5.0) return;

    QPen minorPen(QColor(48, 52, 60), 1);
    QPen majorPen(QColor(65, 70, 82), 1);

    double startX = std::floor(bounds.min.x / spacing) * spacing;
    double endX = std::ceil(bounds.max.x / spacing) * spacing;
    double startY = std::floor(bounds.min.y / spacing) * spacing;
    double endY = std::ceil(bounds.max.y / spacing) * spacing;

    for (double x = startX; x <= endX; x += spacing) {
        bool isMajor = (std::abs(std::fmod(x, m_grid.primarySpacing_mm)) < 1.0);
        painter.setPen(isMajor ? majorPen : minorPen);
        auto p1 = m_state.worldToScreen(x, bounds.min.y);
        auto p2 = m_state.worldToScreen(x, bounds.max.y);
        painter.drawLine(QPointF(p1.x, p1.y), QPointF(p2.x, p2.y));
    }

    for (double y = startY; y <= endY; y += spacing) {
        bool isMajor = (std::abs(std::fmod(y, m_grid.primarySpacing_mm)) < 1.0);
        painter.setPen(isMajor ? majorPen : minorPen);
        auto p1 = m_state.worldToScreen(bounds.min.x, y);
        auto p2 = m_state.worldToScreen(bounds.max.x, y);
        painter.drawLine(QPointF(p1.x, p1.y), QPointF(p2.x, p2.y));
    }
}

void ViewportWidget::drawOrigin(QPainter& painter) {
    auto originScreen = m_state.worldToScreen(0.0, 0.0);
    double ox = originScreen.x;
    double oy = originScreen.y;

    // Red X-Axis (towards East / positive X)
    painter.setPen(QPen(QColor(220, 60, 60), 2));
    painter.drawLine(QPointF(ox, oy), QPointF(ox + 50.0, oy));

    // Green Y-Axis (towards North / positive Y in architectural world)
    painter.setPen(QPen(QColor(60, 200, 60), 2));
    painter.drawLine(QPointF(ox, oy), QPointF(ox, oy - 50.0));

    // Origin center marker
    painter.setBrush(QBrush(QColor(240, 240, 240)));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(ox, oy), 3.0, 3.0);
}

void ViewportWidget::drawSiteAndBuildings(QPainter& painter) {
    if (!m_project) return;

    for (const auto& site : m_project->sites()) {
        if (site->propertyBoundary.size() >= 3) {
            QPolygonF poly;
            for (const auto& pt : site->propertyBoundary) {
                auto s = m_state.worldToScreen(pt);
                poly.append(QPointF(s.x, s.y));
            }
            // Property boundary: dashed yellow / golden line
            painter.setPen(QPen(QColor(230, 190, 80), 2, Qt::DashLine));
            painter.setBrush(QColor(230, 190, 80, 20));
            painter.drawPolygon(poly);
        }
    }
}

void ViewportWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton ||
        (event->button() == Qt::LeftButton && (event->modifiers() & Qt::AltModifier))) {
        m_isPanning = true;
        m_lastMousePos = event->pos();
        event->accept();
    } else if (event->button() == Qt::LeftButton) {
        // Basic selection ray / test point
        auto worldPos = m_state.screenToWorld(event->position().x(), event->position().y());
        if (m_project) {
            for (const auto& site : m_project->sites()) {
                if (kalara::core::geometry::GeometricOps::pointInPolygon(worldPos, site->propertyBoundary)) {
                    m_selection.select(site->id);
                }
            }
        }
        update();
        event->accept();
    }
}

void ViewportWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_isPanning) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();
        m_state.panByScreenDelta(delta.x(), delta.y());
        update();
        event->accept();
    } else {
        auto rawWorld = m_state.screenToWorld(event->position().x(), event->position().y());
        if (m_grid.snapEnabled) {
            m_cursorWorld = m_grid.snap(rawWorld);
        } else {
            m_cursorWorld = rawWorld;
        }

        if (m_orthogonalMode) {
            m_cursorWorld = kalara::core::geometry::GeometricOps::snapToOrthogonal(
                kalara::core::geometry::Point2D(0.0, 0.0), m_cursorWorld
            );
        }

        emit cursorCoordinatesChanged(m_cursorWorld.x, m_cursorWorld.y);
        event->accept();
    }
}

void ViewportWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton) {
        m_isPanning = false;
        event->accept();
    }
}

void ViewportWidget::wheelEvent(QWheelEvent *event) {
    double angleDelta = event->angleDelta().y();
    double factor = (angleDelta > 0) ? 1.15 : (1.0 / 1.15);
    m_state.zoomAtScreenPoint(factor, event->position().x(), event->position().y());
    emit zoomChanged(m_state.scale);
    update();
    event->accept();
}

void ViewportWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift) {
        setOrthogonalMode(true);
    }
    QOpenGLWidget::keyPressEvent(event);
}

void ViewportWidget::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift) {
        setOrthogonalMode(false);
    }
    QOpenGLWidget::keyReleaseEvent(event);
}

} // namespace kalara::editor
