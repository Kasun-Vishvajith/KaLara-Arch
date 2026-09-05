#pragma once
#include "kalara/architecture/room.hpp"
#include "kalara/architecture/wall.hpp"
#include <vector>
#include <memory>

namespace kalara::architecture {

/// Helper that detects closed planar cycles from connected walls to derive rooms (Rule 10).
class RoomBoundaryDetector {
public:
    /// Detect closed room cycles from a collection of connected walls.
    static std::vector<Room> detectRooms(const std::vector<std::unique_ptr<Wall>>& walls, double tolerance_mm = 50.0);
};

} // namespace kalara::architecture
