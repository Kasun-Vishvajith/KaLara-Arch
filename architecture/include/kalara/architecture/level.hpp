#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/metadata.hpp"
#include "kalara/architecture/semantics.hpp"
#include "kalara/architecture/wall.hpp"
#include <string>
#include <vector>
#include <memory>

namespace kalara::architecture {

/// Architectural Level / Story (Principle 5).
/// Stores walls, elevation, and geometric elements.
class Level {
public:
    EntityId id;
    std::string name;
    double elevation_mm = 0.0;
    double height_mm = 3000.0;
    Metadata metadata;

    Level() : id(EntityId::generate("lvl")), name("Ground Floor") {}
    Level(std::string levelName, double elevation, double height = 3000.0)
        : id(EntityId::generate("lvl")),
          name(std::move(levelName)),
          elevation_mm(elevation),
          height_mm(height) {}

    [[nodiscard]] SemanticType semanticType() const noexcept { return SemanticType::Level; }

    Wall& addWall(kalara::core::geometry::Point2D start, kalara::core::geometry::Point2D end, double thickness = 200.0) {
        m_walls.emplace_back(std::make_unique<Wall>(start, end, thickness, height_mm));
        return *m_walls.back();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Wall>>& walls() const noexcept {
        return m_walls;
    }

    [[nodiscard]] Wall* findWall(const EntityId& wallId) const noexcept {
        for (const auto& w : m_walls) {
            if (w->id == wallId) return w.get();
        }
        return nullptr;
    }

    bool removeWall(const EntityId& wallId) {
        for (auto it = m_walls.begin(); it != m_walls.end(); ++it) {
            if ((*it)->id == wallId) {
                m_walls.erase(it);
                return true;
            }
        }
        return false;
    }

private:
    std::vector<std::unique_ptr<Wall>> m_walls;
};

} // namespace kalara::architecture
