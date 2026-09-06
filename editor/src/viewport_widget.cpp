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
    drawMarquee(painter);
    drawSnapIndicator(painter);
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

        // Draw Site Planning elements (roads, landscape, setbacks, parking, pools, spot elevations)
        drawSitePlan(painter, *site);

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

                // 6. Draw Associative Dimensions (extension lines, dimension line, ticks, text)
                for (const auto& dim : lvl->dimensions()) {
                    bool isSelected = m_selection.isSelected(dim->id);
                    auto [d1, d2] = dim->dimensionLine();

                    auto p1s = m_state.worldToScreen(dim->point1);
                    auto p2s = m_state.worldToScreen(dim->point2);
                    auto d1s = m_state.worldToScreen(d1);
                    auto d2s = m_state.worldToScreen(d2);

                    QColor dimColor = isSelected ? QColor(80, 220, 240) : QColor(210, 215, 225);
                    painter.setPen(QPen(dimColor, 1));

                    // Extension witness lines
                    painter.drawLine(QPointF(p1s.x, p1s.y), QPointF(d1s.x, d1s.y));
                    painter.drawLine(QPointF(p2s.x, p2s.y), QPointF(d2s.x, d2s.y));

                    // Dimension line
                    painter.setPen(QPen(dimColor, 1.5));
                    painter.drawLine(QPointF(d1s.x, d1s.y), QPointF(d2s.x, d2s.y));

                    // 45-degree architectural slash ticks at endpoints
                    double tick = 5.0;
                    painter.drawLine(QPointF(d1s.x - tick, d1s.y + tick), QPointF(d1s.x + tick, d1s.y - tick));
                    painter.drawLine(QPointF(d2s.x - tick, d2s.y + tick), QPointF(d2s.x + tick, d2s.y - tick));

                    // Dimension text centered along dimension line
                    double midX = (d1s.x + d2s.x) * 0.5;
                    double midY = (d1s.y + d2s.y) * 0.5;

                    QFont font = painter.font();
                    font.setPointSize(8);
                    painter.setFont(font);

                    QString distStr = QString::fromStdString(dim->formattedText(m_project->displayUnit));
                    QRectF textRect(midX - 50.0, midY - 18.0, 100.0, 20.0);
                    painter.drawText(textRect, Qt::AlignCenter, distStr);
                }

                // 7. Draw Notes & Annotations
                for (const auto& note : lvl->notes()) {
                    auto pScreen = m_state.worldToScreen(note->position);
                    painter.setPen(QColor(230, 230, 180));
                    QFont font = painter.font();
                    font.setPointSize(8);
                    painter.setFont(font);
                    painter.drawText(QPointF(pScreen.x, pScreen.y), QString::fromStdString(note->text));
                }

                // 8. Draw Library Instances (Step 11)
                for (const auto& inst : lvl->libraryInstances()) {
                    bool isSelected = m_selection.isSelected(inst->id);
                    auto worldPoly = inst->worldBoundary();
                    QPolygonF poly;
                    for (const auto& pt : worldPoly) {
                        auto s = m_state.worldToScreen(pt);
                        poly.append(QPointF(s.x, s.y));
                    }

                    // Clearance zone outline if selected or has clearance
                    if (inst->clearance.hasClearance()) {
                        auto clearPoly = inst->worldClearanceZone();
                        QPolygonF cPoly;
                        for (const auto& pt : clearPoly) {
                            auto s = m_state.worldToScreen(pt);
                            cPoly.append(QPointF(s.x, s.y));
                        }
                        painter.setPen(QPen(QColor(255, 180, 50, 100), 1, Qt::DashLine));
                        painter.setBrush(QColor(255, 180, 50, 15));
                        painter.drawPolygon(cPoly);
                    }

                    if (isSelected) {
                        painter.setPen(QPen(QColor(80, 220, 240), 2));
                        painter.setBrush(QColor(80, 220, 240, 70));
                    } else {
                        // Category based color
                        QColor strokeColor(180, 185, 200);
                        QColor fillColor(60, 64, 75, 180);
                        if (inst->semanticType == kalara::architecture::SemanticType::Fixture) {
                            strokeColor = QColor(100, 200, 240); // Fixture teal
                            fillColor = QColor(40, 70, 90, 180);
                        } else if (inst->semanticType == kalara::architecture::SemanticType::Stair) {
                            strokeColor = QColor(220, 160, 80); // Stair amber
                            fillColor = QColor(80, 60, 40, 180);
                        } else if (inst->semanticType == kalara::architecture::SemanticType::SiteElement) {
                            strokeColor = QColor(120, 200, 100); // Site green
                            fillColor = QColor(50, 80, 50, 180);
                        }
                        painter.setPen(QPen(strokeColor, 1.5));
                        painter.setBrush(fillColor);
                    }
                    painter.drawPolygon(poly);

                    // Name label inside or beside object
                    auto centerScreen = m_state.worldToScreen(inst->position);
                    painter.setPen(QColor(220, 225, 235));
                    QFont font = painter.font();
                    font.setPointSize(8);
                    painter.setFont(font);
                    QRectF textRect(centerScreen.x - 60.0, centerScreen.y - 12.0, 120.0, 24.0);
                    painter.drawText(textRect, Qt::AlignCenter, QString::fromStdString(inst->name));
                }

                // 9. Draw Roofs (Step 12)
                for (const auto& roof : lvl->roofs()) {
                    bool isSelected = m_selection.isSelected(roof->id);
                    QPolygonF rPoly;
                    for (const auto& p : roof->eaveBoundary) {
                        auto s = m_state.worldToScreen(p);
                        rPoly.append(QPointF(s.x, s.y));
                    }

                    if (isSelected) {
                        painter.setPen(QPen(QColor(80, 220, 240), 2));
                        painter.setBrush(QColor(180, 80, 60, 90));
                    } else {
                        // Terracotta / slate roof drafting representation with dark outline
                        painter.setPen(QPen(QColor(210, 90, 70), 2));
                        painter.setBrush(QColor(160, 70, 50, 60));
                    }
                    painter.drawPolygon(rPoly);

                    // Draw ridge lines
                    painter.setPen(QPen(QColor(240, 240, 240), 1.5, Qt::SolidLine));
                    for (const auto& ridge : roof->ridgeLines) {
                        auto s1 = m_state.worldToScreen(ridge.start);
                        auto s2 = m_state.worldToScreen(ridge.end);
                        painter.drawLine(QPointF(s1.x, s1.y), QPointF(s2.x, s2.y));
                    }

                    // Roof slope & pitch label
                    if (roof->eaveBoundary.size() >= 3) {
                        auto bb = roof->boundingBox();
                        auto center = m_state.worldToScreen(bb.center());
                        painter.setPen(QColor(240, 230, 220));
                        painter.drawText(QRectF(center.x - 60.0, center.y - 10.0, 120.0, 20.0),
                                         Qt::AlignCenter,
                                         QString("%1 (%2°)").arg(QString::fromStdString(roof->name)).arg(roof->pitch_deg, 0, 'f', 1));
                    }
                }
            }

            // 10. Draw Underlay / Ghost Level Reference (Step 12)
            auto* activeLvl = bld->activeLevel();
            if (activeLvl && activeLvl->underlayLevelId.has_value()) {
                auto* underlayLvl = bld->findLevel(activeLvl->underlayLevelId.value());
                if (underlayLvl) {
                    painter.setPen(QPen(QColor(100, 160, 255, 120), 1.2, Qt::DashLine));
                    painter.setBrush(Qt::NoBrush);
                    for (const auto& uw : underlayLvl->walls()) {
                        auto corners = uw->boundaryPolygon();
                        QPolygonF upoly;
                        for (const auto& c : corners) {
                            auto s = m_state.worldToScreen(c);
                            upoly.append(QPointF(s.x, s.y));
                        }
                        painter.drawPolygon(upoly);
                    }
                }
            }
        }
    }

    // 8. Draw North Arrow in top-right viewport corner
    double northAngle = 0.0;
    if (m_project && m_project->defaultSite()) {
        northAngle = m_project->defaultSite()->northAngle_deg;
    }
    drawNorthArrow(painter, northAngle);
}

