#include "kalara/architecture/constraint_solver.hpp"
#include "kalara/core/geometry/tolerances.hpp"
#include "kalara/core/geometry/ops.hpp"
#include <cmath>

namespace kalara::architecture {

std::vector<ConstraintConflict> ConstraintSolver::validateConstraints(
    const std::vector<std::unique_ptr<Constraint>>& constraints,
    const Level& level,
    const Site* site
) {
    std::vector<ConstraintConflict> conflicts;

    for (const auto& cst : constraints) {
        switch (cst->type) {
            case ConstraintType::Coincidence: {
                if (cst->targetEntityIds.size() >= 2) {
                    auto* w1 = level.findWall(cst->targetEntityIds[0]);
                    auto* w2 = level.findWall(cst->targetEntityIds[1]);
                    if (w1 && w2) {
                        double minDistance = std::min({
                            w1->start.distanceTo(w2->start),
                            w1->start.distanceTo(w2->end),
                            w1->end.distanceTo(w2->start),
                            w1->end.distanceTo(w2->end)
                        });
                        if (minDistance > kalara::core::geometry::Tolerances::NominalPrecision_mm) {
                            conflicts.push_back({
                                cst->id,
                                cst->severity,
                                cst->type,
                                "Endpoint coincidence violation between walls " + w1->id.string() + " and " + w2->id.string(),
                                minDistance
                            });
                        }
                    }
                }
                break;
            }
            case ConstraintType::Perpendicularity: {
                if (cst->targetEntityIds.size() >= 2) {
                    auto* w1 = level.findWall(cst->targetEntityIds[0]);
                    auto* w2 = level.findWall(cst->targetEntityIds[1]);
                    if (w1 && w2) {
                        double dot = std::abs(w1->direction().dot(w2->direction()));
                        // Dot product must be ~0 for 90 degrees
                        if (dot > kalara::core::geometry::Tolerances::AngularEpsilon_rad * 100.0) {
                            conflicts.push_back({
                                cst->id,
                                cst->severity,
                                cst->type,
                                "Perpendicularity violation between walls " + w1->id.string() + " and " + w2->id.string(),
                                dot
                            });
                        }
                    }
                }
                break;
            }
            case ConstraintType::MinimumDistance: { // Clearance / Setback
                if (cst->targetEntityIds.size() >= 2) {
                    auto* w1 = level.findWall(cst->targetEntityIds[0]);
                    auto* w2 = level.findWall(cst->targetEntityIds[1]);
                    if (w1 && w2) {
                        double dist = w1->centerline().closestPoint(w2->start).distanceTo(w2->start);
                        if (dist < cst->targetValue) {
                            conflicts.push_back({
                                cst->id,
                                cst->severity,
                                cst->type,
                                "Clearance violation: distance " + std::to_string(dist) + " mm < required " + std::to_string(cst->targetValue) + " mm",
                                cst->targetValue - dist
                            });
                        }
                    }
                }
                break;
            }
            case ConstraintType::Containment: {
                // e.g. wall must remain inside site boundary
                if (site && !cst->targetEntityIds.empty()) {
                    auto* w = level.findWall(cst->targetEntityIds[0]);
                    if (w && site->propertyBoundary.size() >= 3) {
                        bool insideStart = kalara::core::geometry::GeometricOps::pointInPolygon(w->start, site->propertyBoundary);
                        bool insideEnd = kalara::core::geometry::GeometricOps::pointInPolygon(w->end, site->propertyBoundary);
                        if (!insideStart || !insideEnd) {
                            conflicts.push_back({
                                cst->id,
                                cst->severity,
                                cst->type,
                                "Containment violation: Wall " + w->id.string() + " exceeds site property boundary",
                                1.0
                            });
                        }
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    // Append opening host relationship checks
    auto openingConflicts = validateOpenings(level);
    conflicts.insert(conflicts.end(), openingConflicts.begin(), openingConflicts.end());

    return conflicts;
}

size_t ConstraintSolver::moveWallEndpoint(
    Wall& targetWall,
    bool isStartPoint,
    const kalara::core::geometry::Point2D& newPos,
    Level& level,
    ConnectedGeometryPropagation propagation,
    double snapTolerance_mm
) {
    auto oldPos = isStartPoint ? targetWall.start : targetWall.end;

    if (isStartPoint) {
        targetWall.start = newPos;
    } else {
        targetWall.end = newPos;
    }

    size_t propagatedCount = 1;

    if (propagation == ConnectedGeometryPropagation::PropagateConnected) {
        for (const auto& w : level.walls()) {
            if (w->id == targetWall.id) continue;

            if (w->start.coincidesWith(oldPos, snapTolerance_mm)) {
                w->start = newPos;
                ++propagatedCount;
            } else if (w->end.coincidesWith(oldPos, snapTolerance_mm)) {
                w->end = newPos;
                ++propagatedCount;
            }
        }
    }

    // Re-sync dimensions associated with modified walls
    level.syncDimensions();

    return propagatedCount;
}

std::vector<ConstraintConflict> ConstraintSolver::validateOpenings(const Level& level) {
    std::vector<ConstraintConflict> conflicts;

    // Validate doors
    for (const auto& door : level.doors()) {
        auto* host = level.findWall(door->hostWallId);
        if (!host) {
            conflicts.push_back({
                door->id,
                ConstraintSeverity::Hard,
                ConstraintType::HostAttachment,
                "Door " + door->id.string() + " references missing host wall",
                1.0
            });
            continue;
        }

        double halfW = door->width_mm * 0.5;
        double wallLen = host->length_mm();
        if (door->offsetAlongWall_mm - halfW < 0.0 || door->offsetAlongWall_mm + halfW > wallLen) {
            conflicts.push_back({
                door->id,
                ConstraintSeverity::Hard,
                ConstraintType::HostAttachment,
                "Door " + door->id.string() + " extends beyond host wall boundaries",
                std::max(halfW - door->offsetAlongWall_mm, door->offsetAlongWall_mm + halfW - wallLen)
            });
        }
    }

    // Validate windows
    for (const auto& win : level.windows()) {
        auto* host = level.findWall(win->hostWallId);
        if (!host) {
            conflicts.push_back({
                win->id,
                ConstraintSeverity::Hard,
                ConstraintType::HostAttachment,
                "Window " + win->id.string() + " references missing host wall",
                1.0
            });
            continue;
        }

        double halfW = win->width_mm * 0.5;
        double wallLen = host->length_mm();
        if (win->offsetAlongWall_mm - halfW < 0.0 || win->offsetAlongWall_mm + halfW > wallLen) {
            conflicts.push_back({
                win->id,
                ConstraintSeverity::Hard,
                ConstraintType::HostAttachment,
                "Window " + win->id.string() + " extends beyond host wall boundaries",
                std::max(halfW - win->offsetAlongWall_mm, win->offsetAlongWall_mm + halfW - wallLen)
            });
        }
    }

    return conflicts;
}

} // namespace kalara::architecture
