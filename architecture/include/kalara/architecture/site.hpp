#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/metadata.hpp"
#include "kalara/architecture/semantics.hpp"
#include "kalara/architecture/building.hpp"
#include "kalara/architecture/site_elements.hpp"
#include "kalara/core/geometry/point_vector.hpp"
#include "kalara/core/geometry/ops.hpp"
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

namespace kalara::architecture {

/// Architectural Site representation (Section 7 & Step 13).
/// Supports property boundary polygon, setbacks, roads, entrances, parking,
/// landscape zones, pools/outdoor elements, north direction, and site annotations.
class Site {
public:
    EntityId id;
    std::string name;
    Metadata metadata;
    std::vector<kalara::core::geometry::Point2D> propertyBoundary;
    Setbacks setbacks;
    double northAngle_deg = 0.0; // Angle in degrees to True North (0 = +Y axis)
    kalara::core::geometry::Point2D northArrowPosition{-9000.0, 13500.0};
    std::vector<SpotElevation> spotElevations;

    Site() : id(EntityId::generate("site")), name("Default Site") {}
    explicit Site(std::string siteName)
        : id(EntityId::generate("site")), name(std::move(siteName)) {}

    [[nodiscard]] SemanticType semanticType() const noexcept { return SemanticType::Site; }

    // --- Property Boundary & Setback Metrics ---
    [[nodiscard]] double propertyArea_mm2() const noexcept {
        return kalara::core::geometry::GeometricOps::polygonArea(propertyBoundary);
    }

    [[nodiscard]] double propertyArea_m2() const noexcept {
        return propertyArea_mm2() / 1000000.0;
    }

    [[nodiscard]] double propertyPerimeter_mm() const noexcept {
        return kalara::core::geometry::GeometricOps::polygonPerimeter(propertyBoundary);
    }

    [[nodiscard]] std::vector<kalara::core::geometry::Point2D> buildableEnvelope() const {
        return setbacks.computeBuildableEnvelope(propertyBoundary);
    }

    [[nodiscard]] double buildableArea_mm2() const noexcept {
        return kalara::core::geometry::GeometricOps::polygonArea(buildableEnvelope());
    }

    [[nodiscard]] bool containsPoint(const kalara::core::geometry::Point2D& pt) const noexcept {
        return kalara::core::geometry::GeometricOps::pointInPolygon(pt, propertyBoundary);
    }

    /// Total building footprint area on the site in mm^2 (Section 7).
    [[nodiscard]] double buildingFootprintArea_mm2() const noexcept {
        double total = 0.0;
        for (const auto& bld : m_buildings) {
            if (bld->levels().empty()) continue;
            const auto& ground = bld->levels().front();
            std::vector<kalara::core::geometry::Point2D> wallPts;
            for (const auto& w : ground->walls()) {
                wallPts.push_back(w->start);
                wallPts.push_back(w->end);
            }
            if (!wallPts.empty()) {
                auto bbox = kalara::core::geometry::GeometricOps::boundingBox(wallPts);
                total += bbox.area();
            }
        }
        return total;
    }

    /// Site coverage percentage: (Building Footprint / Property Area) * 100%
    [[nodiscard]] double siteCoveragePercentage() const noexcept {
        double pArea = propertyArea_mm2();
        if (pArea <= 0.0) return 0.0;
        return (buildingFootprintArea_mm2() / pArea) * 100.0;
    }

