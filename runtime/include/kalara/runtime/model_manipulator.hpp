#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/level.hpp"
#include "kalara/architecture/site.hpp"
#include "kalara/core/geometry/point_vector.hpp"
#include "kalara/core/geometry/transform.hpp"
#include "kalara/core/geometry/angle.hpp"
#include <vector>

namespace kalara::runtime {

/// Alignment reference for multi-entity alignment.
enum class AlignmentType {
    AlignLeft,
    AlignRight,
    AlignTop,
    AlignBottom,
    AlignCenterX,
    AlignCenterY
};

/// Controlled Architectural Model Mutation Service (Principle 6 & Rule 3: One Mutation Choke Point).
/// Provides structured translation, rotation, scaling/resizing, and alignment operations.
class ModelManipulator {
public:
    /// Move a set of entities on a level by world delta vector in millimetres.
    /// Automatically syncs associated dimensions and connected geometry if requested.
    static size_t moveEntities(
        kalara::architecture::Level& level,
        const std::vector<kalara::architecture::EntityId>& entityIds,
        const kalara::core::geometry::Vector2D& delta_mm,
        bool propagateConnected = false
    );

    /// Rotate a set of entities around a world pivot point by an angle.
    static size_t rotateEntities(
        kalara::architecture::Level& level,
        const std::vector<kalara::architecture::EntityId>& entityIds,
        const kalara::core::geometry::Point2D& pivot_mm,
        kalara::core::geometry::Angle angle
    );

    /// Align a set of entities relative to their collective bounding box or primary reference.
    static size_t alignEntities(
        kalara::architecture::Level& level,
        const std::vector<kalara::architecture::EntityId>& entityIds,
        AlignmentType alignment
    );

    /// Resize a wall to a new length in mm.
    /// If anchorAtStart is true, start is fixed and end moves; if false, end is fixed and start moves.
    static bool resizeWall(
        kalara::architecture::Level& level,
        const kalara::architecture::EntityId& wallId,
        double newLength_mm,
        bool anchorAtStart = true
    );
};

} // namespace kalara::runtime
