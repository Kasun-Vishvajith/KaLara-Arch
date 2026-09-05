#include "kalara/architecture/room.hpp"
#include "kalara/core/geometry/ops.hpp"

namespace kalara::architecture {

double Room::area_mm2() const noexcept {
    return kalara::core::geometry::GeometricOps::polygonArea(boundary);
}

double Room::area_m2() const noexcept {
    return area_mm2() / 1000000.0;
}

double Room::perimeter_mm() const noexcept {
    return kalara::core::geometry::GeometricOps::polygonPerimeter(boundary);
}

kalara::core::geometry::Rect2D Room::boundingBox() const noexcept {
    return kalara::core::geometry::GeometricOps::boundingBox(boundary);
}

double Room::width_mm() const noexcept {
    return boundingBox().width();
}

double Room::length_mm() const noexcept {
    return boundingBox().height();
}

kalara::core::geometry::Point2D Room::labelPosition() const noexcept {
    if (boundary.empty()) return kalara::core::geometry::Point2D(0.0, 0.0);

    // Compute polygon centroid for nice label centering
    double cx = 0.0, cy = 0.0;
    double signedArea = kalara::core::geometry::GeometricOps::polygonSignedArea(boundary);
    size_t n = boundary.size();

    if (std::abs(signedArea) > 1.0) {
        for (size_t i = 0; i < n; ++i) {
            size_t j = (i + 1) % n;
            double factor = (boundary[i].x * boundary[j].y - boundary[j].x * boundary[i].y);
            cx += (boundary[i].x + boundary[j].x) * factor;
            cy += (boundary[i].y + boundary[j].y) * factor;
        }
        cx /= (6.0 * signedArea);
        cy /= (6.0 * signedArea);
        return kalara::core::geometry::Point2D(cx, cy);
    }

    return boundingBox().center();
}

bool Room::containsPoint(const kalara::core::geometry::Point2D& p) const noexcept {
    return kalara::core::geometry::GeometricOps::pointInPolygon(p, boundary);
}

} // namespace kalara::architecture
