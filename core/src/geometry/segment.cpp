#include "kalara/core/geometry/segment.hpp"

namespace kalara::core::geometry {

std::optional<Point2D> Segment2D::intersect(const Segment2D& other, double eps) const noexcept {
    Vector2D r = end - start;
    Vector2D s = other.end - other.start;

    double rxs = r.cross(s);
    Vector2D qp = other.start - start;
    double qpxr = qp.cross(r);

    // Parallel or collinear
    if (std::abs(rxs) <= eps * eps) {
        return std::nullopt;
    }

    double t = qp.cross(s) / rxs;
    double u = qpxr / rxs;

    if (t >= -eps && t <= 1.0 + eps && u >= -eps && u <= 1.0 + eps) {
        return start + r * t;
    }

    return std::nullopt;
}

} // namespace kalara::core::geometry
