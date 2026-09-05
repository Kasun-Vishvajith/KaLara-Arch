#pragma once
#include "kalara/core/geometry/point_vector.hpp"
#include <cmath>

namespace kalara::editor {

/// Architectural Grid settings and snapping calculations in canonical millimetres.
class GridSettings {
public:
    bool enabled = true;
    bool snapEnabled = true;

    /// Primary grid spacing in mm (e.g. 1000 mm = 1 m).
    double primarySpacing_mm = 1000.0;

    /// Secondary / minor subdivision spacing in mm (e.g. 100 mm).
    double secondarySpacing_mm = 100.0;

    /// Snap a world coordinate to nearest grid point.
    [[nodiscard]] kalara::core::geometry::Point2D snap(const kalara::core::geometry::Point2D& p) const noexcept {
        if (!snapEnabled) return p;
        double spacing = (secondarySpacing_mm > 0.0) ? secondarySpacing_mm : primarySpacing_mm;
        double sx = std::round(p.x / spacing) * spacing;
        double sy = std::round(p.y / spacing) * spacing;
        return kalara::core::geometry::Point2D(sx, sy);
    }
};

} // namespace kalara::editor
