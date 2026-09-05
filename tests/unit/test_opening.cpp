#include "kalara/architecture/opening.hpp"
#include "kalara/architecture/wall.hpp"
#include "kalara/core/geometry/tolerances.hpp"
#include <iostream>
#include <cassert>

using namespace kalara::architecture;
using namespace kalara::core::geometry;

void testDoorPropertiesAndGeometry() {
    std::cout << "Testing Door Properties & Wall-Host Relationship (Step 07)..." << std::endl;
    // Host wall: 5000 mm long, 200 mm thick, from (0, 0) to (5000, 0)
    Wall hostWall(Point2D(0.0, 0.0), Point2D(5000.0, 0.0), 200.0);

    // Door placed at offset 2000 mm, width 900 mm, height 2100 mm
    Door door(hostWall.id, 2000.0, 900.0, 2100.0, DoorSwing::LeftInswing);

    assert(door.id.isValid());
    assert(door.semanticType() == SemanticType::Door);
    assert(door.hostWallId == hostWall.id);
    assert(door.offsetAlongWall_mm == 2000.0);
    assert(door.width_mm == 900.0);
    assert(door.height_mm == 2100.0);
    assert(door.sillHeight_mm == 0.0);

    // Center point along host wall: (2000, 0)
    Point2D center = door.centerPoint(hostWall);
    assert(Tolerances::equalLinear(center.x, 2000.0));
    assert(Tolerances::equalLinear(center.y, 0.0));

    // Opening segment along wall centerline: from 2000 - 450 = 1550 to 2000 + 450 = 2450
    Segment2D seg = door.openingSegment(hostWall);
    assert(Tolerances::equalLinear(seg.start.x, 1550.0));
    assert(Tolerances::equalLinear(seg.end.x, 2450.0));
    assert(Tolerances::equalLinear(seg.length(), 900.0));

    // Opening cutout box (4 corners through wall thickness +/- 100mm)
    auto box = door.openingBox(hostWall);
    assert(box.size() == 4);
    assert(Tolerances::equalLinear(box[0].x, 1550.0) && Tolerances::equalLinear(box[0].y, 100.0));
    assert(Tolerances::equalLinear(box[1].x, 2450.0) && Tolerances::equalLinear(box[1].y, 100.0));
    assert(Tolerances::equalLinear(box[2].x, 2450.0) && Tolerances::equalLinear(box[2].y, -100.0));
    assert(Tolerances::equalLinear(box[3].x, 1550.0) && Tolerances::equalLinear(box[3].y, -100.0));

    // Hit testing
    assert(door.containsPoint(Point2D(2000.0, 0.0), hostWall));
    assert(door.containsPoint(Point2D(1600.0, 50.0), hostWall));
    assert(!door.containsPoint(Point2D(1000.0, 0.0), hostWall)); // Outside opening width
    assert(!door.containsPoint(Point2D(2000.0, 150.0), hostWall)); // Outside wall thickness

    // Hinge point for LeftInswing
    Point2D hinge = door.hingePoint(hostWall);
    assert(Tolerances::equalLinear(hinge.x, 1550.0));
    assert(Tolerances::equalLinear(hinge.y, 0.0));

    std::cout << "-> Door properties & geometry passed." << std::endl;
}

void testWindowPropertiesAndGeometry() {
    std::cout << "Testing Window Properties & Wall-Host Relationship (Step 07)..." << std::endl;
    // Host wall: vertical wall from (0, 0) to (0, 6000), 250 mm thick
    Wall hostWall(Point2D(0.0, 0.0), Point2D(0.0, 6000.0), 250.0);

    // Window placed at offset 3000 mm, width 1200 mm, height 1500 mm, sill 900 mm
    Window win(hostWall.id, 3000.0, 1200.0, 1500.0, 900.0, WindowType::Casement);

    assert(win.id.isValid());
    assert(win.semanticType() == SemanticType::Window);
    assert(win.hostWallId == hostWall.id);
    assert(win.offsetAlongWall_mm == 3000.0);
    assert(win.width_mm == 1200.0);
    assert(win.height_mm == 1500.0);
    assert(win.sillHeight_mm == 900.0);

    // Center point along host wall: (0, 3000)
    Point2D center = win.centerPoint(hostWall);
    assert(Tolerances::equalLinear(center.x, 0.0));
    assert(Tolerances::equalLinear(center.y, 3000.0));

    // Opening segment along vertical wall: y from 3000 - 600 = 2400 to 3000 + 600 = 3600
    Segment2D seg = win.openingSegment(hostWall);
    assert(Tolerances::equalLinear(seg.start.y, 2400.0));
    assert(Tolerances::equalLinear(seg.end.y, 3600.0));
    assert(Tolerances::equalLinear(seg.length(), 1200.0));

    // Hit testing
    assert(win.containsPoint(Point2D(0.0, 3000.0), hostWall));
    assert(win.containsPoint(Point2D(50.0, 2800.0), hostWall));
    assert(!win.containsPoint(Point2D(0.0, 1500.0), hostWall));

    std::cout << "-> Window properties & geometry passed." << std::endl;
}

int main() {
    std::cout << "=========================================" << std::endl;
    std::cout << "KaLara Arch Step 07: Doors & Windows Test" << std::endl;
    std::cout << "=========================================" << std::endl;

    testDoorPropertiesAndGeometry();
    testWindowPropertiesAndGeometry();

    std::cout << "=========================================" << std::endl;
    std::cout << "ALL DOORS & WINDOWS TESTS PASSED (100%)" << std::endl;
    std::cout << "=========================================" << std::endl;
    return 0;
}
