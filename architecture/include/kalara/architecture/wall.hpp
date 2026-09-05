#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/metadata.hpp"
#include "kalara/architecture/semantics.hpp"
#include "kalara/core/geometry/segment.hpp"
#include "kalara/core/geometry/point_vector.hpp"
#include <vector>
#include <string>

namespace kalara::architecture {

/// User preference for how wall thickness changes are applied (Rule 6).
enum class WallThicknessAlignment {
    Centerline,
    PreserveOuterFace,
    PreserveInnerFace
};

/// Architectural Wall Entity (Rule 5 & Principle 2).
/// Represents geometry (centerline, thickness, height), semantics, and intent.
class Wall {
public:
    EntityId id;
    std::string name;
    kalara::core::geometry::Point2D start;
    kalara::core::geometry::Point2D end;
    double thickness_mm = 200.0; // Default wall thickness 200 mm
    double height_mm = 3000.0;    // Default wall height 3000 mm
    Metadata metadata;

    Wall()
        : id(EntityId::generate("wall")),
          start(0.0, 0.0),
          end(4000.0, 0.0),
          thickness_mm(200.0) {}

    Wall(kalara::core::geometry::Point2D s, kalara::core::geometry::Point2D e, double thickness = 200.0, double height = 3000.0)
        : id(EntityId::generate("wall")),
          start(s),
          end(e),
          thickness_mm(thickness),
          height_mm(height) {}

    [[nodiscard]] SemanticType semanticType() const noexcept { return SemanticType::Wall; }

    [[nodiscard]] kalara::core::geometry::Segment2D centerline() const noexcept {
        return kalara::core::geometry::Segment2D(start, end);
    }

    [[nodiscard]] double length_mm() const noexcept {
        return start.distanceTo(end);
    }

    [[nodiscard]] kalara::core::geometry::Vector2D direction() const noexcept {
        return (end - start).normalized();
    }

    [[nodiscard]] kalara::core::geometry::Vector2D normal() const noexcept {
        return direction().perpendicular();
    }

    /// Set thickness respecting the user's alignment preference (Rule 6).
    void setThickness(double newThickness_mm, WallThicknessAlignment alignment = WallThicknessAlignment::Centerline);

    /// Compute the 4 corner points of the wall boundary polygon (in mm).
    /// Ordered: [StartLeft, EndLeft, EndRight, StartRight]
    [[nodiscard]] std::vector<kalara::core::geometry::Point2D> boundaryPolygon() const noexcept;

    /// Hit-test: check if a world point lies within the wall's boundary.
    [[nodiscard]] bool containsPoint(const kalara::core::geometry::Point2D& p) const noexcept;
};

} // namespace kalara::architecture
