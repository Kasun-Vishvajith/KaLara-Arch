#pragma once
#include "kalara/core/geometry/point_vector.hpp"
#include "kalara/core/geometry/segment.hpp"
#include "kalara/core/geometry/ops.hpp"
#include "kalara/architecture/level.hpp"
#include <optional>
#include <vector>

namespace kalara::runtime {

/// Types of semantic snap points (Rule 12).
enum class SnapType {
    None,
    Grid,
    Endpoint,
    Midpoint,
    WallCenterline,
    Alignment
};

/// Result of snapping calculation.
struct SnapResult {
    bool snapped = false;
    SnapType type = SnapType::None;
    kalara::core::geometry::Point2D point{0.0, 0.0};
    double distance_mm = 0.0;
    std::string description;
};

/// Semantic Snapping Engine (Rule 12).
/// Computes snapping targets against architectural entities (endpoints, midpoints, centerlines).
class SnappingEngine {
public:
    /// Snap a query position against walls, grid, and reference points on a level.
    static SnapResult snap(
        const kalara::core::geometry::Point2D& query_mm,
        const kalara::architecture::Level& level,
        double snapTolerance_mm = 150.0,
        bool enableGridSnap = false,
        double gridSpacing_mm = 1000.0
    ) {
        SnapResult bestResult;
        bestResult.point = query_mm;
        double minDistance = snapTolerance_mm;

        // 1. Highest priority: Wall Endpoints (wall-to-wall snapping)
        for (const auto& wall : level.walls()) {
            double dStart = query_mm.distanceTo(wall->start);
            if (dStart < minDistance) {
                minDistance = dStart;
                bestResult.snapped = true;
                bestResult.type = SnapType::Endpoint;
                bestResult.point = wall->start;
                bestResult.distance_mm = dStart;
                bestResult.description = "Wall Endpoint";
            }

            double dEnd = query_mm.distanceTo(wall->end);
            if (dEnd < minDistance) {
                minDistance = dEnd;
                bestResult.snapped = true;
                bestResult.type = SnapType::Endpoint;
                bestResult.point = wall->end;
                bestResult.distance_mm = dEnd;
                bestResult.description = "Wall Endpoint";
            }
        }

        // 2. Medium priority: Wall Midpoints
        for (const auto& wall : level.walls()) {
            auto mid = wall->centerline().midpoint();
            double dMid = query_mm.distanceTo(mid);
            if (dMid < minDistance) {
                minDistance = dMid;
                bestResult.snapped = true;
                bestResult.type = SnapType::Midpoint;
                bestResult.point = mid;
                bestResult.distance_mm = dMid;
                bestResult.description = "Wall Midpoint";
            }
        }

        // 3. Lower priority: Wall Centerline projection (sliding along wall)
        if (!bestResult.snapped) {
            for (const auto& wall : level.walls()) {
                auto seg = wall->centerline();
                auto proj = seg.closestPoint(query_mm);
                double dProj = query_mm.distanceTo(proj);
                if (dProj < snapTolerance_mm && dProj < minDistance) {
                    minDistance = dProj;
                    bestResult.snapped = true;
                    bestResult.type = SnapType::WallCenterline;
                    bestResult.point = proj;
                    bestResult.distance_mm = dProj;
                    bestResult.description = "Wall Centerline";
                }
            }
        }

        // 4. Fallback: Grid snap
        if (!bestResult.snapped && enableGridSnap && gridSpacing_mm > 0.0) {
            auto gridPt = kalara::core::geometry::GeometricOps::snapToGrid(query_mm, gridSpacing_mm);
            double dGrid = query_mm.distanceTo(gridPt);
            if (dGrid < snapTolerance_mm) {
                bestResult.snapped = true;
                bestResult.type = SnapType::Grid;
                bestResult.point = gridPt;
                bestResult.distance_mm = dGrid;
                bestResult.description = "Grid";
            }
        }

        return bestResult;
    }
};

} // namespace kalara::runtime
