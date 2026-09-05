#pragma once
#include "kalara/core/geometry/point_vector.hpp"
#include "kalara/core/geometry/segment.hpp"
#include "kalara/core/geometry/rect.hpp"
#include <vector>

namespace kalara::core::geometry {

/// Basic geometric operations and algorithms for 2D architectural planning.
class GeometricOps {
public:
    /// Calculate polygon area using Shoelace formula (in mm^2).
    /// Positive for counter-clockwise, negative for clockwise.
    static double polygonSignedArea(const std::vector<Point2D>& polygon) noexcept;

    /// Absolute area of polygon in mm^2.
    static double polygonArea(const std::vector<Point2D>& polygon) noexcept;

    /// Perimeter length of polygon in mm.
    static double polygonPerimeter(const std::vector<Point2D>& polygon) noexcept;

    /// Test if point is inside a polygon using ray casting.
    static bool pointInPolygon(const Point2D& point, const std::vector<Point2D>& polygon) noexcept;

    /// Compute axis-aligned bounding box of points.
    static Rect2D boundingBox(const std::vector<Point2D>& points) noexcept;

    /// Snap point to nearest orthogonal angle relative to reference point.
    static Point2D snapToOrthogonal(const Point2D& ref, const Point2D& current) noexcept;

    /// Snap point to grid of specified spacing in mm.
    static Point2D snapToGrid(const Point2D& point, double gridSpacing_mm) noexcept;
};

} // namespace kalara::core::geometry
