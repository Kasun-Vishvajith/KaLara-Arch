#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/metadata.hpp"
#include "kalara/architecture/semantics.hpp"
#include "kalara/architecture/wall.hpp"
#include "kalara/architecture/room.hpp"
#include "kalara/architecture/opening.hpp"
#include "kalara/architecture/room_boundary_detector.hpp"
#include <string>
#include <vector>
#include <memory>

namespace kalara::architecture {

/// Architectural Level / Story (Principle 5).
/// Stores walls, rooms, doors, windows, elevation, and geometric elements.
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

    // --- Wall Management ---
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
        // Also remove hosted openings on this wall
        removeOpeningsForWall(wallId);

        for (auto it = m_walls.begin(); it != m_walls.end(); ++it) {
            if ((*it)->id == wallId) {
                m_walls.erase(it);
                return true;
            }
        }
        return false;
    }

    // --- Room Management ---
    Room& addRoom(std::string roomName, RoomType type, std::vector<kalara::core::geometry::Point2D> boundary) {
        m_rooms.emplace_back(std::make_unique<Room>(std::move(roomName), type, std::move(boundary)));
        return *m_rooms.back();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Room>>& rooms() const noexcept {
        return m_rooms;
    }

    [[nodiscard]] Room* findRoom(const EntityId& roomId) const noexcept {
        for (const auto& r : m_rooms) {
            if (r->id == roomId) return r.get();
        }
        return nullptr;
    }

    bool removeRoom(const EntityId& roomId) {
        for (auto it = m_rooms.begin(); it != m_rooms.end(); ++it) {
            if ((*it)->id == roomId) {
                m_rooms.erase(it);
                return true;
            }
        }
        return false;
    }

    size_t autoDetectRooms(double tolerance_mm = 50.0) {
        auto detected = RoomBoundaryDetector::detectRooms(m_walls, tolerance_mm);
        size_t addedCount = detected.size();
        for (auto& r : detected) {
            m_rooms.emplace_back(std::make_unique<Room>(std::move(r)));
        }
        return addedCount;
    }

    // --- Door & Window Management (hosted by walls) ---
    Door& addDoor(const EntityId& hostWallId, double offsetAlongWall_mm = 1000.0, double width_mm = 900.0, double height_mm = 2100.0, DoorSwing swing = DoorSwing::LeftInswing) {
        m_doors.emplace_back(std::make_unique<Door>(hostWallId, offsetAlongWall_mm, width_mm, height_mm, swing));
        return *m_doors.back();
    }

    Window& addWindow(const EntityId& hostWallId, double offsetAlongWall_mm = 1500.0, double width_mm = 1200.0, double height_mm = 1500.0, double sill_mm = 900.0, WindowType type = WindowType::Casement) {
        m_windows.emplace_back(std::make_unique<Window>(hostWallId, offsetAlongWall_mm, width_mm, height_mm, sill_mm, type));
        return *m_windows.back();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Door>>& doors() const noexcept {
        return m_doors;
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Window>>& windows() const noexcept {
        return m_windows;
    }

    [[nodiscard]] Door* findDoor(const EntityId& doorId) const noexcept {
        for (const auto& d : m_doors) {
            if (d->id == doorId) return d.get();
        }
        return nullptr;
    }

    [[nodiscard]] Window* findWindow(const EntityId& windowId) const noexcept {
        for (const auto& w : m_windows) {
            if (w->id == windowId) return w.get();
        }
        return nullptr;
    }

    bool removeDoor(const EntityId& doorId) {
        for (auto it = m_doors.begin(); it != m_doors.end(); ++it) {
            if ((*it)->id == doorId) {
                m_doors.erase(it);
                return true;
            }
        }
        return false;
    }

    bool removeWindow(const EntityId& windowId) {
        for (auto it = m_windows.begin(); it != m_windows.end(); ++it) {
            if ((*it)->id == windowId) {
                m_windows.erase(it);
                return true;
            }
        }
        return false;
    }

    void removeOpeningsForWall(const EntityId& wallId) {
        std::erase_if(m_doors, [&](const auto& d) { return d->hostWallId == wallId; });
        std::erase_if(m_windows, [&](const auto& w) { return w->hostWallId == wallId; });
    }

private:
    std::vector<std::unique_ptr<Wall>> m_walls;
    std::vector<std::unique_ptr<Room>> m_rooms;
    std::vector<std::unique_ptr<Door>> m_doors;
    std::vector<std::unique_ptr<Window>> m_windows;
};

} // namespace kalara::architecture
