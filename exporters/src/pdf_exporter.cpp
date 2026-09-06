#include "kalara/exporters/pdf_exporter.hpp"
#include "kalara/architecture/wall.hpp"
#include "kalara/architecture/opening.hpp"
#include "kalara/architecture/room.hpp"
#include "kalara/architecture/annotation.hpp"
#include "kalara/core/geometry/ops.hpp"
#include <QPdfWriter>
#include <QPainter>
#include <QPageSize>
#include <QPageLayout>
#include <QDateTime>
#include <QPolygonF>
#include <QFont>
#include <QPen>
#include <QBrush>
#include <cmath>

namespace kalara::exporters {

using namespace kalara::core::geometry;
using namespace kalara::architecture;

static void drawFloorPlanToPainter(QPainter& painter,
                                   const Level& level,
                                   const QRectF& canvasRect,
                                   const ExportSettings& settings) {
    std::vector<Point2D> pts;
    for (const auto& w : level.walls()) {
        pts.push_back(w->start);
        pts.push_back(w->end);
    }
    for (const auto& r : level.rooms()) {
        for (const auto& p : r->boundary) pts.push_back(p);
    }
    if (pts.empty()) {
        pts.push_back(Point2D(-2000.0, -2000.0));
        pts.push_back(Point2D(10000.0, 10000.0));
    }

    auto bbox = GeometricOps::boundingBox(pts);
    double planWidth = bbox.width();
    double planHeight = bbox.height();
    if (planWidth < 100.0) planWidth = 5000.0;
    if (planHeight < 100.0) planHeight = 4000.0;

    // Scale to fit canvas with 10% padding
    double scaleX = (canvasRect.width() * 0.85) / planWidth;
    double scaleY = (canvasRect.height() * 0.85) / planHeight;
    double scale = std::min(scaleX, scaleY);

    double planCenterWorldX = bbox.center().x;
    double planCenterWorldY = bbox.center().y;

    auto toScreen = [&](const Point2D& p) -> QPointF {
        double sx = canvasRect.center().x() + (p.x - planCenterWorldX) * scale;
        // Invert Y so positive world Y is UP on the paper
        double sy = canvasRect.center().y() - (p.y - planCenterWorldY) * scale;
        return QPointF(sx, sy);
    };

    // 1. Rooms
    if (settings.includeRoomTags) {
        painter.setPen(QPen(QColor(120, 140, 160), 1, Qt::DashLine));
        painter.setBrush(QBrush(QColor(230, 240, 250, 120)));
        for (const auto& r : level.rooms()) {
            if (r->boundary.size() >= 3) {
                QPolygonF poly;
                for (const auto& p : r->boundary) {
                    poly << toScreen(p);
                }
                painter.drawPolygon(poly);

                QPointF centroid = toScreen(GeometricOps::boundingBox(r->boundary).center());
                painter.setPen(QColor(40, 60, 80));
                QFont f = painter.font();
                f.setPointSize(10);
                f.setBold(true);
                painter.setFont(f);
                painter.drawText(QRectF(centroid.x() - 100, centroid.y() - 25, 200, 20),
                                 Qt::AlignCenter, QString::fromStdString(r->name));

                double area_m2 = GeometricOps::polygonArea(r->boundary) / 1000000.0;
                f.setPointSize(8);
                f.setBold(false);
                painter.setFont(f);
                painter.drawText(QRectF(centroid.x() - 100, centroid.y() + 5, 200, 20),
                                 Qt::AlignCenter, QString("%1 m²").arg(area_m2, 0, 'f', 1));
            }
        }
    }

    // 2. Walls
    painter.setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(QBrush(QColor(40, 44, 52)));
    for (const auto& w : level.walls()) {
        Vector2D dir = w->end - w->start;
        if (dir.lengthSquared() > 0.001) {
            Vector2D norm = dir.normalized().perpendicular() * (w->thickness_mm * 0.5);
            QPolygonF poly;
            poly << toScreen(w->start + norm)
                 << toScreen(w->end + norm)
                 << toScreen(w->end - norm)
                 << toScreen(w->start - norm);
            painter.drawPolygon(poly);
        }
    }

    // 3. Openings
    if (settings.includeDoorSwings) {
        painter.setPen(QPen(QColor(200, 90, 20), 1.5, Qt::SolidLine));
        painter.setBrush(Qt::NoBrush);
        for (const auto& d : level.doors()) {
            const auto* hostWall = level.findWall(d->hostWallId);
            if (!hostWall) continue;

            Vector2D dir = (hostWall->end - hostWall->start).normalized();
            Vector2D norm = dir.perpendicular();
            Point2D center = hostWall->start + dir * d->offsetAlongWall_mm;
            Point2D hinge = center - dir * (d->width_mm * 0.5);
            Point2D openTip = hinge + norm * d->width_mm;

            painter.drawLine(toScreen(hinge), toScreen(openTip));
        }

        painter.setPen(QPen(QColor(40, 120, 200), 1.5, Qt::SolidLine));
        painter.setBrush(QBrush(QColor(240, 248, 255)));
        for (const auto& win : level.windows()) {
            const auto* hostWall = level.findWall(win->hostWallId);
            if (!hostWall) continue;

            Vector2D dir = (hostWall->end - hostWall->start).normalized();
            Vector2D norm = dir.perpendicular() * (hostWall->thickness_mm * 0.5);
            Point2D center = hostWall->start + dir * win->offsetAlongWall_mm;
            QPolygonF poly;
            poly << toScreen(center - dir * (win->width_mm * 0.5) + norm)
                 << toScreen(center + dir * (win->width_mm * 0.5) + norm)
                 << toScreen(center + dir * (win->width_mm * 0.5) - norm)
                 << toScreen(center - dir * (win->width_mm * 0.5) - norm);
            painter.drawPolygon(poly);
        }
    }

    // 4. Dimensions
    if (settings.includeDimensions) {
        painter.setPen(QPen(QColor(30, 130, 60), 1, Qt::SolidLine));
        painter.setBrush(Qt::NoBrush);
        for (const auto& dim : level.dimensions()) {
            auto dLine = dim->dimensionLine();
            QPointF p1 = toScreen(dLine.first);
            QPointF p2 = toScreen(dLine.second);
            painter.drawLine(p1, p2);

            QPointF mid = (p1 + p2) * 0.5;
            QFont f = painter.font();
            f.setPointSize(7);
            painter.setFont(f);
            painter.drawText(QRectF(mid.x() - 40, mid.y() - 15, 80, 15),
                             Qt::AlignCenter, QString::fromStdString(dim->formattedText()));
        }
    }
}

bool PdfExporter::exportLevelToPdf(const Level& level, const std::string& filePath, const ExportSettings& settings) {
    QPdfWriter writer(QString::fromStdString(filePath));

    QPageSize::PageSizeId pSize = QPageSize::A4;
    if (settings.paperSize == PaperSize::A3) pSize = QPageSize::A3;
    else if (settings.paperSize == PaperSize::A2) pSize = QPageSize::A2;
    else if (settings.paperSize == PaperSize::A1) pSize = QPageSize::A1;

    writer.setPageSize(QPageSize(pSize));
    writer.setPageOrientation(settings.orientation == SheetOrientation::Portrait ? QPageLayout::Portrait : QPageLayout::Landscape);
    writer.setResolution(300);

    QPainter painter(&writer);
    if (!painter.isActive()) return false;

    QRect pageRect = writer.pageLayout().paintRectPixels(writer.resolution());
    double margin = 40.0;
    QRectF outerBorder(pageRect.left() + margin, pageRect.top() + margin,
                       pageRect.width() - 2 * margin, pageRect.height() - 2 * margin);

    // Draw sheet border
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(Qt::white);
    painter.drawRect(outerBorder);

    // Title Block at bottom
    double titleBlockH = 90.0;
    QRectF titleBlockRect(outerBorder.left(), outerBorder.bottom() - titleBlockH, outerBorder.width(), titleBlockH);
    painter.setPen(QPen(Qt::black, 1.5));
    painter.setBrush(QColor(248, 250, 252));
    painter.drawRect(titleBlockRect);

    // Title Block Content
    painter.setPen(Qt::black);
    QFont f = painter.font();
    f.setFamily("Arial");
    f.setPointSize(14);
    f.setBold(true);
    painter.setFont(f);
    painter.drawText(titleBlockRect.adjusted(20, 10, -20, -50), Qt::AlignLeft | Qt::AlignVCenter,
                     QString::fromStdString(level.name).toUpper() + " PLAN");

    f.setPointSize(9);
    f.setBold(false);
    painter.setFont(f);
    QString subInfo = QString("SCALE 1:%1  |  ELEVATION: %2 mm  |  DATE: %3  |  SYSTEM: KaLara Arch v0.1.0")
        .arg(static_cast<int>(1.0 / scaleRatio(settings.scale)))
        .arg(static_cast<int>(level.elevation_mm))
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    painter.drawText(titleBlockRect.adjusted(20, 45, -20, -10), Qt::AlignLeft | Qt::AlignVCenter, subInfo);

    // Canvas rectangle above title block
    QRectF canvasRect(outerBorder.left() + 20, outerBorder.top() + 20,
                      outerBorder.width() - 40, outerBorder.height() - titleBlockH - 40);

    drawFloorPlanToPainter(painter, level, canvasRect, settings);

    painter.end();
    return true;
}

bool PdfExporter::exportProjectToPdf(const Project& project, const std::string& filePath, const ExportSettings& settings) {
    const auto* site = project.defaultSite();
    if (site && !site->buildings().empty() && site->defaultBuilding()->defaultLevel()) {
        return exportLevelToPdf(*site->defaultBuilding()->defaultLevel(), filePath, settings);
    }
    return false;
}

} // namespace kalara::exporters
