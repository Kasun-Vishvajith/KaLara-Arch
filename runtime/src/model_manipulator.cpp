#include "kalara/runtime/model_manipulator.hpp"
#include "kalara/core/geometry/ops.hpp"
#include <unordered_set>
#include <algorithm>
#include <limits>

namespace kalara::runtime {

size_t ModelManipulator::moveEntities(
    kalara::architecture::Level& level,
    const std::vector<kalara::architecture::EntityId>& entityIds,
    const kalara::core::geometry::Vector2D& delta_mm,
    bool propagateConnected
) {
    if (entityIds.empty() || delta_mm.lengthSquared() == 0.0) {
        return 0;
    }

    std::unordered_set<kalara::architecture::EntityId> targetSet(entityIds.begin(), entityIds.end());
    size_t movedCount = 0;

    // Collect wall vertices that are moving if connected propagation is needed
    std::vector<kalara::core::geometry::Point2D> movingPoints;
    if (propagateConnected) {
        for (const auto& id : targetSet) {
            if (auto* w = level.findWall(id)) {
                movingPoints.push_back(w->start);
                movingPoints.push_back(w->end);
            }
        }
    }

    // 1. Move Walls
    for (const auto& id : targetSet) {
        if (auto* wall = level.findWall(id)) {
            wall->start = wall->start + delta_mm;
            wall->end = wall->end + delta_mm;
            ++movedCount;
        }
    }

    // Propagate connected non-selected walls sharing coincident endpoints
    if (propagateConnected && !movingPoints.empty()) {
        const double snapTol = 50.0;
        for (const auto& wall : level.walls()) {
            if (targetSet.find(wall->id) == targetSet.end()) {
                // Not in primary selection: check if endpoints touch moving points
                for (const auto& oldPt : movingPoints) {
                    if (wall->start.distanceTo(oldPt) <= snapTol) {
                        wall->start = wall->start + delta_mm;
                    }
                    if (wall->end.distanceTo(oldPt) <= snapTol) {
                        wall->end = wall->end + delta_mm;
                    }
                }
            }
        }
    }

    // 2. Move Rooms
    for (const auto& id : targetSet) {
        if (auto* room = level.findRoom(id)) {
            for (auto& pt : room->boundary) {
                pt = pt + delta_mm;
            }
            ++movedCount;
        }
    }

    // 3. Move Free Dimensions (unreferenced)
    for (const auto& id : targetSet) {
        for (const auto& dim : level.dimensions()) {
            if (dim->id == id && !dim->referencedEntityId.has_value()) {
                dim->point1 = dim->point1 + delta_mm;
                dim->point2 = dim->point2 + delta_mm;
                ++movedCount;
                break;
            }
        }
    }

    // 4. Move Note Annotations
    for (const auto& id : targetSet) {
        for (const auto& note : level.notes()) {
            if (note->id == id) {
                note->position = note->position + delta_mm;
                ++movedCount;
                break;
            }
        }
    }

    // 5. Always synchronize associative dimensions (Rule 7)
    level.syncDimensions();

    return movedCount;
}

size_t ModelManipulator::rotateEntities(
    kalara::architecture::Level& level,
    const std::vector<kalara::architecture::EntityId>& entityIds,
    const kalara::core::geometry::Point2D& pivot_mm,
    kalara::core::geometry::Angle angle
) {
    if (entityIds.empty() || angle.degrees() == 0.0) {
        return 0;
    }

    std::unordered_set<kalara::architecture::EntityId> targetSet(entityIds.begin(), entityIds.end());
    size_t rotatedCount = 0;

    auto rot = kalara::core::geometry::Transform2D::rotationAround(angle, pivot_mm);

    // Rotate walls
    for (const auto& id : targetSet) {
        if (auto* wall = level.findWall(id)) {
            wall->start = rot.map(wall->start);
            wall->end = rot.map(wall->end);
            ++rotatedCount;
        }
    }

    // Rotate rooms
    for (const auto& id : targetSet) {
        if (auto* room = level.findRoom(id)) {
            for (auto& pt : room->boundary) {
                pt = rot.map(pt);
            }
            ++rotatedCount;
        }
    }

    // Rotate free dimensions
    for (const auto& id : targetSet) {
        for (const auto& dim : level.dimensions()) {
            if (dim->id == id && !dim->referencedEntityId.has_value()) {
                dim->point1 = rot.map(dim->point1);
                dim->point2 = rot.map(dim->point2);
                ++rotatedCount;
                break;
            }
        }
    }

    // Rotate note positions
    for (const auto& id : targetSet) {
        for (const auto& note : level.notes()) {
            if (note->id == id) {
                note->position = rot.map(note->position);
                ++rotatedCount;
                break;
            }
        }
    }

    // Synchronize associative dimensions
    level.syncDimensions();

    return rotatedCount;
}

size_t ModelManipulator::alignEntities(
    kalara::architecture::Level& level,
    const std::vector<kalara::architecture::EntityId>& entityIds,
    AlignmentType alignment
) {
    if (entityIds.size() < 2) {
        return 0;
    }

    // Step 1: Compute total bounding box of all selected entities
    double minX = std::numeric_limits<double>::infinity();
    double maxX = -std::numeric_limits<double>::infinity();
    double minY = std::numeric_limits<double>::infinity();
    double maxY = -std::numeric_limits<double>::infinity();

    for (const auto& id : entityIds) {
        if (auto* w = level.findWall(id)) {
            minX = std::min({minX, w->start.x, w->end.x});
            maxX = std::max({maxX, w->start.x, w->end.x});
            minY = std::min({minY, w->start.y, w->end.y});
            maxY = std::max({maxY, w->start.y, w->end.y});
        } else if (auto* r = level.findRoom(id)) {
            for (const auto& p : r->boundary) {
                minX = std::min(minX, p.x);
                maxX = std::max(maxX, p.x);
                minY = std::min(minY, p.y);
                maxY = std::max(maxY, p.y);
            }
        }
    }

    if (std::isinf(minX)) return 0;

    double targetRef = 0.0;
    switch (alignment) {
        case AlignmentType::AlignLeft:    targetRef = minX; break;
        case AlignmentType::AlignRight:   targetRef = maxX; break;
        case AlignmentType::AlignTop:     targetRef = maxY; break;
        case AlignmentType::AlignBottom:  targetRef = minY; break;
        case AlignmentType::AlignCenterX: targetRef = (minX + maxX) * 0.5; break;
        case AlignmentType::AlignCenterY: targetRef = (minY + maxY) * 0.5; break;
    }

    size_t alignedCount = 0;

    // Step 2: Translate each entity to align with target reference
    for (const auto& id : entityIds) {
        kalara::core::geometry::Vector2D delta{0.0, 0.0};

        if (auto* w = level.findWall(id)) {
            double eMinX = std::min(w->start.x, w->end.x);
            double eMaxX = std::max(w->start.x, w->end.x);
            double eMinY = std::min(w->start.y, w->end.y);
            double eMaxY = std::max(w->start.y, w->end.y);

            switch (alignment) {
                case AlignmentType::AlignLeft:    delta.dx = targetRef - eMinX; break;
                case AlignmentType::AlignRight:   delta.dx = targetRef - eMaxX; break;
                case AlignmentType::AlignTop:     delta.dy = targetRef - eMaxY; break;
                case AlignmentType::AlignBottom:  delta.dy = targetRef - eMinY; break;
                case AlignmentType::AlignCenterX: delta.dx = targetRef - (eMinX + eMaxX) * 0.5; break;
                case AlignmentType::AlignCenterY: delta.dy = targetRef - (eMinY + eMaxY) * 0.5; break;
            }

            if (delta.lengthSquared() > 0.0) {
                w->start = w->start + delta;
                w->end = w->end + delta;
                ++alignedCount;
            }
        } else if (auto* r = level.findRoom(id)) {
            double rMinX = std::numeric_limits<double>::infinity();
            double rMaxX = -std::numeric_limits<double>::infinity();
            double rMinY = std::numeric_limits<double>::infinity();
            double rMaxY = -std::numeric_limits<double>::infinity();

            for (const auto& p : r->boundary) {
                rMinX = std::min(rMinX, p.x);
                rMaxX = std::max(rMaxX, p.x);
                rMinY = std::min(rMinY, p.y);
                rMaxY = std::max(rMaxY, p.y);
            }

            switch (alignment) {
                case AlignmentType::AlignLeft:    delta.dx = targetRef - rMinX; break;
                case AlignmentType::AlignRight:   delta.dx = targetRef - rMaxX; break;
                case AlignmentType::AlignTop:     delta.dy = targetRef - rMaxY; break;
                case AlignmentType::AlignBottom:  delta.dy = targetRef - rMinY; break;
                case AlignmentType::AlignCenterX: delta.dx = targetRef - (rMinX + rMaxX) * 0.5; break;
                case AlignmentType::AlignCenterY: delta.dy = targetRef - (rMinY + rMaxY) * 0.5; break;
            }

            if (delta.lengthSquared() > 0.0) {
                for (auto& p : r->boundary) {
                    p = p + delta;
                }
                ++alignedCount;
            }
        }
    }

    level.syncDimensions();
    return alignedCount;
}

bool ModelManipulator::resizeWall(
    kalara::architecture::Level& level,
    const kalara::architecture::EntityId& wallId,
    double newLength_mm,
    bool anchorAtStart
) {
    if (newLength_mm <= 0.0) return false;

    auto* wall = level.findWall(wallId);
    if (!wall) return false;

    auto dir = wall->direction();
    if (anchorAtStart) {
        wall->end = wall->start + (dir * newLength_mm);
    } else {
        wall->start = wall->end - (dir * newLength_mm);
    }

    level.syncDimensions();
    return true;
}

} // namespace kalara::runtime
