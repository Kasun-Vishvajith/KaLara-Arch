#include "kalara/runtime/validation_engine.hpp"
#include "kalara/core/geometry/ops.hpp"
#include "kalara/core/geometry/tolerances.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace kalara::runtime {

using namespace kalara::architecture;
using namespace kalara::core::geometry;

ValidationReport ValidationEngine::validateProject(const Project& project) {
    ValidationReport report;
    for (const auto& site : project.sites()) {
        auto siteReport = validateSite(*site);
        for (auto& issue : siteReport.issues) {
            report.addIssue(std::move(issue));
        }
    }
    return report;
}

ValidationReport ValidationEngine::validateSite(const Site& site) {
    ValidationReport report;

    // 1. Property Boundary Checks
    if (site.propertyBoundary.size() < 3) {
        report.addIssue({
            .code = "SITE_NO_BOUNDARY",
            .severity = ValidationSeverity::Warning,
            .category = ValidationCategory::SiteAndSetback,
            .message = "Property boundary is not defined or has fewer than 3 boundary vertices.",
            .suggestedRemedy = "Define a closed property boundary polygon for the site.",
            .entityIds = {site.id}
        });
    } else {
        double pArea = site.propertyArea_mm2();
        if (pArea <= 0.0) {
            report.addIssue({
                .code = "SITE_INVALID_AREA",
                .severity = ValidationSeverity::Error,
                .category = ValidationCategory::SiteAndSetback,
                .message = "Property boundary has zero or negative enclosed area.",
                .suggestedRemedy = "Ensure boundary vertices are ordered counter-clockwise and do not self-intersect.",
                .entityIds = {site.id}
            });
        }

        // Setback envelope check
        auto envelope = site.buildableEnvelope();
        if (envelope.empty()) {
            report.addIssue({
                .code = "SITE_NO_BUILDABLE_ENVELOPE",
                .severity = ValidationSeverity::Warning,
                .category = ValidationCategory::SiteAndSetback,
                .message = "Configured municipal setbacks exceed property dimensions; no buildable envelope remains.",
                .suggestedRemedy = "Reduce front, rear, or side setbacks.",
                .entityIds = {site.id}
            });
        }
    }

    // 2. Validate contained buildings
    for (const auto& bld : site.buildings()) {
        auto bldReport = validateBuilding(*bld, &site);
        for (auto& issue : bldReport.issues) {
            report.addIssue(std::move(issue));
        }
    }

    return report;
}

ValidationReport ValidationEngine::validateBuilding(const Building& building, const Site* site) {
    ValidationReport report;

    if (building.levels().empty()) {
        report.addIssue({
            .code = "BLD_NO_LEVELS",
            .severity = ValidationSeverity::Warning,
            .category = ValidationCategory::Geometry,
            .message = "Building '" + building.name + "' has no levels defined.",
            .suggestedRemedy = "Add at least one architectural level / floor.",
            .entityIds = {building.id}
        });
        return report;
    }

    // Validate each level
    for (const auto& lvl : building.levels()) {
        auto lvlReport = validateLevel(*lvl);
        for (auto& issue : lvlReport.issues) {
            report.addIssue(std::move(issue));
        }
    }

    // Validate site and setbacks if site is available
    if (site) {
        validateSiteAndSetbacks(*site, building, report);
    }

    return report;
}

ValidationReport ValidationEngine::validateLevel(const Level& level) {
    ValidationReport report;
    validateGeometry(level, report);
    validateConnections(level, report);
    validateRooms(level, report);
    validateClearances(level, report);
    validateRoofs(level, report);
    return report;
}

