#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/metadata.hpp"
#include "kalara/architecture/semantics.hpp"
#include "kalara/core/geometry/point_vector.hpp"
#include "kalara/core/geometry/angle.hpp"
#include "kalara/core/geometry/segment.hpp"
#include "kalara/core/geometry/rect.hpp"
#include "kalara/core/geometry/ops.hpp"
#include <string>
#include <vector>
#include <string_view>
#include <algorithm>
#include <cmath>

namespace kalara::architecture {

/// Property setback requirements defining the buildable envelope (Step 13).
struct Setbacks {
    double front_mm = 5000.0;
    double rear_mm = 3000.0;
    double sideLeft_mm = 2000.0;
    double sideRight_mm = 2000.0;

    /// Computes the buildable envelope polygon within a property boundary.
    /// For rectangular / axis-aligned sites, offsets the boundary by the respective setbacks.
    [[nodiscard]] std::vector<kalara::core::geometry::Point2D> computeBuildableEnvelope(
        const std::vector<kalara::core::geometry::Point2D>& propertyBoundary) const
    {
        if (propertyBoundary.size() < 3) return {};

        auto bbox = kalara::core::geometry::GeometricOps::boundingBox(propertyBoundary);
        double minX = bbox.min.x + sideLeft_mm;
        double maxX = bbox.max.x - sideRight_mm;
        double minY = bbox.min.y + front_mm;  // Assuming front is along South / -Y
        double maxY = bbox.max.y - rear_mm;   // Assuming rear is along North / +Y

        if (minX >= maxX || minY >= maxY) {
            return {}; // No buildable envelope possible
        }

        return {
            kalara::core::geometry::Point2D(minX, minY),
            kalara::core::geometry::Point2D(maxX, minY),
            kalara::core::geometry::Point2D(maxX, maxY),
            kalara::core::geometry::Point2D(minX, maxY)
        };
    }
};

/// Street / road context along the site boundary.
class RoadContext {
public:
    EntityId id;
    std::string name;
    double width_mm = 10000.0;
    kalara::core::geometry::Segment2D centerline;
    double curbOffset_mm = 1500.0;
    Metadata metadata;

    RoadContext()
        : id(EntityId::generate("road")), name("Main Street") {}

    RoadContext(std::string roadName, kalara::core::geometry::Segment2D roadCenter, double width = 10000.0)
        : id(EntityId::generate("road")),
          name(std::move(roadName)),
          width_mm(width),
          centerline(roadCenter) {}

    [[nodiscard]] SemanticType semanticType() const noexcept { return SemanticType::SiteElement; }
};

/// Type of site access / entrance.
enum class SiteEntranceType {
    VehicularDriveway,
    PedestrianGate,
    ServiceEntrance
};

constexpr std::string_view siteEntranceTypeName(SiteEntranceType type) noexcept {
    switch (type) {
        case SiteEntranceType::VehicularDriveway: return "Driveway";
        case SiteEntranceType::PedestrianGate:    return "Pedestrian Gate";
        case SiteEntranceType::ServiceEntrance:   return "Service Entrance";
    }
    return "Entrance";
}

/// Site access entrance or driveway point.
class SiteEntrance {
public:
    EntityId id;
    std::string name;
    SiteEntranceType type = SiteEntranceType::VehicularDriveway;
    kalara::core::geometry::Point2D position;
    double width_mm = 3600.0;
    kalara::core::geometry::Angle orientation;
    Metadata metadata;

    SiteEntrance()
        : id(EntityId::generate("ent_site")), name("Main Entrance") {}

    SiteEntrance(std::string entranceName, SiteEntranceType entranceType,
                 kalara::core::geometry::Point2D pos, double width = 3600.0,
                 kalara::core::geometry::Angle rot = kalara::core::geometry::Angle{})
        : id(EntityId::generate("ent_site")),
          name(std::move(entranceName)),
          type(entranceType),
          position(pos),
          width_mm(width),
          orientation(rot) {}

    [[nodiscard]] SemanticType semanticType() const noexcept { return SemanticType::SiteElement; }
};

/// Parking bay / lot layout on site.
class ParkingZone {
public:
    EntityId id;
    std::string name;
    kalara::core::geometry::Point2D position; // Base corner
    kalara::core::geometry::Angle angle;
    double stallWidth_mm = 2500.0;
    double stallLength_mm = 5000.0;
    size_t stallCount = 2;
    bool isAccessible = false;
    Metadata metadata;

    ParkingZone()
        : id(EntityId::generate("prk")), name("Parking") {}

    ParkingZone(std::string zoneName, kalara::core::geometry::Point2D pos, size_t count = 2,
                kalara::core::geometry::Angle rot = kalara::core::geometry::Angle{},
                double stallW = 2500.0, double stallL = 5000.0, bool accessible = false)
        : id(EntityId::generate("prk")),
          name(std::move(zoneName)),
          position(pos),
          angle(rot),
          stallWidth_mm(stallW),
          stallLength_mm(stallL),
          stallCount(count),
          isAccessible(accessible) {}

    [[nodiscard]] SemanticType semanticType() const noexcept { return SemanticType::SiteElement; }

