#pragma once
#include "kalara/core/geometry/point_vector.hpp"
#include <algorithm>

namespace kalara::core::geometry {

/// Axis-Aligned 2D Rectangle in millimetres.
class Rect2D {
public:
    Point2D min;
    Point2D max;

    constexpr Rect2D() noexcept
        : min(0.0, 0.0), max(0.0, 0.0) {}

    constexpr Rect2D(const Point2D& p1, const Point2D& p2) noexcept
        : min(std::min(p1.x, p2.x), std::min(p1.y, p2.y)),
          max(std::max(p1.x, p2.x), std::max(p1.y, p2.y)) {}

    constexpr Rect2D(double x, double y, double width, double height) noexcept
        : min(x, y), max(x + width, y + height) {}

    static Rect2D fromCenterAndSize(const Point2D& center, double width, double height) noexcept {
        double hw = width * 0.5;
        double hh = height * 0.5;
        return Rect2D(center.x - hw, center.y - hh, width, height);
    }

    double width() const noexcept { return max.x - min.x; }
    double height() const noexcept { return max.y - min.y; }

    /// Area in mm^2
    double area() const noexcept { return width() * height(); }

    /// Perimeter in mm
    double perimeter() const noexcept { return 2.0 * (width() + height()); }

    Point2D center() const noexcept {
        return Point2D((min.x + max.x) * 0.5, (min.y + max.y) * 0.5);
    }

    bool contains(const Point2D& p, double eps = Tolerances::LinearEpsilon_mm) const noexcept {
        return p.x >= (min.x - eps) && p.x <= (max.x + eps) &&
               p.y >= (min.y - eps) && p.y <= (max.y + eps);
    }

    bool intersects(const Rect2D& other, double eps = Tolerances::LinearEpsilon_mm) const noexcept {
        return (min.x - eps) <= other.max.x && (max.x + eps) >= other.min.x &&
               (min.y - eps) <= other.max.y && (max.y + eps) >= other.min.y;
    }

    Rect2D united(const Rect2D& other) const noexcept {
        return Rect2D(
            Point2D(std::min(min.x, other.min.x), std::min(min.y, other.min.y)),
            Point2D(std::max(max.x, other.max.x), std::max(max.y, other.max.y))
        );
    }

    bool operator==(const Rect2D& other) const noexcept {
        return min == other.min && max == other.max;
    }
};

} // namespace kalara::core::geometry
