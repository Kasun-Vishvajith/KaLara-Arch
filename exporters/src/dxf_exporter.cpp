#include "kalara/exporters/dxf_exporter.hpp"
#include "kalara/exporters/dxf_writer.hpp"
#include "kalara/architecture/wall.hpp"
#include "kalara/architecture/opening.hpp"
#include "kalara/architecture/room.hpp"
#include "kalara/architecture/annotation.hpp"
#include "kalara/architecture/roof.hpp"
#include "kalara/core/geometry/ops.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace kalara::exporters {

using namespace kalara::core::geometry;
using namespace kalara::architecture;

static std::vector<DxfLayerDef> getStandardLayers() {
    return {
        {"0", DxfColor::White, "CONTINUOUS"},
        {"A-WALL", DxfColor::White, "CONTINUOUS"},
        {"A-WALL-CNTR", DxfColor::Gray, "CONTINUOUS"},
        {"A-DOOR", DxfColor::Yellow, "CONTINUOUS"},
        {"A-GLAZ", DxfColor::Cyan, "CONTINUOUS"},
        {"A-AREA", DxfColor::Magenta, "CONTINUOUS"},
        {"A-FLOR-FIXT", DxfColor::Gray, "CONTINUOUS"},
        {"A-FLOR-STRS", DxfColor::Red, "CONTINUOUS"},
        {"A-ROOF", DxfColor::Red, "CONTINUOUS"},
        {"A-DIMS", DxfColor::Green, "CONTINUOUS"},
        {"A-ANNO-TEXT", DxfColor::White, "CONTINUOUS"},
        {"C-PROP", DxfColor::Red, "CONTINUOUS"},
        {"C-SETB", DxfColor::Gray, "CONTINUOUS"},
        {"C-ROAD", DxfColor::LightGray, "CONTINUOUS"},
        {"C-SITE", DxfColor::Yellow, "CONTINUOUS"}
    };
}

static void exportLevelEntities(DxfWriter& writer, const Level& level, const DxfExportOptions& options) {
    // 1. Walls
    for (const auto& w : level.walls()) {
        if (options.exportWallCenterlines) {
            writer.writeLine("A-WALL-CNTR", w->start, w->end);
        }
        if (options.exportWallOutlines) {
            Vector2D dir = w->end - w->start;
            if (dir.lengthSquared() > 0.001) {
                Vector2D norm = dir.normalized().perpendicular() * (w->thickness_mm * 0.5);
                Point2D p1 = w->start + norm;
                Point2D p2 = w->end + norm;
                Point2D p3 = w->end - norm;
                Point2D p4 = w->start - norm;
                writer.writePolyline("A-WALL", {p1, p2, p3, p4}, true);
            }
        }
    }

    // 2. Openings (Doors and Windows)
    if (options.exportDoorsAndWindows) {
        for (const auto& d : level.doors()) {
            const auto* hostWall = level.findWall(d->hostWallId);
            if (!hostWall) continue;

            Vector2D dir = (hostWall->end - hostWall->start).normalized();
            Vector2D norm = dir.perpendicular();
            Point2D center = hostWall->start + dir * d->offsetAlongWall_mm;
            Point2D hinge = center - dir * (d->width_mm * 0.5);
            Point2D openTip = hinge + norm * d->width_mm;

            writer.writeLine("A-DOOR", hinge, openTip);
            double baseAngle = std::atan2(dir.dy, dir.dx) * (180.0 / 3.14159265358979323846);
            writer.writeArc("A-DOOR", hinge, d->width_mm, baseAngle, baseAngle + 90.0);
        }

        for (const auto& win : level.windows()) {
            const auto* hostWall = level.findWall(win->hostWallId);
            if (!hostWall) continue;

            Vector2D dir = (hostWall->end - hostWall->start).normalized();
            Vector2D norm = dir.perpendicular() * (hostWall->thickness_mm * 0.5);
            Point2D center = hostWall->start + dir * win->offsetAlongWall_mm;
            Point2D p1 = center - dir * (win->width_mm * 0.5) + norm;
            Point2D p2 = center + dir * (win->width_mm * 0.5) + norm;
            Point2D p3 = center + dir * (win->width_mm * 0.5) - norm;
            Point2D p4 = center - dir * (win->width_mm * 0.5) - norm;

            writer.writePolyline("A-GLAZ", {p1, p2, p3, p4}, true);
            writer.writeLine("A-GLAZ", center - dir * (win->width_mm * 0.5), center + dir * (win->width_mm * 0.5));
        }
    }

    // 3. Rooms & Areas
    if (options.exportRooms) {
        for (const auto& r : level.rooms()) {
            if (r->boundary.size() >= 3) {
                writer.writePolyline("A-AREA", r->boundary, true);
                Point2D centroid = GeometricOps::boundingBox(r->boundary).center();
                writer.writeText("A-AREA", centroid, 250.0, r->name);

                double area_m2 = GeometricOps::polygonArea(r->boundary) / 1000000.0;
                std::ostringstream ss;
                ss << std::fixed << std::setprecision(1) << area_m2 << " m2";
                writer.writeText("A-AREA", centroid - Vector2D{0.0, 320.0}, 180.0, ss.str());
            }
        }
    }

    // 4. Dimensions
    if (options.exportDimensions) {
        for (const auto& dim : level.dimensions()) {
            auto dLine = dim->dimensionLine();
            Point2D textPos = Point2D{(dLine.first.x + dLine.second.x) * 0.5, (dLine.first.y + dLine.second.y) * 0.5};
            writer.writeDimension("A-DIMS", dim->point1, dim->point2, textPos, dim->formattedText());
        }
    }

    // 5. Annotations & Notes
    if (options.exportAnnotations) {
        for (const auto& note : level.notes()) {
            writer.writeText("A-ANNO-TEXT", note->position, 200.0, note->text);
        }
    }

    // 6. Furniture & Fixtures
    if (options.exportFurniture) {
        for (const auto& inst : level.libraryInstances()) {
            double halfW = inst->width_mm * 0.5;
            double halfH = inst->length_mm * 0.5;
            double cosA = std::cos(inst->rotation.radians());
            double sinA = std::sin(inst->rotation.radians());
            auto rotVec = [cosA, sinA](double vx, double vy) {
                return Vector2D(vx * cosA - vy * sinA, vx * sinA + vy * cosA);
            };

            std::vector<Point2D> corners = {
                inst->position + rotVec(-halfW, -halfH),
                inst->position + rotVec(halfW, -halfH),
                inst->position + rotVec(halfW, halfH),
                inst->position + rotVec(-halfW, halfH)
            };
            writer.writePolyline("A-FLOR-FIXT", corners, true);
            writer.writeText("A-FLOR-FIXT", inst->position, 120.0, inst->name);
        }
    }

    // 7. Roofs
    if (options.exportRoofs) {
        for (const auto& rf : level.roofs()) {
            const auto& boundary = !rf->eaveBoundary.empty() ? rf->eaveBoundary : rf->wallFootprint;
            if (boundary.size() >= 3) {
                writer.writePolyline("A-ROOF", boundary, true);
            }
            for (const auto& ridge : rf->ridgeLines) {
                writer.writeLine("A-ROOF", ridge.start, ridge.end);
            }
        }
    }
}

