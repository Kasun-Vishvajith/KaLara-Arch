#pragma once
#include "kalara/architecture/wall.hpp"
#include <optional>
#include <vector>

namespace kalara::architecture {

enum class JoinType {
    None,
    L_Corner, // Corner join between 2 wall endpoints
    T_Junction, // End of wall meeting another wall along its body
    X_Cross     // Intersection of 2 crossing walls
};

struct WallJoin {
    JoinType type = JoinType::None;
    EntityId wallA;
    EntityId wallB;
    kalara::core::geometry::Point2D junctionPoint;
};

/// Computes join relationships and corner mitre/intersection geometry between walls.
class WallJoinSolver {
public:
    /// Check for endpoint coincidence or intersection between two walls.
    static std::optional<WallJoin> evaluateJoin(const Wall& w1, const Wall& w2, double snapTolerance_mm = 50.0);

    /// Connect endpoint of w1 to endpoint of w2 if within tolerance.
    static bool snapEndpoints(Wall& w1, Wall& w2, double snapTolerance_mm = 50.0);
};

} // namespace kalara::architecture
