#include "kalara/architecture/wall.hpp"
#include "kalara/core/geometry/ops.hpp"

namespace kalara::architecture {

void Wall::setThickness(double newThickness_mm, WallThicknessAlignment alignment) {
    if (newThickness_mm <= 0.0) return;

    double delta = newThickness_mm - thickness_mm;
    auto n = normal();

    switch (alignment) {
        case WallThicknessAlignment::Centerline:
            // Centerline does not move; thickness symmetrically expands
            break;
        case WallThicknessAlignment::PreserveOuterFace:
            // Shift centerline inwards by half of delta
            start = start - n * (delta * 0.5);
            end = end - n * (delta * 0.5);
            break;
        case WallThicknessAlignment::PreserveInnerFace:
            // Shift centerline outwards by half of delta
            start = start + n * (delta * 0.5);
            end = end + n * (delta * 0.5);
            break;
    }

    thickness_mm = newThickness_mm;
}

std::vector<kalara::core::geometry::Point2D> Wall::boundaryPolygon() const noexcept {
    auto n = normal();
    double halfThick = thickness_mm * 0.5;

    auto pStartLeft  = start + n * halfThick;
    auto pEndLeft    = end   + n * halfThick;
    auto pEndRight   = end   - n * halfThick;
    auto pStartRight = start - n * halfThick;

    return { pStartLeft, pEndLeft, pEndRight, pStartRight };
}

bool Wall::containsPoint(const kalara::core::geometry::Point2D& p) const noexcept {
    auto poly = boundaryPolygon();
    return kalara::core::geometry::GeometricOps::pointInPolygon(p, poly);
}

} // namespace kalara::architecture