    /// Returns the polygonal outline of each individual parking stall.
    [[nodiscard]] std::vector<std::vector<kalara::core::geometry::Point2D>> stallOutlines() const {
        std::vector<std::vector<kalara::core::geometry::Point2D>> stalls;
        stalls.reserve(stallCount);

        double rad = angle.radians();
        kalara::core::geometry::Vector2D dirX(std::cos(rad), std::sin(rad));
        kalara::core::geometry::Vector2D dirY(-std::sin(rad), std::cos(rad));

        for (size_t i = 0; i < stallCount; ++i) {
            kalara::core::geometry::Point2D origin = position + dirX * (static_cast<double>(i) * stallWidth_mm);
            kalara::core::geometry::Point2D p1 = origin;
            kalara::core::geometry::Point2D p2 = origin + dirX * stallWidth_mm;
            kalara::core::geometry::Point2D p3 = p2 + dirY * stallLength_mm;
            kalara::core::geometry::Point2D p4 = p1 + dirY * stallLength_mm;
            stalls.push_back({p1, p2, p3, p4});
        }
        return stalls;
    }

    [[nodiscard]] double totalArea_mm2() const noexcept {
        return stallWidth_mm * stallLength_mm * static_cast<double>(stallCount);
    }
};

/// Landscape zone classifications.
enum class LandscapeType {
    Lawn,
    Garden,
    PavedPatio,
    PavedDriveway,
    Deck,
    Hardscape
};

constexpr std::string_view landscapeTypeName(LandscapeType type) noexcept {
    switch (type) {
        case LandscapeType::Lawn:          return "Lawn";
        case LandscapeType::Garden:        return "Garden";
        case LandscapeType::PavedPatio:    return "Paved Patio";
        case LandscapeType::PavedDriveway: return "Driveway";
        case LandscapeType::Deck:          return "Deck";
        case LandscapeType::Hardscape:     return "Hardscape";
    }
    return "Landscape";
}

/// Landscape zone polygon on site.
class LandscapeZone {
public:
    EntityId id;
    std::string name;
    LandscapeType type = LandscapeType::Lawn;
    std::vector<kalara::core::geometry::Point2D> boundary;
    Metadata metadata;

    LandscapeZone()
        : id(EntityId::generate("lnd")), name("Lawn") {}

    LandscapeZone(std::string zoneName, LandscapeType zoneType,
                  std::vector<kalara::core::geometry::Point2D> outline)
        : id(EntityId::generate("lnd")),
          name(std::move(zoneName)),
          type(zoneType),
          boundary(std::move(outline)) {}

    [[nodiscard]] SemanticType semanticType() const noexcept { return SemanticType::SiteElement; }

    [[nodiscard]] double area_mm2() const noexcept {
        return kalara::core::geometry::GeometricOps::polygonArea(boundary);
    }
};

/// Type of pool or outdoor feature.
enum class OutdoorElementType {
    SwimmingPool,
    SpaHotTub,
    WaterFeature,
    PergolaGazebo
};

constexpr std::string_view outdoorElementTypeName(OutdoorElementType type) noexcept {
    switch (type) {
        case OutdoorElementType::SwimmingPool:   return "Swimming Pool";
        case OutdoorElementType::SpaHotTub:      return "Spa / Hot Tub";
        case OutdoorElementType::WaterFeature:   return "Water Feature";
        case OutdoorElementType::PergolaGazebo:  return "Pergola / Gazebo";
    }
    return "Outdoor Feature";
}

/// Swimming pool or outdoor landscape structure.
class OutdoorElement {
public:
    EntityId id;
    std::string name;
    OutdoorElementType type = OutdoorElementType::SwimmingPool;
    std::vector<kalara::core::geometry::Point2D> boundary;
    double depth_mm = 1500.0;
    double copingSurround_mm = 400.0; // Paved deck surround width
    Metadata metadata;

    OutdoorElement()
        : id(EntityId::generate("out")), name("Swimming Pool") {}

    OutdoorElement(std::string elemName, OutdoorElementType elemType,
                   std::vector<kalara::core::geometry::Point2D> poolBoundary,
                   double depth = 1500.0, double coping = 400.0)
        : id(EntityId::generate("out")),
          name(std::move(elemName)),
          type(elemType),
          boundary(std::move(poolBoundary)),
          depth_mm(depth),
          copingSurround_mm(coping) {}

    [[nodiscard]] SemanticType semanticType() const noexcept { return SemanticType::SiteElement; }

    [[nodiscard]] double waterArea_mm2() const noexcept {
        return kalara::core::geometry::GeometricOps::polygonArea(boundary);
    }

    /// Approximate water volume in Litres (1 m^3 = 1000 L = 1e9 mm^3).
    [[nodiscard]] double waterVolume_liters() const noexcept {
        double volume_mm3 = waterArea_mm2() * depth_mm;
        return volume_mm3 / 1000000.0; // 1,000,000 mm^3 = 1 Litre
    }

    /// Deck/coping surround outline.
    [[nodiscard]] std::vector<kalara::core::geometry::Point2D> copingBoundary() const {
        if (boundary.empty()) return {};
        auto bbox = kalara::core::geometry::GeometricOps::boundingBox(boundary);
        return {
            kalara::core::geometry::Point2D(bbox.min.x - copingSurround_mm, bbox.min.y - copingSurround_mm),
            kalara::core::geometry::Point2D(bbox.max.x + copingSurround_mm, bbox.min.y - copingSurround_mm),
            kalara::core::geometry::Point2D(bbox.max.x + copingSurround_mm, bbox.max.y + copingSurround_mm),
            kalara::core::geometry::Point2D(bbox.min.x - copingSurround_mm, bbox.max.y + copingSurround_mm)
        };
    }
};

/// Spot elevation benchmark marker for terrain / site levels (Step 13).
struct SpotElevation {
    kalara::core::geometry::Point2D position;
    double elevation_mm = 0.0;
    std::string label;
};

} // namespace kalara::architecture
