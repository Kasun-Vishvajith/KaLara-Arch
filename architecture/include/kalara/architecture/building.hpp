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

    Level& addLevel(std::string levelName, double elevation_mm, double height_mm = 3000.0, LevelType type = LevelType::GroundFloor) {
        auto lvl = std::make_unique<Level>(std::move(levelName), elevation_mm, height_mm, type);
        EntityId newId = lvl->id;
        m_levels.emplace_back(std::move(lvl));
        if (!m_activeLevelId.has_value()) {
            m_activeLevelId = newId;
        }
        sortLevels();
        return *findLevel(newId);
    }

    /// Add roof level on top of the highest existing level (Step 12).
    Level& addRoofLevel(std::string roofLevelName = "Roof", double height_mm = 2400.0) {
        double topElevation = 0.0;
        if (!m_levels.empty()) {
            sortLevels();
            topElevation = m_levels.back()->elevation_mm + m_levels.back()->height_mm;
        }
        return addLevel(std::move(roofLevelName), topElevation, height_mm, LevelType::Roof);
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

    [[nodiscard]] Level* activeLevel() const noexcept {
        if (m_activeLevelId.has_value()) {
            return findLevel(m_activeLevelId.value());
        }
        return m_levels.empty() ? nullptr : m_levels.front().get();
    }

    [[nodiscard]] Level* defaultLevel() const noexcept {
        return activeLevel();
    }

    void setActiveLevel(const EntityId& levelId) noexcept {
        if (findLevel(levelId)) {
            m_activeLevelId = levelId;
        }
    }

    /// Duplicate an existing level layout upwards as a new floor (Step 12).
    Level* duplicateLevelUpwards(const EntityId& sourceLevelId, std::string newLevelName) {
        auto* src = findLevel(sourceLevelId);
        if (!src) return nullptr;

        double newElevation = src->elevation_mm + src->height_mm;
        auto& newLvl = addLevel(std::move(newLevelName), newElevation, src->height_mm, LevelType::UpperFloor);

        // Copy walls
        for (const auto& w : src->walls()) {
            newLvl.addWall(w->start, w->end, w->thickness_mm);
        }

        // Set underlay reference to source floor for alignment
        newLvl.underlayLevelId = src->id;

        return &newLvl;
    }

    /// Sort levels by elevation in ascending order.
    void sortLevels() {
        std::sort(m_levels.begin(), m_levels.end(), [](const auto& a, const auto& b) {
            return a->elevation_mm < b->elevation_mm;
        });
    }

    bool removeLevel(const EntityId& levelId) {
        if (m_levels.size() <= 1) {
            // Keep at least one level
            return false;
        }

        for (auto it = m_levels.begin(); it != m_levels.end(); ++it) {
            if ((*it)->id == levelId) {
                if (m_activeLevelId.has_value() && m_activeLevelId.value() == levelId) {
                    m_activeLevelId.reset();
                }
                m_levels.erase(it);
                if (!m_activeLevelId.has_value() && !m_levels.empty()) {
                    m_activeLevelId = m_levels.front()->id;
                }
                return true;
            }
        }
        return false;
    }

private:
    std::vector<std::unique_ptr<Level>> m_levels;
    std::optional<EntityId> m_activeLevelId;
};

} // namespace kalara::architecture