void ViewportWidget::drawSitePlan(QPainter& painter, const kalara::architecture::Site& site) {
    // 1. Draw Roads
    for (const auto& road : site.roads()) {
        auto dir = road->centerline.direction();
        kalara::core::geometry::Vector2D perp(-dir.dy, dir.dx);
        double hw = road->width_mm * 0.5;

        kalara::core::geometry::Point2D p1 = road->centerline.start + perp * hw;
        kalara::core::geometry::Point2D p2 = road->centerline.end + perp * hw;
        kalara::core::geometry::Point2D p3 = road->centerline.end - perp * hw;
        kalara::core::geometry::Point2D p4 = road->centerline.start - perp * hw;

        QPolygonF roadPoly;
        for (const auto& pt : {p1, p2, p3, p4}) {
            auto s = m_state.worldToScreen(pt);
            roadPoly.append(QPointF(s.x, s.y));
        }
        painter.setPen(QPen(QColor(120, 125, 135), 1.5));
        painter.setBrush(QColor(50, 53, 60, 180));
        painter.drawPolygon(roadPoly);

        // Centerline dashed
        auto cs = m_state.worldToScreen(road->centerline.start);
        auto ce = m_state.worldToScreen(road->centerline.end);
        painter.setPen(QPen(QColor(230, 210, 100, 160), 1.0, Qt::DashLine));
        painter.drawLine(QPointF(cs.x, cs.y), QPointF(ce.x, ce.y));

        // Road Name
        auto mid = m_state.worldToScreen(road->centerline.midpoint());
        painter.setPen(QColor(200, 205, 215));
        QFont f = painter.font();
        f.setPointSize(8);
        painter.setFont(f);
        painter.drawText(QRectF(mid.x - 100, mid.y - 12, 200, 24), Qt::AlignCenter, QString::fromStdString(road->name));
    }

    // 2. Draw Landscape Zones
    for (const auto& lnd : site.landscapeZones()) {
        QPolygonF poly;
        for (const auto& p : lnd->boundary) {
            auto s = m_state.worldToScreen(p);
            poly.append(QPointF(s.x, s.y));
        }
        QColor fillCol;
        QColor borderCol;
        switch (lnd->type) {
            case kalara::architecture::LandscapeType::Lawn:
                fillCol = QColor(65, 145, 75, 45);
                borderCol = QColor(50, 130, 60, 120);
                break;
            case kalara::architecture::LandscapeType::Garden:
                fillCol = QColor(100, 140, 65, 45);
                borderCol = QColor(85, 120, 50, 120);
                break;
            case kalara::architecture::LandscapeType::PavedDriveway:
            case kalara::architecture::LandscapeType::Hardscape:
                fillCol = QColor(70, 75, 85, 70);
                borderCol = QColor(100, 105, 115, 140);
                break;
            case kalara::architecture::LandscapeType::PavedPatio:
            case kalara::architecture::LandscapeType::Deck:
                fillCol = QColor(180, 160, 130, 50);
                borderCol = QColor(150, 130, 100, 120);
                break;
        }
        painter.setPen(QPen(borderCol, 1.0, Qt::DashLine));
        painter.setBrush(fillCol);
        painter.drawPolygon(poly);

        // Zone label
        if (!lnd->boundary.empty()) {
            auto bbox = kalara::core::geometry::GeometricOps::boundingBox(lnd->boundary);
            auto c = m_state.worldToScreen(bbox.center());
            painter.setPen(QColor(180, 190, 180));
            QFont f = painter.font();
            f.setPointSize(8);
            painter.setFont(f);
            painter.drawText(QRectF(c.x - 75, c.y - 10, 150, 20), Qt::AlignCenter,
                             QString::fromStdString(lnd->name) + QString(" (%1 m²)").arg(lnd->area_mm2() / 1e6, 0, 'f', 1));
        }
    }

    // 3. Draw Setback Envelope (Buildable Area)
    auto envelope = site.buildableEnvelope();
    if (envelope.size() >= 3) {
        QPolygonF envPoly;
        for (const auto& p : envelope) {
            auto s = m_state.worldToScreen(p);
            envPoly.append(QPointF(s.x, s.y));
        }
        painter.setPen(QPen(QColor(230, 110, 70, 180), 1.5, Qt::DashDotLine));
        painter.setBrush(QColor(230, 110, 70, 15));
        painter.drawPolygon(envPoly);

        // Label along top of envelope
        auto topPt = m_state.worldToScreen(envelope[2]);
        painter.setPen(QColor(230, 120, 80, 200));
        QFont f = painter.font();
        f.setPointSize(8);
        painter.setFont(f);
        painter.drawText(QRectF(topPt.x - 120, topPt.y - 18, 240, 18), Qt::AlignCenter, "Buildable Setback Envelope");
    }

    // 4. Draw Parking Zones
    for (const auto& prk : site.parkingZones()) {
        auto stalls = prk->stallOutlines();
        for (size_t i = 0; i < stalls.size(); ++i) {
            QPolygonF stallPoly;
            for (const auto& pt : stalls[i]) {
                auto s = m_state.worldToScreen(pt);
                stallPoly.append(QPointF(s.x, s.y));
            }
            painter.setPen(QPen(QColor(240, 240, 210, 200), 1.5));
            painter.setBrush(QColor(60, 65, 75, 80));
            painter.drawPolygon(stallPoly);

            // Stall label "P1", "P2"
            auto sc = m_state.worldToScreen(kalara::core::geometry::GeometricOps::boundingBox(stalls[i]).center());
            painter.setPen(QColor(240, 240, 210));
            QFont f = painter.font();
            f.setPointSize(8);
            f.setBold(true);
            painter.setFont(f);
            painter.drawText(QRectF(sc.x - 20, sc.y - 10, 40, 20), Qt::AlignCenter,
                             (prk->isAccessible && i == 0) ? QString("♿ P%1").arg(i + 1) : QString("P%1").arg(i + 1));
        }
    }

    // 5. Draw Entrances
    for (const auto& ent : site.entrances()) {
        auto s = m_state.worldToScreen(ent->position);
        double sw = ent->width_mm * m_state.scale;
        painter.setPen(QPen(QColor(240, 200, 80), 2.0));
        painter.drawLine(QPointF(s.x - sw * 0.5, s.y), QPointF(s.x + sw * 0.5, s.y));

        painter.setPen(QColor(240, 200, 80));
        QFont f = painter.font();
        f.setPointSize(8);
        painter.setFont(f);
        painter.drawText(QRectF(s.x - 75, s.y - 16, 150, 16), Qt::AlignCenter, QString::fromStdString(ent->name));
    }

    // 6. Draw Pools & Outdoor Elements
    for (const auto& out : site.outdoorElements()) {
        // Draw coping deck
        auto coping = out->copingBoundary();
        if (!coping.empty()) {
            QPolygonF copingPoly;
            for (const auto& p : coping) {
                auto s = m_state.worldToScreen(p);
                copingPoly.append(QPointF(s.x, s.y));
            }
            painter.setPen(QPen(QColor(170, 165, 155), 1.5));
            painter.setBrush(QColor(185, 180, 170, 90));
            painter.drawPolygon(copingPoly);
        }

        // Draw pool water
        QPolygonF waterPoly;
        for (const auto& p : out->boundary) {
            auto s = m_state.worldToScreen(p);
            waterPoly.append(QPointF(s.x, s.y));
        }
        painter.setPen(QPen(QColor(40, 170, 230), 2.0));
        painter.setBrush(QColor(40, 170, 230, 130));
        painter.drawPolygon(waterPoly);

        // Water ripple detail line
        if (waterPoly.size() >= 4) {
            painter.setPen(QPen(QColor(255, 255, 255, 90), 1.0, Qt::DashLine));
            QPointF midL = (waterPoly[0] + waterPoly[3]) * 0.5;
            QPointF midR = (waterPoly[1] + waterPoly[2]) * 0.5;
            painter.drawLine(midL, midR);
        }

        // Label
        auto wc = m_state.worldToScreen(kalara::core::geometry::GeometricOps::boundingBox(out->boundary).center());
        painter.setPen(QColor(255, 255, 255));
        QFont f = painter.font();
        f.setPointSize(8);
        f.setBold(true);
        painter.setFont(f);
        painter.drawText(QRectF(wc.x - 80, wc.y - 16, 160, 32), Qt::AlignCenter,
                         QString::fromStdString(out->name) + QString("\n%1 m² | %2 L").arg(out->waterArea_mm2() / 1e6, 0, 'f', 1).arg(static_cast<int>(std::round(out->waterVolume_liters()))));
    }

    // 7. Draw Spot Elevations
    for (const auto& spot : site.spotElevations) {
        auto s = m_state.worldToScreen(spot.position);
        painter.setPen(QPen(QColor(180, 220, 240), 1.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(QPointF(s.x, s.y), 4.0, 4.0);
        painter.drawLine(QPointF(s.x - 7.0, s.y), QPointF(s.x + 7.0, s.y));
        painter.drawLine(QPointF(s.x, s.y - 7.0), QPointF(s.x, s.y + 7.0));

        painter.setPen(QColor(180, 220, 240));
        QFont f = painter.font();
        f.setPointSize(8);
        painter.setFont(f);
        QString label = spot.label.empty() ? QString("%1 mm").arg(spot.elevation_mm, 0, 'f', 0) : QString::fromStdString(spot.label);
        painter.drawText(QPointF(s.x + 8.0, s.y - 2.0), label);
    }
}

void ViewportWidget::drawNorthArrow(QPainter& painter, double northAngle_deg) {
    double nx = m_state.viewportWidth - 60.0;
    double ny = 60.0;

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);

    // North arrow circle & pointer
    painter.setPen(QPen(QColor(180, 185, 195), 1.5));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(QPointF(nx, ny), 22.0, 22.0);

    // Translate and rotate needle by northAngle_deg
    painter.translate(nx, ny);
    painter.rotate(northAngle_deg);

    // Arrow pointer towards North (-Y in screen space)
    QPolygonF arrow;
    arrow << QPointF(0, -18.0) << QPointF(6.0, 10.0) << QPointF(0, 4.0);
    painter.setBrush(QColor(220, 60, 60));
    painter.setPen(Qt::NoPen);
    painter.drawPolygon(arrow);

    QPolygonF arrowLeft;
    arrowLeft << QPointF(0, -18.0) << QPointF(-6.0, 10.0) << QPointF(0, 4.0);
    painter.setBrush(QColor(180, 185, 195));
    painter.drawPolygon(arrowLeft);

    // "N" label
    QFont font = painter.font();
    font.setPointSize(8);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(QColor(240, 240, 240));
    painter.drawText(QRectF(-15.0, -36.0, 30.0, 16.0), Qt::AlignCenter, "N");

    painter.restore();
}

kalara::architecture::Level* ViewportWidget::activeLevel() const {
    if (!m_project) return nullptr;
    auto* site = m_project->defaultSite();
    if (!site || site->buildings().empty()) return nullptr;
    auto* bld = site->buildings().front().get();
    if (!bld) return nullptr;
    return bld->activeLevel();
}

void ViewportWidget::drawMarquee(QPainter& painter) {
    if (m_mode != ViewportInteractionMode::RubberbandSelect) return;

    auto p1 = m_state.worldToScreen(m_pressWorldPos);
    auto p2 = m_state.worldToScreen(m_dragCurrentWorldPos);

    double rx = std::min(p1.x, p2.x);
    double ry = std::min(p1.y, p2.y);
    double rw = std::abs(p2.x - p1.x);
    double rh = std::abs(p2.y - p1.y);

    QRectF rect(rx, ry, rw, rh);
    painter.setPen(QPen(QColor(80, 180, 255, 220), 1.5, Qt::DashLine));
    painter.setBrush(QColor(80, 180, 255, 35));
    painter.drawRect(rect);
}

void ViewportWidget::drawSnapIndicator(QPainter& painter) {
    if (!m_currentSnap.snapped) return;

    auto s = m_state.worldToScreen(m_currentSnap.point);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_currentSnap.type == kalara::runtime::SnapType::Endpoint) {
        // Yellow square for endpoint snap
        painter.setPen(QPen(QColor(255, 215, 0), 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(QRectF(s.x - 5.0, s.y - 5.0, 10.0, 10.0));
    } else if (m_currentSnap.type == kalara::runtime::SnapType::Midpoint) {
        // Cyan triangle for midpoint snap
        painter.setPen(QPen(QColor(0, 230, 255), 2));
        painter.setBrush(Qt::NoBrush);
        QPolygonF tri;
        tri << QPointF(s.x, s.y - 6.0) << QPointF(s.x + 6.0, s.y + 5.0) << QPointF(s.x - 6.0, s.y + 5.0);
        painter.drawPolygon(tri);
    } else if (m_currentSnap.type == kalara::runtime::SnapType::WallCenterline) {
        // Hourglass / cross for centerline snap
        painter.setPen(QPen(QColor(180, 255, 120), 2));
        painter.drawLine(QPointF(s.x - 4.0, s.y - 4.0), QPointF(s.x + 4.0, s.y + 4.0));
        painter.drawLine(QPointF(s.x - 4.0, s.y + 4.0), QPointF(s.x + 4.0, s.y - 4.0));
    } else if (m_currentSnap.type == kalara::runtime::SnapType::Grid) {
        // Subtle white circle for grid snap
        painter.setPen(QPen(QColor(200, 200, 200, 180), 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(QPointF(s.x, s.y), 4.0, 4.0);
    }

    painter.restore();
}

void ViewportWidget::moveSelection(const kalara::core::geometry::Vector2D& delta_mm, bool propagateConnected) {
    auto* lvl = activeLevel();
    if (!lvl || m_selection.empty()) return;

    kalara::runtime::ModelManipulator::moveEntities(*lvl, m_selection.selectedList(), delta_mm, propagateConnected);
    update();
}

void ViewportWidget::rotateSelection(kalara::core::geometry::Angle angle) {
    auto* lvl = activeLevel();
    if (!lvl || m_selection.empty()) return;

    auto summary = m_selection.summarize(*lvl);
    kalara::core::geometry::Point2D pivot(0.0, 0.0);
    if (summary.boundingBox.has_value()) {
        pivot = summary.boundingBox->center();
    }

    kalara::runtime::ModelManipulator::rotateEntities(*lvl, m_selection.selectedList(), pivot, angle);
    update();
}

void ViewportWidget::alignSelection(kalara::runtime::AlignmentType alignment) {
    auto* lvl = activeLevel();
    if (!lvl || m_selection.count() < 2) return;

    kalara::runtime::ModelManipulator::alignEntities(*lvl, m_selection.selectedList(), alignment);
    update();
}

void ViewportWidget::deleteSelection() {
    auto* lvl = activeLevel();
    if (!lvl || m_selection.empty()) return;

    auto list = m_selection.selectedList();
    for (const auto& id : list) {
        if (lvl->removeWall(id)) continue;
        if (lvl->removeRoom(id)) continue;
        if (lvl->removeDoor(id)) continue;
        if (lvl->removeWindow(id)) continue;
        if (lvl->removeLibraryInstance(id)) continue;
        if (lvl->removeRoof(id)) continue;
        if (lvl->removeDimension(id)) continue;
        if (lvl->removeNote(id)) continue;
    }
    m_selection.clear();
    emit selectionChanged();
    update();
}

void ViewportWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton ||
        (event->button() == Qt::LeftButton && (event->modifiers() & Qt::AltModifier))) {
        m_isPanning = true;
        m_lastMousePos = event->pos();
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        m_pressScreenPos = event->pos();
        auto rawWorld = m_state.screenToWorld(event->position().x(), event->position().y());
        m_pressWorldPos = rawWorld;
        m_dragCurrentWorldPos = rawWorld;

        // Check if user clicked on an entity
        auto* lvl = activeLevel();
        std::optional<kalara::architecture::EntityId> hitId;

        if (lvl) {
            // 1. Check doors
            for (const auto& door : lvl->doors()) {
                auto* w = lvl->findWall(door->hostWallId);
                if (w && door->containsPoint(rawWorld, *w)) {
                    hitId = door->id;
                    break;
                }
            }
            // 2. Check windows
            if (!hitId) {
                for (const auto& win : lvl->windows()) {
                    auto* w = lvl->findWall(win->hostWallId);
                    if (w && win->containsPoint(rawWorld, *w)) {
                        hitId = win->id;
                        break;
                    }
                }
            }
            // 3. Check library instances (furniture, fixtures, etc.)
            if (!hitId) {
                for (const auto& inst : lvl->libraryInstances()) {
                    if (inst->containsPoint(rawWorld)) {
                        hitId = inst->id;
                        break;
                    }
                }
            }
            // 4. Check walls
            if (!hitId) {
                for (const auto& wall : lvl->walls()) {
                    if (wall->containsPoint(rawWorld)) {
                        hitId = wall->id;
                        break;
                    }
                }
            }
            // 5. Check rooms
            if (!hitId) {
                for (const auto& room : lvl->rooms()) {
                    if (room->containsPoint(rawWorld)) {
                        hitId = room->id;
                        break;
                    }
                }
            }
            // 6. Check roofs (Step 12)
            if (!hitId) {
                for (const auto& roof : lvl->roofs()) {
                    if (roof->containsPoint(rawWorld)) {
                        hitId = roof->id;
                        break;
                    }
                }
            }
        }

        if (hitId) {
            if (event->modifiers() & Qt::ShiftModifier) {
                // Multi-select toggle
                m_selection.toggle(*hitId);
            } else if (event->modifiers() & Qt::ControlModifier) {
                // Additive select
                m_selection.select(*hitId);
            } else {
                // If clicked item is not in current selection, select only it
                if (!m_selection.isSelected(*hitId)) {
                    m_selection.clear();
                    m_selection.select(*hitId);
                }
            }
            // Prepare for potential drag move of selected entities
            m_mode = ViewportInteractionMode::DragMove;
            emit selectionChanged();
        } else {
            // Clicked empty canvas space
            if (!(event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier))) {
                m_selection.clear();
                emit selectionChanged();
            }
            // Start rubberband marquee selection
            m_mode = ViewportInteractionMode::RubberbandSelect;
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
        return;
    }

    auto rawWorld = m_state.screenToWorld(event->position().x(), event->position().y());
    auto* lvl = activeLevel();

    // Perform semantic snapping (Rule 12)
    if (lvl) {
        m_currentSnap = kalara::runtime::SnappingEngine::snap(
            rawWorld, *lvl, 150.0 / m_state.scale, m_grid.snapEnabled, m_grid.secondarySpacing_mm
        );
    } else {
        m_currentSnap.snapped = false;
    }

    if (m_currentSnap.snapped) {
        m_cursorWorld = m_currentSnap.point;
    } else if (m_grid.snapEnabled) {
        m_cursorWorld = m_grid.snap(rawWorld);
    } else {
        m_cursorWorld = rawWorld;
    }

    if (m_orthogonalMode) {
        m_cursorWorld = kalara::core::geometry::GeometricOps::snapToOrthogonal(
            kalara::core::geometry::Point2D(0.0, 0.0), m_cursorWorld
        );
    }

    m_dragCurrentWorldPos = m_cursorWorld;

    if (m_mode == ViewportInteractionMode::RubberbandSelect) {
        update();
    } else if (m_mode == ViewportInteractionMode::DragMove && (event->buttons() & Qt::LeftButton)) {
        // Interactive live drag move
        if (lvl && !m_selection.empty()) {
            kalara::core::geometry::Vector2D delta = m_cursorWorld - m_pressWorldPos;
            if (delta.lengthSquared() > 0.0) {
                kalara::runtime::ModelManipulator::moveEntities(*lvl, m_selection.selectedList(), delta, false);
                m_pressWorldPos = m_cursorWorld;
            }
        }
        update();
    }

    emit cursorCoordinatesChanged(m_cursorWorld.x, m_cursorWorld.y);
    event->accept();
}

void ViewportWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton ||
        (event->button() == Qt::LeftButton && m_isPanning)) {
        m_isPanning = false;
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (m_mode == ViewportInteractionMode::RubberbandSelect) {
            auto* lvl = activeLevel();
            if (lvl) {
                double minX = std::min(m_pressWorldPos.x, m_dragCurrentWorldPos.x);
                double maxX = std::max(m_pressWorldPos.x, m_dragCurrentWorldPos.x);
                double minY = std::min(m_pressWorldPos.y, m_dragCurrentWorldPos.y);
                double maxY = std::max(m_pressWorldPos.y, m_dragCurrentWorldPos.y);

                // If drag was greater than a small click threshold, perform box select
                if ((maxX - minX > 50.0) || (maxY - minY > 50.0)) {
                    kalara::core::geometry::Rect2D marquee(minX, minY, maxX - minX, maxY - minY);
                    bool additive = (event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier));
                    m_selection.selectInRect(marquee, *lvl, additive);
                    emit selectionChanged();
                }
            }
        }

        m_mode = ViewportInteractionMode::Select;
        update();
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
    // Manipulation keyboard shortcuts
    if (event->key() == Qt::Key_Left) {
        moveSelection(kalara::core::geometry::Vector2D(-100.0, 0.0));
        event->accept();
        return;
    } else if (event->key() == Qt::Key_Right) {
        moveSelection(kalara::core::geometry::Vector2D(100.0, 0.0));
        event->accept();
        return;
    } else if (event->key() == Qt::Key_Up) {
        moveSelection(kalara::core::geometry::Vector2D(0.0, 100.0));
        event->accept();
        return;
    } else if (event->key() == Qt::Key_Down) {
        moveSelection(kalara::core::geometry::Vector2D(0.0, -100.0));
        event->accept();
        return;
    } else if (event->key() == Qt::Key_R) {
        // Rotate selected entities 90 degrees CCW
        rotateSelection(kalara::core::geometry::Angle::fromDegrees(90.0));
        event->accept();
        return;
    } else if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        // Delete selected entities
        deleteSelection();
        event->accept();
        return;
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