static void exportSiteEntities(DxfWriter& writer, const Site& site, const DxfExportOptions& options) {
    if (!options.exportSite) return;

    if (site.propertyBoundary.size() >= 3) {
        writer.writePolyline("C-PROP", site.propertyBoundary, true);
    }

    auto envelope = site.buildableEnvelope();
    if (envelope.size() >= 3) {
        writer.writePolyline("C-SETB", envelope, true);
    }

    for (const auto& road : site.roads()) {
        writer.writeLine("C-ROAD", road->centerline.start, road->centerline.end);
    }

    for (const auto& ent : site.entrances()) {
        writer.writeCircle("C-SITE", ent->position, ent->width_mm * 0.5);
        writer.writeText("C-SITE", ent->position, 150.0, ent->name);
    }

    for (const auto& pool : site.outdoorElements()) {
        if (pool->boundary.size() >= 3) {
            writer.writePolyline("C-SITE", pool->boundary, true);
            writer.writeText("C-SITE", pool->boundary.front(), 180.0, pool->name);
        }
    }

    for (const auto& sp : site.spotElevations) {
        std::string lbl = sp.label.empty() ? ("+" + std::to_string(static_cast<int>(sp.elevation_mm))) : sp.label;
        writer.writeText("A-ANNO-TEXT", sp.position, 150.0, lbl);
    }
}

std::string DxfExporter::exportProjectToDxf(const Project& project, const DxfExportOptions& options) {
    DxfWriter writer;
    writer.writeHeader();
    writer.writeTables(getStandardLayers());
    writer.writeBlocks();
    writer.beginEntities();

    // Export site context
    const auto* site = project.defaultSite();
    if (site) {
        exportSiteEntities(writer, *site, options);

        // Export buildings and levels
        for (const auto& bld : site->buildings()) {
            for (const auto& lvl : bld->levels()) {
                if (options.targetLevelId.has_value() && lvl->id != options.targetLevelId.value()) {
                    continue;
                }
                exportLevelEntities(writer, *lvl, options);
            }
        }
    }

    writer.endEntities();
    writer.writeEof();
    return writer.str();
}

bool DxfExporter::exportProjectToFile(const Project& project,
                                      const std::string& filePath,
                                      const DxfExportOptions& options) {
    std::string content = exportProjectToDxf(project, options);
    std::ofstream out(filePath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }
    out << content;
    return out.good();
}

std::string DxfExporter::exportLevelToDxf(const Level& level, const DxfExportOptions& options) {
    DxfWriter writer;
    writer.writeHeader();
    writer.writeTables(getStandardLayers());
    writer.writeBlocks();
    writer.beginEntities();

    exportLevelEntities(writer, level, options);

    writer.endEntities();
    writer.writeEof();
    return writer.str();
}

std::string DxfExporter::exportSiteToDxf(const Site& site, const DxfExportOptions& options) {
    DxfWriter writer;
    writer.writeHeader();
    writer.writeTables(getStandardLayers());
    writer.writeBlocks();
    writer.beginEntities();

    exportSiteEntities(writer, site, options);

    writer.endEntities();
    writer.writeEof();
    return writer.str();
}

} // namespace kalara::exporters