void ValidationEngine::validateGeometry(const Level& level, ValidationReport& report) {
    const auto& walls = level.walls();

    for (size_t i = 0; i < walls.size(); ++i) {
        const auto& w = walls[i];
        double len = w->length_mm();

        // 1. Degenerate zero-length walls
        if (len < 10.0) {
            report.addIssue({
                .code = "GEOM_WALL_ZERO_LENGTH",
                .severity = ValidationSeverity::Error,
                .category = ValidationCategory::Geometry,
                .message = "Wall has near-zero length (" + std::to_string(static_cast<int>(len)) + " mm < 10 mm).",
                .suggestedRemedy = "Delete the degenerate wall or extend its endpoints.",
                .entityIds = {w->id},
                .location = w->start
            });
        }

        // 2. Invalid thickness
        if (w->thickness_mm <= 0.0) {
            report.addIssue({
                .code = "GEOM_WALL_INVALID_THICKNESS",
                .severity = ValidationSeverity::Error,
                .category = ValidationCategory::Geometry,
                .message = "Wall thickness is zero or negative (" + std::to_string(static_cast<int>(w->thickness_mm)) + " mm).",
                .suggestedRemedy = "Set wall thickness to a standard architectural value (e.g. 150-250 mm).",
                .entityIds = {w->id},
                .location = w->start
            });
        }

        // 3. Duplicate overlapping walls
        for (size_t j = i + 1; j < walls.size(); ++j) {
            const auto& w2 = walls[j];
            bool sameEnds = (w->start.distanceTo(w2->start) < 1.0 && w->end.distanceTo(w2->end) < 1.0) ||
                            (w->start.distanceTo(w2->end) < 1.0 && w->end.distanceTo(w2->start) < 1.0);
            if (sameEnds) {
                report.addIssue({
                    .code = "GEOM_WALL_DUPLICATE",
                    .severity = ValidationSeverity::Warning,
                    .category = ValidationCategory::Geometry,
                    .message = "Duplicate collinear walls detected at identical position.",
                    .suggestedRemedy = "Remove the redundant overlapping wall.",
                    .entityIds = {w->id, w2->id},
                    .location = w->start
                });
            }
        }
    }
}

void ValidationEngine::validateConnections(const Level& level, ValidationReport& report) {
    // 1. Validate Doors
    for (const auto& d : level.doors()) {
        auto* host = level.findWall(d->hostWallId);
        if (!host) {
            report.addIssue({
                .code = "CONN_ORPHANED_DOOR",
                .severity = ValidationSeverity::Error,
                .category = ValidationCategory::Connection,
                .message = "Door references nonexistent host wall.",
                .suggestedRemedy = "Re-assign door to a valid wall on this level.",
                .entityIds = {d->id}
            });
            continue;
        }

        double wallLen = host->length_mm();
        if (d->offsetAlongWall_mm < 0.0 || (d->offsetAlongWall_mm + d->width_mm) > wallLen + 1.0) {
            report.addIssue({
                .code = "CONN_DOOR_OUT_OF_BOUNDS",
                .severity = ValidationSeverity::Error,
                .category = ValidationCategory::Connection,
                .message = "Door extends outside host wall bounds (wall length: " +
                           std::to_string(static_cast<int>(wallLen)) + " mm, door range: " +
                           std::to_string(static_cast<int>(d->offsetAlongWall_mm)) + " - " +
                           std::to_string(static_cast<int>(d->offsetAlongWall_mm + d->width_mm)) + " mm).",
                .suggestedRemedy = "Adjust door position along wall or reduce door width.",
                .entityIds = {d->id, host->id},
                .location = d->centerPoint(*host)
            });
        }
    }

    // 2. Validate Windows
    for (const auto& win : level.windows()) {
        auto* host = level.findWall(win->hostWallId);
        if (!host) {
            report.addIssue({
                .code = "CONN_ORPHANED_WINDOW",
                .severity = ValidationSeverity::Error,
                .category = ValidationCategory::Connection,
                .message = "Window references nonexistent host wall.",
                .suggestedRemedy = "Re-assign window to a valid wall on this level.",
                .entityIds = {win->id}
            });
            continue;
        }

        double wallLen = host->length_mm();
        if (win->offsetAlongWall_mm < 0.0 || (win->offsetAlongWall_mm + win->width_mm) > wallLen + 1.0) {
            report.addIssue({
                .code = "CONN_WINDOW_OUT_OF_BOUNDS",
                .severity = ValidationSeverity::Error,
                .category = ValidationCategory::Connection,
                .message = "Window extends outside host wall bounds (wall length: " +
                           std::to_string(static_cast<int>(wallLen)) + " mm, window range: " +
                           std::to_string(static_cast<int>(win->offsetAlongWall_mm)) + " - " +
                           std::to_string(static_cast<int>(win->offsetAlongWall_mm + win->width_mm)) + " mm).",
                .suggestedRemedy = "Adjust window position along wall or reduce window width.",
                .entityIds = {win->id, host->id},
                .location = win->centerPoint(*host)
            });
        }
    }

    // 3. Check for overlapping openings on the same host wall
    for (const auto& wall : level.walls()) {
        struct Interval { double start; double end; EntityId id; };
        std::vector<Interval> intervals;
        for (const auto& d : level.doors()) {
            if (d->hostWallId == wall->id) {
                intervals.push_back({d->offsetAlongWall_mm, d->offsetAlongWall_mm + d->width_mm, d->id});
            }
        }
        for (const auto& win : level.windows()) {
            if (win->hostWallId == wall->id) {
                intervals.push_back({win->offsetAlongWall_mm, win->offsetAlongWall_mm + win->width_mm, win->id});
            }
        }

        for (size_t i = 0; i < intervals.size(); ++i) {
            for (size_t j = i + 1; j < intervals.size(); ++j) {
                if (std::max(intervals[i].start, intervals[j].start) < std::min(intervals[i].end, intervals[j].end)) {
                    report.addIssue({
                        .code = "CONN_OPENINGS_OVERLAP",
                        .severity = ValidationSeverity::Warning,
                        .category = ValidationCategory::Connection,
                        .message = "Openings on the same wall overlap each other.",
                        .suggestedRemedy = "Separate the door and window positions along the wall.",
                        .entityIds = {intervals[i].id, intervals[j].id, wall->id},
                        .location = wall->centerline().midpoint()
                    });
                }
            }
        }
    }
}

