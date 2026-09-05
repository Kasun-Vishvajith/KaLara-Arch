#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/metadata.hpp"
#include "kalara/architecture/semantics.hpp"
#include "kalara/architecture/level.hpp"
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace kalara::architecture {

/// Architectural Building entity.
/// Manages a collection of ordered levels.
class Building {
public:
    EntityId id;
    std::string name;
    Metadata metadata;

    Building() : id(EntityId::generate("bld")), name("Main Building") {}
    explicit Building(std::string buildingName)
        : id(EntityId::generate("bld")), name(std::move(buildingName)) {}

    [[nodiscard]] SemanticType semanticType() const noexcept { return SemanticType::Building; }

    Level& addLevel(std::string levelName, double elevation_mm, double height_mm = 3000.0) {
        m_levels.emplace_back(std::make_unique<Level>(std::move(levelName), elevation_mm, height_mm));
        return *m_levels.back();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Level>>& levels() const noexcept {
        return m_levels;
    }

    [[nodiscard]] Level* findLevel(const EntityId& levelId) const noexcept {
        for (const auto& lvl : m_levels) {
            if (lvl->id == levelId) return lvl.get();
        }
        return nullptr;
    }

    bool removeLevel(const EntityId& levelId) {
        for (auto it = m_levels.begin(); it != m_levels.end(); ++it) {
            if ((*it)->id == levelId) {
                m_levels.erase(it);
                return true;
            }
        }
        return false;
    }

private:
    std::vector<std::unique_ptr<Level>> m_levels;
};

} // namespace kalara::architecture
