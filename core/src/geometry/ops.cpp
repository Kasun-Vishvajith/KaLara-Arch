#include "kalara/core/geometry/ops.hpp"
#include <cmath>

namespace kalara::core::geometry {

double GeometricOps::polygonSignedArea(const std::vector<Point2D>& polygon) noexcept {
    if (polygon.size() < 3) return 0.0;
    double area = 0.0;
    size_t n = polygon.size();
    for (size_t i = 0; i < n; ++i) {
        size_t j = (i + 1) % n;
        area += polygon[i].x * polygon[j].y;
        area -= polygon[j].x * polygon[i].y;
    }
    return area * 0.5;
}

double GeometricOps::polygonArea(const std::vector<Point2D>& polygon) noexcept {
    return std::abs(polygonSignedArea(polygon));
}

double GeometricOps::polygonPerimeter(const std::vector<Point2D>& polygon) noexcept {
    if (polygon.size() < 2) return 0.0;
    double perim = 0.0;
    size_t n = polygon.size();
    for (size_t i = 0; i < n; ++i) {
        size_t j = (i + 1) % n;
        perim += polygon[i].distanceTo(polygon[j]);
    }
    return perim;
}

bool GeometricOps::pointInPolygon(const Point2D& point, const std::vector<Point2D>& polygon) noexcept {
    if (polygon.size() < 3) return false;
    bool inside = false;
    size_t n = polygon.size();
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        if (((polygon[i].y > point.y) != (polygon[j].y > point.y)) &&
            (point.x < (polygon[j].x - polygon[i].x) * (point.y - polygon[i].y) /
                       (polygon[j].y - polygon[i].y) + polygon[i].x)) {
            inside = !inside;
        }
    }
    return inside;
}

Rect2D GeometricOps::boundingBox(const std::vector<Point2D>& points) noexcept {
    if (points.empty()) return Rect2D();
    double minX = points[0].x, maxX = points[0].x;
    double minY = points[0].y, maxY = points[0].y;
    for (const auto& p : points) {
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }
    return Rect2D(Point2D(minX, minY), Point2D(maxX, maxY));
}

Point2D GeometricOps::snapToOrthogonal(const Point2D& ref, const Point2D& current) noexcept {
    double dx = std::abs(current.x - ref.x);
    double dy = std::abs(current.y - ref.y);
    if (dx >= dy) {
        // Constrain to horizontal
        return Point2D(current.x, ref.y);
    } else {
        // Constrain to vertical
        return Point2D(ref.x, current.y);
    }
}

Point2D GeometricOps::snapToGrid(const Point2D& point, double gridSpacing_mm) noexcept {
    if (gridSpacing_mm <= Tolerances::LinearEpsilon_mm) return point;
    double x = std::round(point.x / gridSpacing_mm) * gridSpacing_mm;
    double y = std::round(point.y / gridSpacing_mm) * gridSpacing_mm;
    return Point2D(x, y);
}

} // namespace kalara::core::geometry
