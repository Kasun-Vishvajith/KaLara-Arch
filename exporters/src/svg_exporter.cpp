#include "kalara/exporters/svg_exporter.hpp"
#include "kalara/architecture/wall.hpp"
#include "kalara/architecture/opening.hpp"
#include "kalara/architecture/room.hpp"
#include "kalara/architecture/annotation.hpp"
#include "kalara/architecture/roof.hpp"
#include "kalara/core/geometry/ops.hpp"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cmath>

namespace kalara::exporters {

using namespace kalara::core::geometry;
using namespace kalara::architecture;

static std::string generateSvgStyle(ExportTheme theme) {
    if (theme == ExportTheme::Blueprint) {
        return R"(
            .bg { fill: #0b2545; }
            .wall { fill: #134074; stroke: #8da9c4; stroke-width: 25; }
            .room { fill: #13315c; fill-opacity: 0.5; stroke: #8da9c4; stroke-width: 10; stroke-dasharray: 40,20; }
            .door-leaf { stroke: #eef4f8; stroke-width: 20; }
            .door-arc { stroke: #8da9c4; stroke-width: 12; fill: none; stroke-dasharray: 25,15; }
            .window { fill: #134074; stroke: #eef4f8; stroke-width: 18; }
            .dimension { stroke: #eef4f8; stroke-width: 12; fill: none; }
            .dim-text { font-family: 'Segoe UI', Arial, sans-serif; font-size: 150px; fill: #eef4f8; text-anchor: middle; font-weight: bold; }
            .room-title { font-family: 'Segoe UI', Arial, sans-serif; font-size: 220px; fill: #eef4f8; font-weight: bold; text-anchor: middle; }
            .room-area { font-family: 'Segoe UI', Arial, sans-serif; font-size: 160px; fill: #8da9c4; text-anchor: middle; }
            .anno-text { font-family: 'Segoe UI', Arial, sans-serif; font-size: 160px; fill: #eef4f8; }
            .prop-boundary { stroke: #e74c3c; stroke-width: 35; stroke-dasharray: 80,40; fill: none; }
            .setback { stroke: #f39c12; stroke-width: 18; stroke-dasharray: 40,20; fill: none; }
            .road { stroke: #bdc3c7; stroke-width: 15; fill: none; }
            .pool { fill: #1d4e89; stroke: #8da9c4; stroke-width: 20; }
        )";
    } else if (theme == ExportTheme::ArchitecturalMonochrome) {
        return R"(
            .bg { fill: #ffffff; }
            .wall { fill: #000000; stroke: #000000; stroke-width: 20; }
            .room { fill: none; stroke: #555555; stroke-width: 10; stroke-dasharray: 30,15; }
            .door-leaf { stroke: #000000; stroke-width: 18; }
            .door-arc { stroke: #000000; stroke-width: 10; fill: none; stroke-dasharray: 20,10; }
            .window { fill: #ffffff; stroke: #000000; stroke-width: 15; }
            .dimension { stroke: #000000; stroke-width: 10; fill: none; }
            .dim-text { font-family: 'Segoe UI', Arial, sans-serif; font-size: 150px; fill: #000000; text-anchor: middle; }
            .room-title { font-family: 'Segoe UI', Arial, sans-serif; font-size: 220px; fill: #000000; font-weight: bold; text-anchor: middle; }
            .room-area { font-family: 'Segoe UI', Arial, sans-serif; font-size: 160px; fill: #444444; text-anchor: middle; }
            .anno-text { font-family: 'Segoe UI', Arial, sans-serif; font-size: 160px; fill: #000000; }
            .prop-boundary { stroke: #000000; stroke-width: 30; stroke-dasharray: 60,30; fill: none; }
            .setback { stroke: #666666; stroke-width: 15; stroke-dasharray: 30,15; fill: none; }
            .road { stroke: #888888; stroke-width: 15; fill: none; }
            .pool { fill: #f0f0f0; stroke: #000000; stroke-width: 20; }
        )";
    }

    // Default: ColorPresentation
    return R"(
        .bg { fill: #fdfdfd; }
        .wall { fill: #2c3e50; stroke: #1a252f; stroke-width: 20; }
        .room { fill: #3498db; fill-opacity: 0.08; stroke: #7f8c8d; stroke-width: 10; stroke-dasharray: 30,15; }
        .door-leaf { stroke: #d35400; stroke-width: 20; }
        .door-arc { stroke: #e67e22; stroke-width: 12; fill: none; stroke-dasharray: 25,15; }
        .window { fill: #ecf0f1; stroke: #2980b9; stroke-width: 18; }
        .dimension { stroke: #27ae60; stroke-width: 12; fill: none; }
        .dim-text { font-family: 'Segoe UI', Arial, sans-serif; font-size: 150px; fill: #27ae60; text-anchor: middle; font-weight: bold; }
        .room-title { font-family: 'Segoe UI', Arial, sans-serif; font-size: 220px; fill: #2c3e50; font-weight: bold; text-anchor: middle; }
        .room-area { font-family: 'Segoe UI', Arial, sans-serif; font-size: 160px; fill: #7f8c8d; text-anchor: middle; }
        .anno-text { font-family: 'Segoe UI', Arial, sans-serif; font-size: 160px; fill: #34495e; }
        .prop-boundary { stroke: #c0392b; stroke-width: 35; stroke-dasharray: 80,40; fill: none; }
        .setback { stroke: #e67e22; stroke-width: 18; stroke-dasharray: 40,20; fill: none; }
        .road { stroke: #95a5a6; stroke-width: 20; fill: none; }
        .pool { fill: #3498db; fill-opacity: 0.25; stroke: #2980b9; stroke-width: 25; }
    )";
}

static void collectLevelBounds(const Level& level, std::vector<Point2D>& pts) {
    for (const auto& w : level.walls()) {
        pts.push_back(w->start);
        pts.push_back(w->end);
    }
    for (const auto& r : level.rooms()) {
        for (const auto& p : r->boundary) pts.push_back(p);
    }
}

static void collectSiteBounds(const Site& site, std::vector<Point2D>& pts) {
    for (const auto& p : site.propertyBoundary) pts.push_back(p);
    for (const auto& r : site.roads()) {
        pts.push_back(r->centerline.start);
        pts.push_back(r->centerline.end);
    }
}

std::string SvgExporter::exportLevelToSvg(const Level& level, const ExportSettings& settings) {
    std::vector<Point2D> pts;
    collectLevelBounds(level, pts);
    if (pts.empty()) {
        pts.push_back(Point2D(-2000.0, -2000.0));
        pts.push_back(Point2D(10000.0, 10000.0));
    }

    auto bbox = GeometricOps::boundingBox(pts);
    double margin = 1500.0;
    double minX = bbox.min.x - margin;
    double minY = bbox.min.y - margin;
    double maxX = bbox.max.x + margin;
    double maxY = bbox.max.y + margin;
    double width = maxX - minX;
    double height = maxY - minY;

    auto toSvg = [minX, maxY](const Point2D& p) -> Point2D {
        return Point2D(p.x - minX, maxY - p.y);
    };

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1);

    ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" ";
    ss << "width=\"" << width << "mm\" height=\"" << height << "mm\" ";
    ss << "viewBox=\"0 0 " << width << " " << height << "\">\n";

    ss << "<style>\n" << generateSvgStyle(settings.theme) << "</style>\n";
    ss << "<rect class=\"bg\" width=\"" << width << "\" height=\"" << height << "\"/>\n";

    // 1. Rooms
    if (settings.includeRoomTags) {
        for (const auto& r : level.rooms()) {
            if (r->boundary.size() >= 3) {
                ss << "<polygon class=\"room\" points=\"";
                for (const auto& p : r->boundary) {
                    auto sp = toSvg(p);
                    ss << sp.x << "," << sp.y << " ";
                }
                ss << "\"/>\n";

                Point2D centroid = toSvg(GeometricOps::boundingBox(r->boundary).center());
                ss << "<text class=\"room-title\" x=\"" << centroid.x << "\" y=\"" << centroid.y - 40.0 << "\">"
                   << r->name << "</text>\n";

                double area_m2 = GeometricOps::polygonArea(r->boundary) / 1000000.0;
                ss << "<text class=\"room-area\" x=\"" << centroid.x << "\" y=\"" << centroid.y + 160.0 << "\">"
                   << std::fixed << std::setprecision(1) << area_m2 << " m²</text>\n";
            }
        }
    }

    // 2. Walls
    for (const auto& w : level.walls()) {
        Vector2D dir = w->end - w->start;
        if (dir.lengthSquared() > 0.001) {
            Vector2D norm = dir.normalized().perpendicular() * (w->thickness_mm * 0.5);
            Point2D p1 = toSvg(w->start + norm);
            Point2D p2 = toSvg(w->end + norm);
            Point2D p3 = toSvg(w->end - norm);
            Point2D p4 = toSvg(w->start - norm);

            ss << "<polygon class=\"wall\" points=\""
               << p1.x << "," << p1.y << " "
               << p2.x << "," << p2.y << " "
               << p3.x << "," << p3.y << " "
               << p4.x << "," << p4.y << "\"/>\n";
        }
    }

    // 3. Openings (Doors and Windows)
    if (settings.includeDoorSwings) {
        for (const auto& d : level.doors()) {
            const auto* hostWall = level.findWall(d->hostWallId);
            if (!hostWall) continue;

            Vector2D dir = (hostWall->end - hostWall->start).normalized();
            Vector2D norm = dir.perpendicular();
            Point2D center = hostWall->start + dir * d->offsetAlongWall_mm;
            Point2D hinge = center - dir * (d->width_mm * 0.5);
            Point2D openTip = hinge + norm * d->width_mm;

            Point2D sHinge = toSvg(hinge);
            Point2D sOpenTip = toSvg(openTip);
            ss << "<line class=\"door-leaf\" x1=\"" << sHinge.x << "\" y1=\"" << sHinge.y
               << "\" x2=\"" << sOpenTip.x << "\" y2=\"" << sOpenTip.y << "\"/>\n";

            // Approximate swing arc as cubic bezier
            Point2D strike = center + dir * (d->width_mm * 0.5);
            Point2D sStrike = toSvg(strike);
            Point2D sControl = toSvg(hinge + (dir + norm) * (d->width_mm * 0.8));
            ss << "<path class=\"door-arc\" d=\"M " << sStrike.x << " " << sStrike.y
               << " Q " << sControl.x << " " << sControl.y << " " << sOpenTip.x << " " << sOpenTip.y << "\"/>\n";
        }

        for (const auto& win : level.windows()) {
            const auto* hostWall = level.findWall(win->hostWallId);
            if (!hostWall) continue;

            Vector2D dir = (hostWall->end - hostWall->start).normalized();
            Vector2D norm = dir.perpendicular() * (hostWall->thickness_mm * 0.5);
            Point2D center = hostWall->start + dir * win->offsetAlongWall_mm;
            Point2D p1 = toSvg(center - dir * (win->width_mm * 0.5) + norm);
            Point2D p2 = toSvg(center + dir * (win->width_mm * 0.5) + norm);
            Point2D p3 = toSvg(center + dir * (win->width_mm * 0.5) - norm);
            Point2D p4 = toSvg(center - dir * (win->width_mm * 0.5) - norm);

            ss << "<polygon class=\"window\" points=\""
               << p1.x << "," << p1.y << " "
               << p2.x << "," << p2.y << " "
               << p3.x << "," << p3.y << " "
               << p4.x << "," << p4.y << "\"/>\n";
        }
    }

    // 4. Dimensions
    if (settings.includeDimensions) {
        for (const auto& dim : level.dimensions()) {
            auto dLine = dim->dimensionLine();
            Point2D sd1 = toSvg(dLine.first);
            Point2D sd2 = toSvg(dLine.second);
            ss << "<line class=\"dimension\" x1=\"" << sd1.x << "\" y1=\"" << sd1.y
               << "\" x2=\"" << sd2.x << "\" y2=\"" << sd2.y << "\"/>\n";

            Point2D mid = Point2D{(sd1.x + sd2.x) * 0.5, (sd1.y + sd2.y) * 0.5};
            ss << "<text class=\"dim-text\" x=\"" << mid.x << "\" y=\"" << mid.y - 40.0 << "\">"
               << dim->formattedText() << "</text>\n";
        }
    }

    // 5. Annotations & Notes
    for (const auto& note : level.notes()) {
        Point2D sp = toSvg(note->position);
        ss << "<text class=\"anno-text\" x=\"" << sp.x << "\" y=\"" << sp.y << "\">"
           << note->text << "</text>\n";
    }

    // 6. Title badge
    if (settings.includeTitleBlock) {
        ss << "<g transform=\"translate(80, " << height - 80.0 << ")\">\n";
        ss << "  <text class=\"room-title\" x=\"0\" y=\"-100\" text-anchor=\"start\">" << level.name << "</text>\n";
        ss << "  <text class=\"room-area\" x=\"0\" y=\"0\" text-anchor=\"start\">Scale 1:"
           << static_cast<int>(1.0 / scaleRatio(settings.scale)) << " | Level Elevation: "
           << static_cast<int>(level.elevation_mm) << " mm</text>\n";
        ss << "</g>\n";
    }

    ss << "</svg>\n";
    return ss.str();
}

std::string SvgExporter::exportSiteToSvg(const Site& site, const ExportSettings& settings) {
    std::vector<Point2D> pts;
    collectSiteBounds(site, pts);
    for (const auto& bld : site.buildings()) {
        for (const auto& lvl : bld->levels()) {
            collectLevelBounds(*lvl, pts);
        }
    }
    if (pts.empty()) {
        pts.push_back(Point2D(-10000.0, -10000.0));
        pts.push_back(Point2D(40000.0, 40000.0));
    }

    auto bbox = GeometricOps::boundingBox(pts);
    double margin = 3000.0;
    double minX = bbox.min.x - margin;
    double minY = bbox.min.y - margin;
    double maxX = bbox.max.x + margin;
    double maxY = bbox.max.y + margin;
    double width = maxX - minX;
    double height = maxY - minY;

    auto toSvg = [minX, maxY](const Point2D& p) -> Point2D {
        return Point2D(p.x - minX, maxY - p.y);
    };

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1);

    ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" ";
    ss << "width=\"" << width << "mm\" height=\"" << height << "mm\" ";
    ss << "viewBox=\"0 0 " << width << " " << height << "\">\n";
    ss << "<style>\n" << generateSvgStyle(settings.theme) << "</style>\n";
    ss << "<rect class=\"bg\" width=\"" << width << "\" height=\"" << height << "\"/>\n";

    // Property boundary
    if (site.propertyBoundary.size() >= 3) {
        ss << "<polygon class=\"prop-boundary\" points=\"";
        for (const auto& p : site.propertyBoundary) {
            auto sp = toSvg(p);
            ss << sp.x << "," << sp.y << " ";
        }
        ss << "\"/>\n";
    }

    // Setback envelope
    auto envelope = site.buildableEnvelope();
    if (envelope.size() >= 3) {
        ss << "<polygon class=\"setback\" points=\"";
        for (const auto& p : envelope) {
            auto sp = toSvg(p);
            ss << sp.x << "," << sp.y << " ";
        }
        ss << "\"/>\n";
    }

    // Roads
    for (const auto& road : site.roads()) {
        auto s1 = toSvg(road->centerline.start);
        auto s2 = toSvg(road->centerline.end);
        ss << "<line class=\"road\" x1=\"" << s1.x << "\" y1=\"" << s1.y
           << "\" x2=\"" << s2.x << "\" y2=\"" << s2.y << "\"/>\n";
    }

    // Pools & outdoor elements
    for (const auto& pool : site.outdoorElements()) {
        if (pool->boundary.size() >= 3) {
            ss << "<polygon class=\"pool\" points=\"";
            for (const auto& p : pool->boundary) {
                auto sp = toSvg(p);
                ss << sp.x << "," << sp.y << " ";
            }
            ss << "\"/>\n";
        }
    }

    // Title
    if (settings.includeTitleBlock) {
        ss << "<g transform=\"translate(120, " << height - 120.0 << ")\">\n";
        ss << "  <text class=\"room-title\" x=\"0\" y=\"-100\" text-anchor=\"start\">" << site.name << " — Site Plan</text>\n";
        ss << "  <text class=\"room-area\" x=\"0\" y=\"0\" text-anchor=\"start\">Property Area: "
           << std::fixed << std::setprecision(1) << site.propertyArea_m2() << " m²</text>\n";
        ss << "</g>\n";
    }

    ss << "</svg>\n";
    return ss.str();
}

std::string SvgExporter::exportProjectToSvg(const Project& project, const ExportSettings& settings) {
    const auto* site = project.defaultSite();
    if (site && !site->buildings().empty() && site->defaultBuilding()->defaultLevel()) {
        return exportLevelToSvg(*site->defaultBuilding()->defaultLevel(), settings);
    }
    if (site) {
        return exportSiteToSvg(*site, settings);
    }
    return "";
}

bool SvgExporter::exportLevelToFile(const Level& level, const std::string& filePath, const ExportSettings& settings) {
    std::ofstream out(filePath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) return false;
    out << exportLevelToSvg(level, settings);
    return out.good();
}

bool SvgExporter::exportProjectToFile(const Project& project, const std::string& filePath, const ExportSettings& settings) {
    std::ofstream out(filePath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) return false;
    out << exportProjectToSvg(project, settings);
    return out.good();
}

} // namespace kalara::exporters
