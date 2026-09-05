#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/metadata.hpp"
#include "kalara/architecture/semantics.hpp"
#include "kalara/core/geometry/point_vector.hpp"
#include "kalara/core/geometry/rect.hpp"
#include <string>
#include <vector>
#include <string_view>

namespace kalara::architecture {

/// Architectural Room classification types.
enum class RoomType {
    Custom,
    LivingRoom,
    Bedroom,
    Kitchen,
    Bathroom,
    DiningRoom,
    Corridor,
    Office,
    Balcony,
    Garage,
    Utility
};

constexpr std::string_view roomTypeName(RoomType type) noexcept {
    switch (type) {
        case RoomType::Custom:     return "Custom";
        case RoomType::LivingRoom: return "Living Room";
        case RoomType::Bedroom:    return "Bedroom";
        case RoomType::Kitchen:    return "Kitchen";
        case RoomType::Bathroom:   return "Bathroom";
        case RoomType::DiningRoom: return "Dining Room";
        case RoomType::Corridor:   return "Corridor";
        case RoomType::Office:     return "Office";
        case RoomType::Balcony:    return "Balcony";
        case RoomType::Garage:     return "Garage";
        case RoomType::Utility:    return "Utility";
    }
    return "Custom";
}

/// Architectural Room / Space entity (Rule 10).
/// Supports explicit definitions and automatically derived boundaries.
class Room {
public:
    EntityId id;
    std::string name;
    RoomType type = RoomType::LivingRoom;
    std::vector<kalara::core::geometry::Point2D> boundary;
    std::vector<EntityId> boundaryWallIds; // References to bounding walls if derived
    Metadata metadata;

    Room()
        : id(EntityId::generate("room")),
          name("Living Room"),
          type(RoomType::LivingRoom) {}

    Room(std::string roomName, RoomType roomType, std::vector<kalara::core::geometry::Point2D> roomBoundary)
        : id(EntityId::generate("room")),
          name(std::move(roomName)),
          type(roomType),
          boundary(std::move(roomBoundary)) {}

    [[nodiscard]] SemanticType semanticType() const noexcept { return SemanticType::Room; }

    /// Calculated floor area in square millimetres (mm^2).
    [[nodiscard]] double area_mm2() const noexcept;

    /// Calculated floor area in square metres (m^2).
    [[nodiscard]] double area_m2() const noexcept;

    /// Calculated room boundary perimeter in millimetres (mm).
    [[nodiscard]] double perimeter_mm() const noexcept;

    /// Bounding box dimensions (width, length).
    [[nodiscard]] kalara::core::geometry::Rect2D boundingBox() const noexcept;
    [[nodiscard]] double width_mm() const noexcept;
    [[nodiscard]] double length_mm() const noexcept;

    /// Center of room for label placement.
    [[nodiscard]] kalara::core::geometry::Point2D labelPosition() const noexcept;

    /// Hit-test if a point is inside the room.
    [[nodiscard]] bool containsPoint(const kalara::core::geometry::Point2D& p) const noexcept;
};

} // namespace kalara::architecture
