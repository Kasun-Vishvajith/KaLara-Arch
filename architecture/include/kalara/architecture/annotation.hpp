#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/metadata.hpp"
#include "kalara/architecture/semantics.hpp"
#include "kalara/architecture/wall.hpp"
#include "kalara/core/geometry/point_vector.hpp"
#include "kalara/core/geometry/units.hpp"
#include <string>
#include <vector>
#include <optional>

namespace kalara::architecture {

/// Professional drawing scale ratios (Section 8).
enum class DrawingScale {
    Scale_1_20,
    Scale_1_50,
    Scale_1_100,
    Scale_1_200,
    Scale_1_500
};

constexpr double scaleRatio(DrawingScale scale) noexcept {
    switch (scale) {
        case DrawingScale::Scale_1_20:  return 1.0 / 20.0;
        case DrawingScale::Scale_1_50:  return 1.0 / 50.0;
        case DrawingScale::Scale_1_100: return 1.0 / 100.0;
        case DrawingScale::Scale_1_200: return 1.0 / 200.0;
        case DrawingScale::Scale_1_500: return 1.0 / 500.0;
    }
    return 1.0 / 100.0;
}

/// Associative Linear Dimension entity (Rule 7).
/// References either two explicit points or tracks a host entity (e.g. Wall) by ID.
class Dimension {
public:
    EntityId id;
    std::optional<EntityId> referencedEntityId; // e.g. Wall ID for associative updates
    kalara::core::geometry::Point2D point1;
    kalara::core::geometry::Point2D point2;
    double offsetDistance_mm = 500.0; // Distance offset from measured line for dimension line
    Metadata metadata;

    Dimension()
        : id(EntityId::generate("dim")),
          point1(0.0, 0.0),
          point2(1000.0, 0.0) {}

    Dimension(kalara::core::geometry::Point2D p1, kalara::core::geometry::Point2D p2, double offset = 500.0)
        : id(EntityId::generate("dim")),
          point1(p1),
          point2(p2),
          offsetDistance_mm(offset) {}

    Dimension(EntityId wallId, const Wall& wall, double offset = 500.0)
        : id(EntityId::generate("dim")),
          referencedEntityId(std::move(wallId)),
          point1(wall.start),
          point2(wall.end),
          offsetDistance_mm(offset) {}

    [[nodiscard]] SemanticType semanticType() const noexcept { return SemanticType::Dimension; }

    /// Re-sync dimension with its referenced entity geometry (Rule 7).
    void syncWithReferencedWall(const Wall& wall) noexcept {
        point1 = wall.start;
        point2 = wall.end;
    }

    /// Measured distance in canonical millimetres (Rule 7).
    [[nodiscard]] double measuredDistance_mm() const noexcept {
        return point1.distanceTo(point2);
    }

    /// Formatted measurement text in specified display unit.
    [[nodiscard]] std::string formattedText(kalara::core::geometry::UnitSystem unit = kalara::core::geometry::UnitSystem::Millimetres) const {
        return kalara::core::geometry::UnitConverter::format(measuredDistance_mm(), unit, 0);
    }

    /// Calculate parallel dimension line start & end points in world mm.
    [[nodiscard]] std::pair<kalara::core::geometry::Point2D, kalara::core::geometry::Point2D> dimensionLine() const noexcept;
};

/// Architectural Text Note / Annotation.
class NoteAnnotation {
public:
    EntityId id;
    kalara::core::geometry::Point2D position;
    std::string text;
    Metadata metadata;

    NoteAnnotation(kalara::core::geometry::Point2D pos, std::string noteText)
        : id(EntityId::generate("note")),
          position(pos),
          text(std::move(noteText)) {}

    [[nodiscard]] SemanticType semanticType() const noexcept { return SemanticType::Annotation; }
};

/// North Arrow Indicator.
class NorthArrow {
public:
    EntityId id;
    kalara::core::geometry::Point2D position{0.0, 0.0};
    double angleDegrees = 0.0; // 0 = pointing North (+Y)

    NorthArrow() : id(EntityId::generate("north")) {}
    NorthArrow(kalara::core::geometry::Point2D pos, double angle = 0.0)
        : id(EntityId::generate("north")), position(pos), angleDegrees(angle) {}

    [[nodiscard]] SemanticType semanticType() const noexcept { return SemanticType::Annotation; }
};

} // namespace kalara::architecture
