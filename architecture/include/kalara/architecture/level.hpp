#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/metadata.hpp"
#include "kalara/architecture/semantics.hpp"
#include "kalara/architecture/wall.hpp"
#include "kalara/architecture/room.hpp"
#include "kalara/architecture/opening.hpp"
#include "kalara/architecture/annotation.hpp"
#include "kalara/architecture/library_item.hpp"
#include "kalara/architecture/room_boundary_detector.hpp"
#include <string>
#include <vector>
#include <memory>

namespace kalara::architecture {

/// Architectural Level / Story (Principle 5).
/// Stores walls, rooms, openings, dimensions, and annotations.
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
        removeOpeningsForWall(wallId);
        removeDimensionsForWall(wallId);

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

    // --- Door & Window Management ---
    Door& addDoor(const EntityId& hostWallId, double offsetAlongWall_mm = 1000.0, double width_mm = 900.0, double height_mm = 2100.0, DoorSwing swing = DoorSwing::LeftInswing) {
        m_doors.emplace_back(std::make_unique<Door>(hostWallId, offsetAlongWall_mm, width_mm, height_mm, swing));
        return *m_doors.back();
    }

    Window& addWindow(const EntityId& hostWallId, double offsetAlongWall_mm = 1500.0, double width_mm = 1200.0, double height_mm = 1500.0, double sill_mm = 900.0, WindowType type = WindowType::Casement) {
        m_windows.emplace_back(std::make_unique<Window>(hostWallId, offsetAlongWall_mm, width_mm, height_mm, sill_mm, type));
        return *m_windows.back();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Door>>& doors() const noexcept { return m_doors; }
    [[nodiscard]] const std::vector<std::unique_ptr<Window>>& windows() const noexcept { return m_windows; }

    void removeOpeningsForWall(const EntityId& wallId) {
        std::erase_if(m_doors, [&](const auto& d) { return d->hostWallId == wallId; });
        std::erase_if(m_windows, [&](const auto& w) { return w->hostWallId == wallId; });
    }

    // --- Dimension & Annotation Management (Step 08) ---
    Dimension& addDimension(kalara::core::geometry::Point2D p1, kalara::core::geometry::Point2D p2, double offset = 500.0) {
        m_dimensions.emplace_back(std::make_unique<Dimension>(p1, p2, offset));
        return *m_dimensions.back();
    }

    Dimension& addDimensionForWall(const Wall& wall, double offset = 500.0) {
        m_dimensions.emplace_back(std::make_unique<Dimension>(wall.id, wall, offset));
        return *m_dimensions.back();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Dimension>>& dimensions() const noexcept {
        return m_dimensions;
    }

    void syncDimensions() {
        for (auto& dim : m_dimensions) {
            if (dim->referencedEntityId.has_value()) {
                auto* wall = findWall(dim->referencedEntityId.value());
                if (wall) {
                    dim->syncWithReferencedWall(*wall);
                }
            }
        }
    }

    void removeDimensionsForWall(const EntityId& wallId) {
        std::erase_if(m_dimensions, [&](const auto& dim) {
            return dim->referencedEntityId.has_value() && dim->referencedEntityId.value() == wallId;
        });
    }

    NoteAnnotation& addNote(kalara::core::geometry::Point2D pos, std::string text) {
        m_notes.emplace_back(std::make_unique<NoteAnnotation>(pos, std::move(text)));
        return *m_notes.back();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<NoteAnnotation>>& notes() const noexcept {
        return m_notes;
    }

    // --- Library Instances Management (Step 11) ---
    LibraryInstance& addLibraryInstance(const LibraryItem& item, kalara::core::geometry::Point2D pos,
                                        kalara::core::geometry::Angle rot = kalara::core::geometry::Angle{}) {
        m_libraryInstances.emplace_back(std::make_unique<LibraryInstance>(item, pos, rot));
        return *m_libraryInstances.back();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<LibraryInstance>>& libraryInstances() const noexcept {
        return m_libraryInstances;
    }

    [[nodiscard]] LibraryInstance* findLibraryInstance(const EntityId& instId) const noexcept {
        for (const auto& inst : m_libraryInstances) {
            if (inst->id == instId) return inst.get();
        }
        return nullptr;
    }

    bool removeLibraryInstance(const EntityId& instId) {
        for (auto it = m_libraryInstances.begin(); it != m_libraryInstances.end(); ++it) {
            if ((*it)->id == instId) {
                m_libraryInstances.erase(it);
                return true;
            }
        }
        return false;
    }

private:
    std::vector<std::unique_ptr<Wall>> m_walls;
    std::vector<std::unique_ptr<Room>> m_rooms;
    std::vector<std::unique_ptr<Door>> m_doors;
    std::vector<std::unique_ptr<Window>> m_windows;
    std::vector<std::unique_ptr<Dimension>> m_dimensions;
    std::vector<std::unique_ptr<NoteAnnotation>> m_notes;
    std::vector<std::unique_ptr<LibraryInstance>> m_libraryInstances;
};

} // namespace kalara::architecture