    // --- Building Management ---
    Building& addBuilding(std::string buildingName = "Main Building") {
        m_buildings.emplace_back(std::make_unique<Building>(std::move(buildingName)));
        return *m_buildings.back();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Building>>& buildings() const noexcept {
        return m_buildings;
    }

    [[nodiscard]] Building* findBuilding(const EntityId& buildingId) const noexcept {
        for (const auto& bld : m_buildings) {
            if (bld->id == buildingId) return bld.get();
        }
        return nullptr;
    }

    [[nodiscard]] Building* defaultBuilding() const noexcept {
        return m_buildings.empty() ? nullptr : m_buildings.front().get();
    }

    bool removeBuilding(const EntityId& buildingId) {
        for (auto it = m_buildings.begin(); it != m_buildings.end(); ++it) {
            if ((*it)->id == buildingId) {
                m_buildings.erase(it);
                return true;
            }
        }
        return false;
    }

    // --- Road Context (Step 13) ---
    RoadContext& addRoad(std::string roadName, kalara::core::geometry::Segment2D centerline, double width_mm = 10000.0) {
        m_roads.emplace_back(std::make_unique<RoadContext>(std::move(roadName), centerline, width_mm));
        return *m_roads.back();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<RoadContext>>& roads() const noexcept { return m_roads; }

    [[nodiscard]] RoadContext* findRoad(const EntityId& roadId) const noexcept {
        for (const auto& r : m_roads) {
            if (r->id == roadId) return r.get();
        }
        return nullptr;
    }

    // --- Entrances (Step 13) ---
    SiteEntrance& addEntrance(std::string entranceName, SiteEntranceType type,
                              kalara::core::geometry::Point2D pos, double width_mm = 3600.0,
                              kalara::core::geometry::Angle rot = kalara::core::geometry::Angle{}) {
        m_entrances.emplace_back(std::make_unique<SiteEntrance>(std::move(entranceName), type, pos, width_mm, rot));
        return *m_entrances.back();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<SiteEntrance>>& entrances() const noexcept { return m_entrances; }

    [[nodiscard]] SiteEntrance* findEntrance(const EntityId& entId) const noexcept {
        for (const auto& e : m_entrances) {
            if (e->id == entId) return e.get();
        }
        return nullptr;
    }

    // --- Parking (Step 13) ---
    ParkingZone& addParkingZone(std::string zoneName, kalara::core::geometry::Point2D pos, size_t stallCount = 2,
                               kalara::core::geometry::Angle rot = kalara::core::geometry::Angle{},
                               double stallW = 2500.0, double stallL = 5000.0, bool accessible = false) {
        m_parkingZones.emplace_back(std::make_unique<ParkingZone>(std::move(zoneName), pos, stallCount, rot, stallW, stallL, accessible));
        return *m_parkingZones.back();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<ParkingZone>>& parkingZones() const noexcept { return m_parkingZones; }

    [[nodiscard]] ParkingZone* findParkingZone(const EntityId& pId) const noexcept {
        for (const auto& p : m_parkingZones) {
            if (p->id == pId) return p.get();
        }
        return nullptr;
    }

    // --- Landscape Zones (Step 13) ---
    LandscapeZone& addLandscapeZone(std::string zoneName, LandscapeType type,
                                    std::vector<kalara::core::geometry::Point2D> outline) {
        m_landscapeZones.emplace_back(std::make_unique<LandscapeZone>(std::move(zoneName), type, std::move(outline)));
        return *m_landscapeZones.back();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<LandscapeZone>>& landscapeZones() const noexcept { return m_landscapeZones; }

    [[nodiscard]] LandscapeZone* findLandscapeZone(const EntityId& lId) const noexcept {
        for (const auto& l : m_landscapeZones) {
            if (l->id == lId) return l.get();
        }
        return nullptr;
    }

    // --- Pools & Outdoor Elements (Step 13) ---
    OutdoorElement& addOutdoorElement(std::string elemName, OutdoorElementType type,
                                      std::vector<kalara::core::geometry::Point2D> boundary,
                                      double depth_mm = 1500.0, double coping_mm = 400.0) {
        m_outdoorElements.emplace_back(std::make_unique<OutdoorElement>(std::move(elemName), type, std::move(boundary), depth_mm, coping_mm));
        return *m_outdoorElements.back();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<OutdoorElement>>& outdoorElements() const noexcept { return m_outdoorElements; }

    [[nodiscard]] OutdoorElement* findOutdoorElement(const EntityId& oId) const noexcept {
        for (const auto& o : m_outdoorElements) {
            if (o->id == oId) return o.get();
        }
        return nullptr;
    }

    // --- Spot Elevations & Annotations (Step 13) ---
    void addSpotElevation(kalara::core::geometry::Point2D pos, double elev_mm, std::string label = "") {
        spotElevations.push_back(SpotElevation{pos, elev_mm, std::move(label)});
    }

private:
    std::vector<std::unique_ptr<Building>> m_buildings;
    std::vector<std::unique_ptr<RoadContext>> m_roads;
    std::vector<std::unique_ptr<SiteEntrance>> m_entrances;
    std::vector<std::unique_ptr<ParkingZone>> m_parkingZones;
    std::vector<std::unique_ptr<LandscapeZone>> m_landscapeZones;
    std::vector<std::unique_ptr<OutdoorElement>> m_outdoorElements;
};

} // namespace kalara::architecture
