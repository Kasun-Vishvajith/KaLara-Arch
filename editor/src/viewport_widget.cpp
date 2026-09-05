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

    // 1. Draw Site Boundaries
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

        // 2. Draw Rooms (shaded floor fill, name, and area labels)
        for (const auto& bld : site->buildings()) {
            for (const auto& lvl : bld->levels()) {
                for (const auto& room : lvl->rooms()) {
                    bool isSelected = m_selection.isSelected(room->id);
                    QPolygonF roomPoly;
                    for (const auto& p : room->boundary) {
                        auto s = m_state.worldToScreen(p);
                        roomPoly.append(QPointF(s.x, s.y));
                    }

                    if (isSelected) {
                        painter.setPen(QPen(QColor(80, 220, 240, 180), 1.5, Qt::DashLine));
                        painter.setBrush(QColor(80, 220, 240, 45));
                    } else {
                        painter.setPen(Qt::NoPen);
                        painter.setBrush(QColor(255, 255, 255, 12)); // Subtle floor surface fill
                    }
                    painter.drawPolygon(roomPoly);

                    // Room label (Name + Area in m^2)
                    auto labelPos = m_state.worldToScreen(room->labelPosition());
                    painter.setPen(QColor(220, 225, 235));
                    QFont font = painter.font();
                    font.setPointSize(9);
                    font.setBold(true);
                    painter.setFont(font);

                    QString nameStr = QString::fromStdString(room->name);
                    QString areaStr = QString("%1 m²").arg(room->area_m2(), 0, 'f', 1);

                    QRectF textRect(labelPos.x - 75.0, labelPos.y - 20.0, 150.0, 40.0);
                    painter.drawText(textRect, Qt::AlignCenter, nameStr + "\n" + areaStr);
                }

                // 3. Draw Walls across buildings and levels
                for (const auto& wall : lvl->walls()) {
                    bool isSelected = m_selection.isSelected(wall->id);
                    auto corners = wall->boundaryPolygon();

                    QPolygonF wallPoly;
                    for (const auto& c : corners) {
                        auto s = m_state.worldToScreen(c);
                        wallPoly.append(QPointF(s.x, s.y));
                    }

                    if (isSelected) {
                        // Highlighted selected wall in cyan/teal
                        painter.setPen(QPen(QColor(80, 220, 240), 2));
                        painter.setBrush(QColor(80, 220, 240, 80));
                    } else {
                        // Standard architectural wall: solid dark gray body with crisp edges
                        painter.setPen(QPen(QColor(200, 205, 215), 1.5));
                        painter.setBrush(QColor(110, 115, 125));
                    }
                    painter.drawPolygon(wallPoly);

                    // Draw wall centerline as subtle reference
                    auto startScreen = m_state.worldToScreen(wall->start);
                    auto endScreen = m_state.worldToScreen(wall->end);
                    painter.setPen(QPen(QColor(160, 165, 175, 100), 1, Qt::DashDotLine));
                    painter.drawLine(QPointF(startScreen.x, startScreen.y), QPointF(endScreen.x, endScreen.y));
                }

                // 4. Draw Doors (opening cutout + door leaf + swing arc)
                for (const auto& door : lvl->doors()) {
                    auto* hostWall = lvl->findWall(door->hostWallId);
                    if (!hostWall) continue;

                    bool isSelected = m_selection.isSelected(door->id);
                    auto cutout = door->openingBox(*hostWall);

                    // Clear wall body in opening cutout area
                    QPolygonF cutoutPoly;
                    for (const auto& pt : cutout) {
                        auto s = m_state.worldToScreen(pt);
                        cutoutPoly.append(QPointF(s.x, s.y));
                    }
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(QColor(32, 34, 38)); // Canvas background matches opening void
                    painter.drawPolygon(cutoutPoly);

                    // Draw opening jamb lines
                    auto seg = door->openingSegment(*hostWall);
                    auto p1s = m_state.worldToScreen(cutout[0]);
                    auto p4s = m_state.worldToScreen(cutout[3]);
                    auto p2s = m_state.worldToScreen(cutout[1]);
                    auto p3s = m_state.worldToScreen(cutout[2]);

                    painter.setPen(QPen(isSelected ? QColor(80, 220, 240) : QColor(220, 140, 60), 2));
                    painter.drawLine(QPointF(p1s.x, p1s.y), QPointF(p4s.x, p4s.y));
                    painter.drawLine(QPointF(p2s.x, p2s.y), QPointF(p3s.x, p3s.y));

                    // Door leaf & swing arc
                    auto hinge = door->hingePoint(*hostWall);
                    auto hScreen = m_state.worldToScreen(hinge);
                    double leafLengthScreen = door->width_mm * m_state.scale;

                    painter.setPen(QPen(isSelected ? QColor(80, 220, 240) : QColor(220, 140, 60), 1.5, Qt::DashLine));
                    painter.drawEllipse(QPointF(hScreen.x, hScreen.y), leafLengthScreen, leafLengthScreen);
                }

                // 5. Draw Windows (cutout + frame + glazing lines)
                for (const auto& win : lvl->windows()) {
                    auto* hostWall = lvl->findWall(win->hostWallId);
                    if (!hostWall) continue;

                    bool isSelected = m_selection.isSelected(win->id);
                    auto cutout = win->openingBox(*hostWall);

                    QPolygonF cutoutPoly;
                    for (const auto& pt : cutout) {
                        auto s = m_state.worldToScreen(pt);
                        cutoutPoly.append(QPointF(s.x, s.y));
                    }
                    painter.setPen(QPen(isSelected ? QColor(80, 220, 240) : QColor(100, 180, 255), 2));
                    painter.setBrush(QColor(100, 180, 255, 60)); // Light blue architectural glass tint
                    painter.drawPolygon(cutoutPoly);

                    // Central glazing line
                    auto seg = win->openingSegment(*hostWall);
                    auto s1 = m_state.worldToScreen(seg.start);
                    auto s2 = m_state.worldToScreen(seg.end);
                    painter.setPen(QPen(QColor(240, 245, 255), 1.5));
                    painter.drawLine(QPointF(s1.x, s1.y), QPointF(s2.x, s2.y));
                }
            }
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
        // Selection hit-test order: Doors/Windows -> Walls -> Rooms -> Sites
        auto worldPos = m_state.screenToWorld(event->position().x(), event->position().y());
        bool found = false;

        if (!(event->modifiers() & Qt::ShiftModifier)) {
            m_selection.clear();
        }

        if (m_project) {
            for (const auto& site : m_project->sites()) {
                for (const auto& bld : site->buildings()) {
                    for (const auto& lvl : bld->levels()) {
                        // 1. Check doors
                        for (const auto& door : lvl->doors()) {
                            auto* w = lvl->findWall(door->hostWallId);
                            if (w && door->containsPoint(worldPos, *w)) {
                                m_selection.select(door->id);
                                found = true;
                                break;
                            }
                        }
                        if (found) break;

                        // 2. Check windows
                        for (const auto& win : lvl->windows()) {
                            auto* w = lvl->findWall(win->hostWallId);
                            if (w && win->containsPoint(worldPos, *w)) {
                                m_selection.select(win->id);
                                found = true;
                                break;
                            }
                        }
                        if (found) break;

                        // 3. Check walls
                        for (const auto& wall : lvl->walls()) {
                            if (wall->containsPoint(worldPos)) {
                                m_selection.select(wall->id);
                                found = true;
                                break;
                            }
                        }
                        if (found) break;

                        // 4. Check rooms
                        for (const auto& room : lvl->rooms()) {
                            if (room->containsPoint(worldPos)) {
                                m_selection.select(room->id);
                                found = true;
                                break;
                            }
                        }
                        if (found) break;
                    }
                    if (found) break;
                }
                if (found) break;
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