void ValidationEngine::validateRooms(const Level& level, ValidationReport& report) {
    for (const auto& room : level.rooms()) {
        if (room->boundary.size() < 3) {
            report.addIssue({
                .code = "ROOM_DEGENERATE_BOUNDARY",
                .severity = ValidationSeverity::Error,
                .category = ValidationCategory::Room,
                .message = "Room '" + room->name + "' has fewer than 3 boundary vertices.",
                .suggestedRemedy = "Re-detect room boundary or specify a valid polygon.",
                .entityIds = {room->id}
            });
            continue;
        }

        double area = room->area_mm2();
        if (area <= 0.0) {
            report.addIssue({
                .code = "ROOM_ZERO_AREA",
                .severity = ValidationSeverity::Error,
                .category = ValidationCategory::Room,
                .message = "Room '" + room->name + "' has zero or negative enclosed area.",
                .suggestedRemedy = "Ensure room boundary vertices form a valid non-inverted polygon.",
                .entityIds = {room->id},
                .location = room->labelPosition()
            });
        }
    }
}

void ValidationEngine::validateClearances(const Level& level, ValidationReport& report) {
    // 1. Library Instance Clearances colliding with walls
    for (const auto& inst : level.libraryInstances()) {
        if (!inst->clearance.hasClearance()) continue;

        auto clBox = GeometricOps::boundingBox(inst->worldClearanceZone());
        for (const auto& w : level.walls()) {
            if (clBox.contains(w->start) || clBox.contains(w->end) || clBox.contains(w->centerline().midpoint())) {
                report.addIssue({
                    .code = "CLEARANCE_WALL_OBSTRUCTION",
                    .severity = ValidationSeverity::Warning,
                    .category = ValidationCategory::Clearance,
                    .message = "Clearance zone for '" + inst->name + "' is obstructed by an interior wall.",
                    .suggestedRemedy = "Provide adequate clearance in front of fixtures and furniture.",
                    .entityIds = {inst->id, w->id},
                    .location = inst->position
                });
                break;
            }
        }
    }

    // 2. Door swing collisions with host or adjacent perpendicular walls
    for (const auto& d : level.doors()) {
        auto* host = level.findWall(d->hostWallId);
        if (!host) continue;

        auto hinge = d->hingePoint(*host);
        double r = d->width_mm;
        auto swingBox = Rect2D::fromCenterAndSize(hinge, r * 2.0, r * 2.0);
        for (const auto& otherWall : level.walls()) {
            if (otherWall->id == host->id) continue;
            // Check if other wall cuts inside door swing box
            if (swingBox.contains(otherWall->start) || swingBox.contains(otherWall->end)) {
                report.addIssue({
                    .code = "CLEARANCE_DOOR_SWING_COLLISION",
                    .severity = ValidationSeverity::Warning,
                    .category = ValidationCategory::Clearance,
                    .message = "Door swing path intersects an adjacent wall.",
                    .suggestedRemedy = "Flip door swing direction or move door away from corner.",
                    .entityIds = {d->id, otherWall->id},
                    .location = d->centerPoint(*host)
                });
                break;
            }
        }
    }
}

