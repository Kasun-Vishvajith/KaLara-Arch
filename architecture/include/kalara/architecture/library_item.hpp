#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/metadata.hpp"
#include "kalara/architecture/semantics.hpp"
#include "kalara/core/geometry/point_vector.hpp"
#include "kalara/core/geometry/rect.hpp"
#include "kalara/core/geometry/segment.hpp"
#include "kalara/core/geometry/angle.hpp"
#include "kalara/core/geometry/transform.hpp"
#include "kalara/core/geometry/ops.hpp"
#include <string>
#include <vector>
#include <memory>
#include <string_view>

namespace kalara::architecture {

/// Architectural library categories (Step 11).
enum class LibraryCategory {
    Furniture,
    BathroomFixtures,
    KitchenObjects,
    Stairs,
    DoorsAndWindows,
    SiteOutdoor
};

constexpr std::string_view libraryCategoryName(LibraryCategory cat) noexcept {
    switch (cat) {
        case LibraryCategory::Furniture:        return "Furniture";
        case LibraryCategory::BathroomFixtures: return "Bathroom Fixtures";
        case LibraryCategory::KitchenObjects:   return "Kitchen Objects";
        case LibraryCategory::Stairs:           return "Stairs";
        case LibraryCategory::DoorsAndWindows:  return "Doors & Windows";
        case LibraryCategory::SiteOutdoor:      return "Site / Outdoor";
    }
    return "Furniture";
}

/// Architectural clearance requirements around an object (Rule 11).
struct ClearanceProfile {
    double front_mm = 0.0;
    double rear_mm = 0.0;
    double left_mm = 0.0;
    double right_mm = 0.0;

    [[nodiscard]] bool hasClearance() const noexcept {
        return front_mm > 0.0 || rear_mm > 0.0 || left_mm > 0.0 || right_mm > 0.0;
    }
};

/// 2D linework primitive for detailed architectural symbol representation.
struct SymbolPrimitive {
    enum class Type { Line, Circle, Arc };
    Type type = Type::Line;
    kalara::core::geometry::Point2D p1;
    kalara::core::geometry::Point2D p2;
    double radius_mm = 0.0;
    kalara::core::geometry::Angle startAngle{};
    kalara::core::geometry::Angle sweepAngle{};

    static SymbolPrimitive makeLine(kalara::core::geometry::Point2D a, kalara::core::geometry::Point2D b) {
        SymbolPrimitive s;
        s.type = Type::Line;
        s.p1 = a;
        s.p2 = b;
        return s;
    }

    static SymbolPrimitive makeCircle(kalara::core::geometry::Point2D center, double r) {
        SymbolPrimitive s;
        s.type = Type::Circle;
        s.p1 = center;
        s.radius_mm = r;
        return s;
    }
};

/// Reusable 2D Architectural Catalog Item definition (Rule 11 & Step 11).
class LibraryItem {
public:
    std::string id;          // Catalog code, e.g., "BED_DOUBLE"
    std::string name;        // e.g. "Double Bed (Queen)"
    LibraryCategory category = LibraryCategory::Furniture;
    SemanticType semanticType = SemanticType::Furniture;
    double width_mm = 1600.0;
    double length_mm = 2000.0;
    double height_mm = 600.0; // Elevation / 3D information preserved (Principle 8)
    ClearanceProfile clearance;
    std::vector<kalara::core::geometry::Point2D> anchorPoints; // Relative to center (0, 0)
    std::vector<SymbolPrimitive> detailGeometry;               // Internal symbols (e.g. pillows, burners, treads)
    Metadata metadata;
    bool isUserCreated = false;

    LibraryItem() = default;

    LibraryItem(std::string itemId, std::string itemName, LibraryCategory cat, SemanticType sem,
                double w_mm, double l_mm, double h_mm = 800.0)
        : id(std::move(itemId)),
          name(std::move(itemName)),
          category(cat),
          semanticType(sem),
          width_mm(w_mm),
          length_mm(l_mm),
          height_mm(h_mm) {
        // Default anchor points: Center, 4 corners, and 4 edge midpoints
        double hw = width_mm * 0.5;
        double hl = length_mm * 0.5;
        anchorPoints = {
            {0.0, 0.0},     // Center
            {-hw, -hl},     // Bottom-Left
            {hw, -hl},      // Bottom-Right
            {hw, hl},       // Top-Right
            {-hw, hl},      // Top-Left
            {0.0, -hl},     // Bottom-Mid
            {hw, 0.0},      // Right-Mid
            {0.0, hl},      // Top-Mid
            {-hw, 0.0}      // Left-Mid
        };
    }

