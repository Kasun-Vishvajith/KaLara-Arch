#include "kalara/architecture/wall_join.hpp"
#include "kalara/core/geometry/tolerances.hpp"

namespace kalara::architecture {

std::optional<WallJoin> WallJoinSolver::evaluateJoin(const Wall& w1, const Wall& w2, double snapTolerance_mm) {
    if (w1.id == w2.id) return std::nullopt;

    // 1. Check L-Corner (endpoint-to-endpoint)
    if (w1.start.coincidesWith(w2.start, snapTolerance_mm)) {
        return WallJoin{JoinType::L_Corner, w1.id, w2.id, w1.start};
    }
    if (w1.start.coincidesWith(w2.end, snapTolerance_mm)) {
        return WallJoin{JoinType::L_Corner, w1.id, w2.id, w1.start};
    }
    if (w1.end.coincidesWith(w2.start, snapTolerance_mm)) {
        return WallJoin{JoinType::L_Corner, w1.id, w2.id, w1.end};
    }
    if (w1.end.coincidesWith(w2.end, snapTolerance_mm)) {
        return WallJoin{JoinType::L_Corner, w1.id, w2.id, w1.end};
    }

    // 2. Check T-Junction (endpoint of w1 on segment of w2, or vice versa)
    if (w2.centerline().distanceToPoint(w1.start) <= snapTolerance_mm) {
        return WallJoin{JoinType::T_Junction, w1.id, w2.id, w1.start};
    }
    if (w2.centerline().distanceToPoint(w1.end) <= snapTolerance_mm) {
        return WallJoin{JoinType::T_Junction, w1.id, w2.id, w1.end};
    }
    if (w1.centerline().distanceToPoint(w2.start) <= snapTolerance_mm) {
        return WallJoin{JoinType::T_Junction, w2.id, w1.id, w2.start};
    }
    if (w1.centerline().distanceToPoint(w2.end) <= snapTolerance_mm) {
        return WallJoin{JoinType::T_Junction, w2.id, w1.id, w2.end};
    }

    // 3. Check X-Cross
    auto inter = w1.centerline().intersect(w2.centerline(), snapTolerance_mm);
    if (inter.has_value()) {
        return WallJoin{JoinType::X_Cross, w1.id, w2.id, inter.value()};
    }

    return std::nullopt;
}

bool WallJoinSolver::snapEndpoints(Wall& w1, Wall& w2, double snapTolerance_mm) {
    if (w1.start.coincidesWith(w2.start, snapTolerance_mm)) {
        w1.start = w2.start;
        return true;
    }
    if (w1.start.coincidesWith(w2.end, snapTolerance_mm)) {
        w1.start = w2.end;
        return true;
    }
    if (w1.end.coincidesWith(w2.start, snapTolerance_mm)) {
        w1.end = w2.start;
        return true;
    }
    if (w1.end.coincidesWith(w2.end, snapTolerance_mm)) {
        w1.end = w2.end;
        return true;
    }
    return false;
}

} // namespace kalara::architecture