void ValidationEngine::validateRoofs(const Level& level, ValidationReport& report) {
    for (const auto& roof : level.roofs()) {
        if (roof->eaveBoundary.size() < 3) {
            report.addIssue({
                .code = "ROOF_DEGENERATE_OUTLINE",
                .severity = ValidationSeverity::Error,
                .category = ValidationCategory::Geometry,
                .message = "Roof '" + roof->name + "' has fewer than 3 boundary vertices.",
                .suggestedRemedy = "Specify a valid closed footprint for the roof.",
                .entityIds = {roof->id}
            });
        }

        if (roof->pitch_deg < 0.0 || roof->pitch_deg > 85.0) {
            report.addIssue({
                .code = "ROOF_EXTREME_PITCH",
                .severity = ValidationSeverity::Warning,
                .category = ValidationCategory::Geometry,
                .message = "Roof pitch (" + std::to_string(static_cast<int>(roof->pitch_deg)) + "°) is outside standard architectural ranges (0° - 85°).",
                .suggestedRemedy = "Check roof pitch configuration.",
                .entityIds = {roof->id}
            });
        }
    }
}

void ValidationEngine::validateSiteAndSetbacks(const Site& site, const Building& building, ValidationReport& report) {
    if (site.propertyBoundary.size() < 3) return;

    auto envelope = site.buildableEnvelope();

    for (const auto& lvl : building.levels()) {
        for (const auto& w : lvl->walls()) {
            // Check if wall points are outside property boundary
            if (!site.containsPoint(w->start) || !site.containsPoint(w->end)) {
                report.addIssue({
                    .code = "SITE_OUTSIDE_PROPERTY",
                    .severity = ValidationSeverity::Error,
                    .category = ValidationCategory::SiteAndSetback,
                    .message = "Building wall extends outside property boundaries.",
                    .suggestedRemedy = "Reposition building inside property boundary lines.",
                    .entityIds = {w->id, building.id, site.id},
                    .location = w->start
                });
            } else if (!envelope.empty()) {
                // Check if wall points fall outside buildable envelope (i.e. into setback buffers)
                bool startInEnv = GeometricOps::pointInPolygon(w->start, envelope);
                bool endInEnv = GeometricOps::pointInPolygon(w->end, envelope);
                if (!startInEnv || !endInEnv) {
                    report.addIssue({
                        .code = "SITE_SETBACK_ENCROACHMENT",
                        .severity = ValidationSeverity::Warning,
                        .category = ValidationCategory::SiteAndSetback,
                        .message = "Building wall encroaches into municipal setback buffer.",
                        .suggestedRemedy = "Adjust building placement or apply for setback variance.",
                        .entityIds = {w->id, building.id, site.id},
                        .location = w->start
                    });
                }
            }
        }
    }
}

} // namespace kalara::runtime
