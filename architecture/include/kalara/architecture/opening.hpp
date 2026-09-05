#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/metadata.hpp"
#include "kalara/architecture/semantics.hpp"
#include "kalara/architecture/wall.hpp"
#include "kalara/core/geometry/segment.hpp"
#include "kalara/core/geometry/point_vector.hpp"
#include <string>
#include <vector>

namespace kalara::architecture {

/// Door swing direction and orientation.
enum class DoorSwing {
    LeftInswing,
    RightInswing,
    LeftOutswing,
    RightOutswing
};

/// Window opening styles.
enum class WindowType {
    Fixed,
    Casement,
    Sliding,
    Awning
};

/// Base class for wall-hosted openings (doors and windows).
class Opening {
public:
    EntityId id;
    std::string name;
    EntityId hostWallId; // Stable ID of host wall
    double offsetAlongWall_mm = 1000.0; // Distance in mm from host wall start to opening center
    double width_mm = 900.0;            // Opening width in mm
    double height_mm = 2100.0;          // Opening height in mm
    double sillHeight_mm = 0.0;         // Sill height from level floor in mm (0 for doors)
    Metadata metadata;

    virtual ~Opening() = default;

    [[nodiscard]] virtual SemanticType semanticType() const noexcept = 0;

    /// Calculate center point along host wall centerline.
    [[nodiscard]] kalara::core::geometry::Point2D centerPoint(const Wall& hostWall) const noexcept;

    /// Calculate the 2D opening segment along host wall centerline.
    [[nodiscard]] kalara::core::geometry::Segment2D openingSegment(const Wall& hostWall) const noexcept;

    /// 4 corner points representing the opening cutout through the host wall thickness.
    [[nodiscard]] std::vector<kalara::core::geometry::Point2D> openingBox(const Wall& hostWall) const noexcept;

    /// Hit-test: check if a world point lies within the opening's bounding box.
    [[nodiscard]] bool containsPoint(const kalara::core::geometry::Point2D& p, const Wall& hostWall) const noexcept;

protected:
    Opening(EntityId entityId, std::string entityName, EntityId hostId, double offset, double width, double height, double sill = 0.0)
        : id(std::move(entityId)),
          name(std::move(entityName)),
          hostWallId(std::move(hostId)),
          offsetAlongWall_mm(offset),
          width_mm(width),
          height_mm(height),
          sillHeight_mm(sill) {}
};

/// Semantic Architectural Door (hosted in a wall).
class Door : public Opening {
public:
    DoorSwing swing = DoorSwing::LeftInswing;

    Door(EntityId hostId, double offset = 1000.0, double width = 900.0, double height = 2100.0, DoorSwing doorSwing = DoorSwing::LeftInswing)
        : Opening(EntityId::generate("door"), "Single Door", std::move(hostId), offset, width, height, 0.0),
          swing(doorSwing) {}

    [[nodiscard]] SemanticType semanticType() const noexcept override { return SemanticType::Door; }

    /// Arc points / swing trajectory geometry in 2D for architectural drafting.
    [[nodiscard]] kalara::core::geometry::Point2D hingePoint(const Wall& hostWall) const noexcept;
};

/// Semantic Architectural Window (hosted in a wall).
class Window : public Opening {
public:
    WindowType windowType = WindowType::Casement;

    Window(EntityId hostId, double offset = 1500.0, double width = 1200.0, double height = 1500.0, double sill = 900.0, WindowType type = WindowType::Casement)
        : Opening(EntityId::generate("win"), "Standard Window", std::move(hostId), offset, width, height, sill),
          windowType(type) {}

    [[nodiscard]] SemanticType semanticType() const noexcept override { return SemanticType::Window; }
};

} // namespace kalara::architecture
