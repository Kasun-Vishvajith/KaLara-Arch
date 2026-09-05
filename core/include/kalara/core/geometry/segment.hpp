#pragma once
#include "kalara/core/geometry/point_vector.hpp"
#include "kalara/core/geometry/transform.hpp"
#include <optional>
#include <algorithm>

namespace kalara::core::geometry {

/// Directed 2D Line Segment in millimetres.
class Segment2D {
public:
    Point2D start;
    Point2D end;

    constexpr Segment2D() noexcept = default;
    constexpr Segment2D(const Point2D& s, const Point2D& e) noexcept : start(s), end(e) {}
    constexpr Segment2D(double x1, double y1, double x2, double y2) noexcept
        : start(x1, y1), end(x2, y2) {}

    double length() const noexcept {
        return start.distanceTo(end);
    }

    double lengthSquared() const noexcept {
        return start.distanceSquaredTo(end);
    }

    Vector2D direction() const noexcept {
        return (end - start).normalized();
    }

    Point2D midpoint() const noexcept {
        return Point2D((start.x + end.x) * 0.5, (start.y + end.y) * 0.5);
    }

    Vector2D normal() const noexcept {
        return direction().perpendicular();
    }

    Angle angle() const noexcept {
        return (end - start).angle();
    }

    bool isOrthogonal(double eps = Tolerances::AngularEpsilon_rad) const noexcept {
        return angle().isOrthogonal(eps);
    }

    /// Point projected perpendicularly onto the infinite line containing this segment.
    Point2D projectOntoLine(const Point2D& p) const noexcept {
        Vector2D v = end - start;
        double lenSq = v.lengthSquared();
        if (lenSq <= Tolerances::LinearEpsilon_mm * Tolerances::LinearEpsilon_mm) {
            return start;
        }
        double t = (p - start).dot(v) / lenSq;
        return start + v * t;
    }

    /// Closest point on the bounded segment to point p.
    Point2D closestPoint(const Point2D& p) const noexcept {
        Vector2D v = end - start;
        double lenSq = v.lengthSquared();
        if (lenSq <= Tolerances::LinearEpsilon_mm * Tolerances::LinearEpsilon_mm) {
            return start;
        }
        double t = std::clamp((p - start).dot(v) / lenSq, 0.0, 1.0);
        return start + v * t;
    }

    double distanceToPoint(const Point2D& p) const noexcept {
        return p.distanceTo(closestPoint(p));
    }

    /// Intersection between two segments. Returns nullopt if parallel or disjoint.
    std::optional<Point2D> intersect(const Segment2D& other, double eps = Tolerances::LinearEpsilon_mm) const noexcept;

    Segment2D transformed(const Transform2D& t) const noexcept {
        return Segment2D(t.map(start), t.map(end));
    }

    bool operator==(const Segment2D& other) const noexcept {
        return start == other.start && end == other.end;
    }
};

} // namespace kalara::core::geometry
