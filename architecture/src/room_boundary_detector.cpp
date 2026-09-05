#include "kalara/architecture/room_boundary_detector.hpp"
#include "kalara/core/geometry/ops.hpp"
#include <map>
#include <set>
#include <cmath>

namespace kalara::architecture {

std::vector<Room> RoomBoundaryDetector::detectRooms(const std::vector<std::unique_ptr<Wall>>& walls, double tolerance_mm) {
    std::vector<Room> discoveredRooms;
    if (walls.size() < 3) return discoveredRooms;

    // Simple cycle finder for rectilinear wall loops (e.g. 4-wall rooms)
    // Find closed chains of wall endpoints
    std::vector<kalara::core::geometry::Point2D> currentLoop;
    std::vector<EntityId> loopWallIds;
    std::set<EntityId> visited;

    // Look for closed loops
    for (size_t i = 0; i < walls.size(); ++i) {
        if (visited.count(walls[i]->id)) continue;

        currentLoop.clear();
        loopWallIds.clear();

        const Wall* current = walls[i].get();
        currentLoop.push_back(current->start);
        loopWallIds.push_back(current->id);
        visited.insert(current->id);

        auto currentEnd = current->end;

        bool closed = false;
        for (size_t step = 0; step < walls.size(); ++step) {
            if (currentEnd.coincidesWith(currentLoop.front(), tolerance_mm)) {
                closed = true;
                break;
            }

            const Wall* nextWall = nullptr;
            for (const auto& w : walls) {
                if (visited.count(w->id)) continue;
                if (w->start.coincidesWith(currentEnd, tolerance_mm)) {
                    nextWall = w.get();
                    currentLoop.push_back(w->start);
                    currentEnd = w->end;
                    break;
                } else if (w->end.coincidesWith(currentEnd, tolerance_mm)) {
                    nextWall = w.get();
                    currentLoop.push_back(w->end);
                    currentEnd = w->start;
                    break;
                }
            }

            if (!nextWall) break;
            visited.insert(nextWall->id);
            loopWallIds.push_back(nextWall->id);
        }

        if (closed && currentLoop.size() >= 3) {
            double area = kalara::core::geometry::GeometricOps::polygonArea(currentLoop);
            if (area > 100000.0) { // Min 0.1 m^2 to ignore degenerate slivers
                Room room("Detected Room", RoomType::LivingRoom, currentLoop);
                room.boundaryWallIds = loopWallIds;
                discoveredRooms.push_back(std::move(room));
            }
        }
    }

    return discoveredRooms;
}

} // namespace kalara::architecture
