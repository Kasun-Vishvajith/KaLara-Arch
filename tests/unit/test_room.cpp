#include "kalara/architecture/room.hpp"
#include "kalara/architecture/room_boundary_detector.hpp"
#include "kalara/architecture/wall.hpp"
#include "kalara/core/geometry/tolerances.hpp"
#include <iostream>
#include <cassert>

using namespace kalara::architecture;
using namespace kalara::core::geometry;

void testExplicitRoomProperties() {
    std::cout << "Testing Explicit Room Properties & Calculations (Rule 10)..." << std::endl;
    // 5000 mm x 4000 mm room (20 m^2)
    std::vector<Point2D> boundary = {
        Point2D(0.0, 0.0),
        Point2D(5000.0, 0.0),
        Point2D(5000.0, 4000.0),
        Point2D(0.0, 4000.0)
    };

    Room room("Master Bedroom", RoomType::Bedroom, boundary);
    assert(room.id.isValid());
    assert(room.semanticType() == SemanticType::Room);
    assert(room.name == "Master Bedroom");
    assert(room.type == RoomType::Bedroom);
    assert(roomTypeName(room.type) == "Bedroom");

    // Area: 5m * 4m = 20 m^2 = 20,000,000 mm^2
    assert(Tolerances::equalLinear(room.area_mm2(), 20000000.0));
    assert(Tolerances::equalLinear(room.area_m2(), 20.0));

    // Perimeter: 2 * (5m + 4m) = 18 m = 18,000 mm
    assert(Tolerances::equalLinear(room.perimeter_mm(), 18000.0));

    // Dimensions: width = 5000 mm, length = 4000 mm
    assert(Tolerances::equalLinear(room.width_mm(), 5000.0));
    assert(Tolerances::equalLinear(room.length_mm(), 4000.0));

    // Label position at centroid (2500, 2000)
    auto labelPos = room.labelPosition();
    assert(Tolerances::equalLinear(labelPos.x, 2500.0));
    assert(Tolerances::equalLinear(labelPos.y, 2000.0));

    // Containment test
    assert(room.containsPoint(Point2D(2500.0, 2000.0)));
    assert(!room.containsPoint(Point2D(6000.0, 2000.0)));
    std::cout << "-> Explicit room properties passed." << std::endl;
}

void testAutomaticRoomDetection() {
    std::cout << "Testing Automatic Room Boundary Detection from Walls (Rule 10)..." << std::endl;
    // Build 4 connected walls enclosing a 6000 mm x 5000 mm room
    std::vector<std::unique_ptr<Wall>> walls;
    walls.emplace_back(std::make_unique<Wall>(Point2D(0.0, 0.0), Point2D(6000.0, 0.0)));      // South
    walls.emplace_back(std::make_unique<Wall>(Point2D(6000.0, 0.0), Point2D(6000.0, 5000.0))); // East
    walls.emplace_back(std::make_unique<Wall>(Point2D(6000.0, 5000.0), Point2D(0.0, 5000.0))); // North
    walls.emplace_back(std::make_unique<Wall>(Point2D(0.0, 5000.0), Point2D(0.0, 0.0)));      // West

    auto detected = RoomBoundaryDetector::detectRooms(walls, 50.0);
    assert(detected.size() == 1);

    const auto& room = detected.front();
    assert(room.boundary.size() == 4);
    assert(Tolerances::equalLinear(room.area_m2(), 30.0)); // 6m * 5m = 30 m^2
    assert(Tolerances::equalLinear(room.perimeter_mm(), 22000.0));
    assert(room.boundaryWallIds.size() == 4);

    std::cout << "-> Automatic room boundary detection passed." << std::endl;
}

int main() {
    std::cout << "=========================================" << std::endl;
    std::cout << "KaLara Arch Step 06: Rooms & Spaces Test" << std::endl;
    std::cout << "=========================================" << std::endl;

    testExplicitRoomProperties();
    testAutomaticRoomDetection();

    std::cout << "=========================================" << std::endl;
    std::cout << "ALL ROOMS & SPACES TESTS PASSED (100%)" << std::endl;
    std::cout << "=========================================" << std::endl;
    return 0;
}
