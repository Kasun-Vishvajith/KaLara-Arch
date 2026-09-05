#pragma once
#include "kalara/architecture/constraint.hpp"
#include "kalara/architecture/wall.hpp"
#include "kalara/architecture/level.hpp"
#include "kalara/architecture/site.hpp"
#include <vector>

namespace kalara::architecture {

/// Preference for propagating geometry changes through connected constraints (Rule 6 & Step 09).
enum class ConnectedGeometryPropagation {
    PropagateConnected,
    PreserveSelectedOnly
};

/// Evaluates first-class constraints, checks clearances, propagates edits, and reports conflicts.
class ConstraintSolver {
public:
    /// Validate all constraints on a level and return any conflicts (Step 09).
    static std::vector<ConstraintConflict> validateConstraints(
        const std::vector<std::unique_ptr<Constraint>>& constraints,
        const Level& level,
        const Site* site = nullptr
    );

    /// Move a wall endpoint and optionally propagate connected coincidence constraints (Rule 6).
    static size_t moveWallEndpoint(
        Wall& targetWall,
        bool isStartPoint,
        const kalara::core::geometry::Point2D& newPos,
        Level& level,
        ConnectedGeometryPropagation propagation,
        double snapTolerance_mm = 50.0
    );

    /// Validate host relationship: ensure opening lies completely on host wall.
    static std::vector<ConstraintConflict> validateOpenings(const Level& level);
};

} // namespace kalara::architecture
