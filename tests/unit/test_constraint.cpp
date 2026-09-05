#include "kalara/architecture/constraint.hpp"
#include "kalara/architecture/constraint_solver.hpp"
#include "kalara/architecture/level.hpp"
#include "kalara/architecture/wall.hpp"
#include "kalara/architecture/opening.hpp"
#include "kalara/core/geometry/tolerances.hpp"
#include <iostream>
#include <cassert>

using namespace kalara::architecture;
using namespace kalara::core::geometry;

void testConstraintDefinitionsAndSeverity() {
    std::cout << "Testing Constraint Classification & Severities (Rule 9)..." << std::endl;
    EntityId w1 = EntityId::generate("wall");
    EntityId w2 = EntityId::generate("wall");

    Constraint hardPerp("Wall Perpendicularity", ConstraintType::Perpendicularity, ConstraintSeverity::Hard, {w1, w2});
    assert(hardPerp.id.isValid());
    assert(hardPerp.severity == ConstraintSeverity::Hard);
    assert(constraintSeverityName(hardPerp.severity) == "HARD");
    assert(constraintTypeName(hardPerp.type) == "Perpendicularity");

    Constraint softClearance("Bedroom Garden Distance", ConstraintType::MinimumDistance, ConstraintSeverity::Soft, {w1, w2}, 3000.0);
    assert(softClearance.severity == ConstraintSeverity::Soft);
    assert(constraintSeverityName(softClearance.severity) == "SOFT");
    assert(softClearance.targetValue == 3000.0);
    std::cout << "-> Constraint definitions passed." << std::endl;
}

void testConnectedGeometryPropagation() {
    std::cout << "Testing Preference-Controlled Connected Geometry Propagation (Rule 6)..." << std::endl;
    Level level("Test Level", 0.0);
    // Wall 1: (0, 0) to (4000, 0)
    Wall& w1 = level.addWall(Point2D(0.0, 0.0), Point2D(4000.0, 0.0), 200.0);
    // Wall 2: (4000, 0) to (4000, 3000) meeting w1 at (4000, 0)
    Wall& w2 = level.addWall(Point2D(4000.0, 0.0), Point2D(4000.0, 3000.0), 200.0);

    // Add associated dimension to w1
    auto& dim1 = level.addDimensionForWall(w1);
    assert(Tolerances::equalLinear(dim1.measuredDistance_mm(), 4000.0));

    // Case 1: Move endpoint with PropagateConnected (corner moves from 4000,0 to 5000,0)
    size_t propagated = ConstraintSolver::moveWallEndpoint(
        w1, false, Point2D(5000.0, 0.0), level, ConnectedGeometryPropagation::PropagateConnected
    );
    assert(propagated == 2); // Both w1.end and w2.start moved
    assert(Tolerances::equalLinear(w1.end.x, 5000.0));
    assert(Tolerances::equalLinear(w2.start.x, 5000.0));
    assert(Tolerances::equalLinear(dim1.measuredDistance_mm(), 5000.0)); // Dimension auto-synced!

    // Case 2: Move endpoint with PreserveSelectedOnly (moves w1.end back to 4000,0 without moving w2.start)
    size_t single = ConstraintSolver::moveWallEndpoint(
        w1, false, Point2D(4000.0, 0.0), level, ConnectedGeometryPropagation::PreserveSelectedOnly
    );
    assert(single == 1);
    assert(Tolerances::equalLinear(w1.end.x, 4000.0));
    assert(Tolerances::equalLinear(w2.start.x, 5000.0)); // w2 remained at 5000!

    std::cout << "-> Connected geometry propagation passed." << std::endl;
}

void testConflictReporting() {
    std::cout << "Testing Conflict Reporting (Step 09)..." << std::endl;
    Level level("Conflict Test Level", 0.0);
    Wall& w1 = level.addWall(Point2D(0.0, 0.0), Point2D(4000.0, 0.0), 200.0); // 4000 mm wall

    // Case 1: Door extends beyond wall end (offset 3800 mm + half width 500 mm = 4300 mm > 4000 mm)
    level.addDoor(w1.id, 3800.0, 1000.0, 2100.0);

    // Case 2: Coincidence constraint between disconnected walls
    Wall& w2 = level.addWall(Point2D(6000.0, 0.0), Point2D(6000.0, 3000.0), 200.0);
    std::vector<std::unique_ptr<Constraint>> constraints;
    constraints.emplace_back(std::make_unique<Constraint>(
        "Corner Coincidence", ConstraintType::Coincidence, ConstraintSeverity::Hard,
        std::vector<EntityId>{w1.id, w2.id}
    ));

    auto conflicts = ConstraintSolver::validateConstraints(constraints, level);
    assert(conflicts.size() >= 2);

    bool foundCoincidenceConflict = false;
    bool foundDoorBoundaryConflict = false;

    for (const auto& conf : conflicts) {
        if (conf.type == ConstraintType::Coincidence) foundCoincidenceConflict = true;
        if (conf.type == ConstraintType::HostAttachment) foundDoorBoundaryConflict = true;
    }

    assert(foundCoincidenceConflict);
    assert(foundDoorBoundaryConflict);

    std::cout << "-> Conflict reporting passed." << std::endl;
}

int main() {
    std::cout << "=========================================" << std::endl;
    std::cout << "KaLara Arch Step 09: Constraints Test" << std::endl;
    std::cout << "=========================================" << std::endl;

    testConstraintDefinitionsAndSeverity();
    testConnectedGeometryPropagation();
    testConflictReporting();

    std::cout << "=========================================" << std::endl;
    std::cout << "ALL CONSTRAINT & SMART EDITING TESTS PASSED (100%)" << std::endl;
    std::cout << "=========================================" << std::endl;
    return 0;
}
