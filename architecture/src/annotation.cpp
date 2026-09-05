#include "kalara/architecture/annotation.hpp"

namespace kalara::architecture {

std::pair<kalara::core::geometry::Point2D, kalara::core::geometry::Point2D> Dimension::dimensionLine() const noexcept {
    auto dir = (point2 - point1).normalized();
    auto normal = dir.perpendicular();

    auto d1 = point1 + normal * offsetDistance_mm;
    auto d2 = point2 + normal * offsetDistance_mm;
    return { d1, d2 };
}

} // namespace kalara::architecture