    /// Local boundary polygon centered at (0, 0) in mm.
    [[nodiscard]] std::vector<kalara::core::geometry::Point2D> localBoundary() const noexcept {
        double hw = width_mm * 0.5;
        double hl = length_mm * 0.5;
        return {
            {-hw, -hl},
            {hw, -hl},
            {hw, hl},
            {-hw, hl}
        };
    }

    /// Local clearance zone polygon centered at (0, 0) in mm.
    [[nodiscard]] std::vector<kalara::core::geometry::Point2D> localClearanceZone() const noexcept {
        double hw = width_mm * 0.5;
        double hl = length_mm * 0.5;
        return {
            {-hw - clearance.left_mm, -hl - clearance.rear_mm},
            {hw + clearance.right_mm, -hl - clearance.rear_mm},
            {hw + clearance.right_mm, hl + clearance.front_mm},
            {-hw - clearance.left_mm, hl + clearance.front_mm}
        };
    }
};

/// Placed instance of a library item on a Level (Step 11).
class LibraryInstance {
public:
    EntityId id;
    std::string itemId;                                    // ID of catalog item definition
    std::string name;
    kalara::core::geometry::Point2D position{0.0, 0.0};    // Center anchor in world mm
    kalara::core::geometry::Angle rotation{};
    double width_mm = 0.0;                                 // Instance dimension override (or matches item)
    double length_mm = 0.0;
    ClearanceProfile clearance;
    SemanticType semanticType = SemanticType::Furniture;
    Metadata metadata;

    LibraryInstance() : id(EntityId::generate("lib_inst")) {}

    LibraryInstance(const LibraryItem& item, kalara::core::geometry::Point2D pos,
                    kalara::core::geometry::Angle rot = kalara::core::geometry::Angle{})
        : id(EntityId::generate("lib_inst")),
          itemId(item.id),
          name(item.name),
          position(pos),
          rotation(rot),
          width_mm(item.width_mm),
          length_mm(item.length_mm),
          clearance(item.clearance),
          semanticType(item.semanticType) {}

    /// Compute world transform for this instance.
    [[nodiscard]] kalara::core::geometry::Transform2D transform() const noexcept {
        return kalara::core::geometry::Transform2D::translation(position.x, position.y) *
               kalara::core::geometry::Transform2D::rotation(rotation);
    }

    /// Compute world boundary polygon in mm.
    [[nodiscard]] std::vector<kalara::core::geometry::Point2D> worldBoundary() const noexcept {
        double hw = width_mm * 0.5;
        double hl = length_mm * 0.5;
        std::vector<kalara::core::geometry::Point2D> local = {
            {-hw, -hl}, {hw, -hl}, {hw, hl}, {-hw, hl}
        };
        auto xform = transform();
        std::vector<kalara::core::geometry::Point2D> world;
        world.reserve(4);
        for (const auto& p : local) {
            world.push_back(xform.map(p));
        }
        return world;
    }

    /// Compute world clearance polygon in mm.
    [[nodiscard]] std::vector<kalara::core::geometry::Point2D> worldClearanceZone() const noexcept {
        double hw = width_mm * 0.5;
        double hl = length_mm * 0.5;
        std::vector<kalara::core::geometry::Point2D> local = {
            {-hw - clearance.left_mm, -hl - clearance.rear_mm},
            {hw + clearance.right_mm, -hl - clearance.rear_mm},
            {hw + clearance.right_mm, hl + clearance.front_mm},
            {-hw - clearance.left_mm, hl + clearance.front_mm}
        };
        auto xform = transform();
        std::vector<kalara::core::geometry::Point2D> world;
        world.reserve(4);
        for (const auto& p : local) {
            world.push_back(xform.map(p));
        }
        return world;
    }

    /// Hit-test: check if world point lies within instance boundary.
    [[nodiscard]] bool containsPoint(const kalara::core::geometry::Point2D& p) const noexcept {
        return kalara::core::geometry::GeometricOps::pointInPolygon(p, worldBoundary());
    }

    /// Bounding box in world coordinates.
    [[nodiscard]] kalara::core::geometry::Rect2D boundingBox() const noexcept {
        return kalara::core::geometry::GeometricOps::boundingBox(worldBoundary());
    }
};

} // namespace kalara::architecture
