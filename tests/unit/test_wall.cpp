#include "kalara/architecture/wall.hpp"
#include "kalara/architecture/wall_join.hpp"
#include "kalara/core/geometry/tolerances.hpp"
#include <iostream>
#include <cassert>

using namespace kalara::architecture;
using namespace kalara::core::geometry;

void testWallGeometryAndBoundary() {
    std::cout << "Testing Wall Geometry & Boundary Polygon..." << std::endl;
    // Horizontal wall from (0, 0) to (4000, 0) mm with thickness 200 mm
    Wall w1(Point2D(0.0, 0.0), Point2D(4000.0, 0.0), 200.0);

    assert(w1.id.isValid());
    assert(w1.semanticType() == SemanticType::Wall);
    assert(Tolerances::equalLinear(w1.length_mm(), 4000.0));
    assert(Tolerances::equalLinear(w1.thickness_mm, 200.0));

    // Normal vector points North (0, 1)
    auto n = w1.normal();
    assert(Tolerances::equalLinear(n.dx, 0.0));
    assert(Tolerances::equalLinear(n.dy, 1.0));

    // Boundary corners: Left=+100, Right=-100
    auto corners = w1.boundaryPolygon();
    assert(corners.size() == 4);
    assert(corners[0] == Point2D(0.0, 100.0));    // StartLeft
    assert(corners[1] == Point2D(4000.0, 100.0)); // EndLeft
    assert(corners[2] == Point2D(4000.0, -100.0)); // EndRight
    assert(corners[3] == Point2D(0.0, -100.0));  // StartRight

    // Hit-testing
    assert(w1.containsPoint(Point2D(2000.0, 0.0)));
    assert(w1.containsPoint(Point2D(2000.0, 50.0)));
    assert(w1.containsPoint(Point2D(2000.0, -50.0)));
    assert(!w1.containsPoint(Point2D(2000.0, 150.0)));
    assert(!w1.containsPoint(Point2D(5000.0, 0.0)));
    std::cout << "-> Wall Geometry & Boundary passed." << std::endl;
}

void testWallThicknessPreferences() {
    std::cout << "Testing Preference-Controlled Thickness Alignment (Rule 6)..." << std::endl;
    // Base wall: centerline y = 0, thickness 200 mm (y from -100 to +100)
    Wall wCenter(Point2D(0.0, 0.0), Point2D(4000.0, 0.0), 200.0);
    // Expand to 300 mm preserving Centerline
    wCenter.setThickness(300.0, WallThicknessAlignment::Centerline);
    assert(Tolerances::equalLinear(wCenter.start.y, 0.0));
    assert(Tolerances::equalLinear(wCenter.end.y, 0.0));
    assert(Tolerances::equalLinear(wCenter.thickness_mm, 300.0));
    auto cCenter = wCenter.boundaryPolygon();
    assert(Tolerances::equalLinear(cCenter[0].y, 150.0));
    assert(Tolerances::equalLinear(cCenter[3].y, -150.0));

    // Expand to 300 mm preserving Outer Face (North edge y = +100 must stay +100)
    Wall wOuter(Point2D(0.0, 0.0), Point2D(4000.0, 0.0), 200.0);
    wOuter.setThickness(300.0, WallThicknessAlignment::PreserveOuterFace);
    // Centerline moves down by 50 mm to y = -50
    assert(Tolerances::equalLinear(wOuter.start.y, -50.0));
    assert(Tolerances::equalLinear(wOuter.end.y, -50.0));
    auto cOuter = wOuter.boundaryPolygon();
    assert(Tolerances::equalLinear(cOuter[0].y, 100.0));  // Outer face preserved!
    assert(Tolerances::equalLinear(cOuter[3].y, -200.0));

    // Expand to 300 mm preserving Inner Face (South edge y = -100 must stay -100)
    Wall wInner(Point2D(0.0, 0.0), Point2D(4000.0, 0.0), 200.0);
    wInner.setThickness(300.0, WallThicknessAlignment::PreserveInnerFace);
    // Centerline moves up by 50 mm to y = +50
    assert(Tolerances::equalLinear(wInner.start.y, 50.0));
    assert(Tolerances::equalLinear(wInner.end.y, 50.0));
    auto cInner = wInner.boundaryPolygon();
    assert(Tolerances::equalLinear(cInner[0].y, 200.0));
    assert(Tolerances::equalLinear(cInner[3].y, -100.0)); // Inner face preserved!

    std::cout << "-> Preference-controlled thickness passed." << std::endl;
}

void testWallJoinsAndSnapping() {
    std::cout << "Testing Wall Joins & Connected Endpoints..." << std::endl;
    // L-Corner: Wall 1 (South) meets Wall 2 (East) at (4000, 0)
    Wall wSouth(Point2D(0.0, 0.0), Point2D(4000.0, 0.0), 200.0);
    Wall wEast(Point2D(4000.0, 0.0), Point2D(4000.0, 3000.0), 200.0);

    auto joinL = WallJoinSolver::evaluateJoin(wSouth, wEast);
    assert(joinL.has_value());
    assert(joinL->type == JoinType::L_Corner);
    assert(joinL->junctionPoint == Point2D(4000.0, 0.0));

    // T-Junction: Wall 3 ends on Wall 1 at (2000, 0)
    Wall wInterior(Point2D(2000.0, -2000.0), Point2D(2000.0, 0.0), 150.0);
    auto joinT = WallJoinSolver::evaluateJoin(wInterior, wSouth);
    assert(joinT.has_value());
    assert(joinT->type == JoinType::T_Junction);
    assert(joinT->junctionPoint == Point2D(2000.0, 0.0));

    // Endpoint snapping test: slightly misaligned by 10 mm
    Wall wMisaligned(Point2D(4010.0, 0.0), Point2D(4010.0, 3000.0), 200.0);
    bool snapped = WallJoinSolver::snapEndpoints(wMisaligned, wSouth, 20.0);
    assert(snapped);
    assert(wMisaligned.start == wSouth.end);

    std::cout << "-> Wall joins & snapping passed." << std::endl;
}

int main() {
    std::cout << "=========================================" << std::endl;
    std::cout << "KaLara Arch Step 05: Wall System Test" << std::endl;
    std::cout << "=========================================" << std::endl;

    testWallGeometryAndBoundary();
    testWallThicknessPreferences();
    testWallJoinsAndSnapping();

    std::cout << "=========================================" << std::endl;
    std::cout << "ALL WALL SYSTEM TESTS PASSED (100%)" << std::endl;
    std::cout << "=========================================" << std::endl;
    return 0;
}
