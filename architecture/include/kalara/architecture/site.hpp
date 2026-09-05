#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/metadata.hpp"
#include "kalara/architecture/semantics.hpp"
#include "kalara/architecture/building.hpp"
#include "kalara/core/geometry/point_vector.hpp"
#include <string>
#include <vector>
#include <memory>

namespace kalara::architecture {

/// Architectural Site representation (Section 7).
/// Supports property boundary polygon in canonical millimetres.
class Site {
public:
    EntityId id;
    std::string name;
    Metadata metadata;
    std::vector<kalara::core::geometry::Point2D> propertyBoundary;

    Site() : id(EntityId::generate("site")), name("Default Site") {}
    explicit Site(std::string siteName)
        : id(EntityId::generate("site")), name(std::move(siteName)) {}

    [[nodiscard]] SemanticType semanticType() const noexcept { return SemanticType::Site; }

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

    bool removeBuilding(const EntityId& buildingId) {
        for (auto it = m_buildings.begin(); it != m_buildings.end(); ++it) {
            if ((*it)->id == buildingId) {
                m_buildings.erase(it);
                return true;
            }
        }
        return false;
    }

private:
    std::vector<std::unique_ptr<Building>> m_buildings;
};

} // namespace kalara::architecture
