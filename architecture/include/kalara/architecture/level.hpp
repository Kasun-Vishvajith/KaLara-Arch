#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/metadata.hpp"
#include "kalara/architecture/semantics.hpp"
#include "kalara/architecture/wall.hpp"
#include "kalara/architecture/room.hpp"
#include "kalara/architecture/opening.hpp"
#include "kalara/architecture/annotation.hpp"
#include "kalara/architecture/library_item.hpp"
#include "kalara/architecture/roof.hpp"
#include "kalara/architecture/constraint.hpp"
#include "kalara/architecture/room_boundary_detector.hpp"
#include <string>
#include <vector>
#include <memory>

namespace kalara::architecture {

/// Classification of architectural building levels.
enum class LevelType {
    Basement,
    GroundFloor,
    UpperFloor,
    Mezzanine,
    Attic,
    Roof
};

constexpr std::string_view levelTypeName(LevelType type) noexcept {
    switch (type) {
        case LevelType::Basement:    return "Basement";
        case LevelType::GroundFloor: return "Ground Floor";
        case LevelType::UpperFloor:  return "Upper Floor";
        case LevelType::Mezzanine:   return "Mezzanine";
        case LevelType::Attic:       return "Attic";
        case LevelType::Roof:        return "Roof";
    }
    return "Floor";
}

/// Architectural Level / Story (Principle 5 & Step 12).
/// Stores walls, rooms, openings, dimensions, annotations, and roofs with real elevation data.
class Level {
public:
    EntityId id;
    std::string name;
    LevelType type = LevelType::GroundFloor;
    double elevation_mm = 0.0;
    double height_mm = 3000.0;
    bool visible = true;
    bool locked = false;
    std::optional<EntityId> underlayLevelId; // Cross-level reference (ghost floor underneath)
    double underlayOpacity = 0.4;
    Metadata metadata;

    Level() : id(EntityId::generate("lvl")), name("Ground Floor") {}
    Level(std::string levelName, double elevation, double height = 3000.0, LevelType lvlType = LevelType::GroundFloor)
        : id(EntityId::generate("lvl")),
          name(std::move(levelName)),
          type(lvlType),
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

    bool removeDimension(const EntityId& dimId) {
        for (auto it = m_dimensions.begin(); it != m_dimensions.end(); ++it) {
            if ((*it)->id == dimId) {
                m_dimensions.erase(it);
                return true;
            }
        }
        return false;
    }

    NoteAnnotation& addNote(kalara::core::geometry::Point2D pos, std::string text) {
        m_notes.emplace_back(std::make_unique<NoteAnnotation>(pos, std::move(text)));
        return *m_notes.back();
    }

    bool removeNote(const EntityId& noteId) {
        for (auto it = m_notes.begin(); it != m_notes.end(); ++it) {
            if ((*it)->id == noteId) {
                m_notes.erase(it);
                return true;
            }
        }
        return false;
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

    // --- Roof Management (Step 12) ---
    Roof& addRoof(std::string roofName, RoofType type, std::vector<kalara::core::geometry::Point2D> boundary,
                  double pitch_deg = 22.5, double overhang_mm = 600.0) {
        m_roofs.emplace_back(std::make_unique<Roof>(std::move(roofName), type, std::move(boundary), pitch_deg, overhang_mm));
        return *m_roofs.back();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Roof>>& roofs() const noexcept {
        return m_roofs;
    }

    [[nodiscard]] Roof* findRoof(const EntityId& roofId) const noexcept {
        for (const auto& r : m_roofs) {
            if (r->id == roofId) return r.get();
        }
        return nullptr;
    }

    bool removeRoof(const EntityId& roofId) {
        for (auto it = m_roofs.begin(); it != m_roofs.end(); ++it) {
            if ((*it)->id == roofId) {
                m_roofs.erase(it);
                return true;
            }
        }
        return false;
    }

    // --- Constraint Management (Step 09 & Step 15) ---
    Constraint& addConstraint(std::string constraintName, ConstraintType type, ConstraintSeverity severity,
                              std::vector<EntityId> targets, double value = 0.0) {
        m_constraints.emplace_back(std::make_unique<Constraint>(std::move(constraintName), type, severity, std::move(targets), value));
        return *m_constraints.back();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Constraint>>& constraints() const noexcept {
        return m_constraints;
    }

    [[nodiscard]] Constraint* findConstraint(const EntityId& constraintId) const noexcept {
        for (const auto& c : m_constraints) {
            if (c->id == constraintId) return c.get();
        }
        return nullptr;
    }

    bool removeConstraint(const EntityId& constraintId) {
        for (auto it = m_constraints.begin(); it != m_constraints.end(); ++it) {
            if ((*it)->id == constraintId) {
                m_constraints.erase(it);
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
    std::vector<std::unique_ptr<Roof>> m_roofs;
    std::vector<std::unique_ptr<Constraint>> m_constraints;
};

} // namespace kalara::architecture
