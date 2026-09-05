#include "kalara/runtime/selection_manager.hpp"
#include "kalara/runtime/model_manipulator.hpp"
#include "kalara/runtime/snapping_engine.hpp"
#include "kalara/architecture/level.hpp"
#include "kalara/core/geometry/tolerances.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

using namespace kalara::architecture;
using namespace kalara::runtime;
using namespace kalara::core::geometry;

void testSelectionManager() {
    std::cout << "[TEST] SelectionManager single, multi, toggle, and summary...\n";

    Level level("Test Level", 0.0, 3000.0);
    auto& w1 = level.addWall({0.0, 0.0}, {4000.0, 0.0});
    auto& w2 = level.addWall({4000.0, 0.0}, {4000.0, 3000.0});
    auto& r1 = level.addRoom("Bed", RoomType::Bedroom, {
        {0.0, 0.0}, {4000.0, 0.0}, {4000.0, 3000.0}, {0.0, 3000.0}
    });
    level.addDimensionForWall(w1);

    SelectionManager sm;
    assert(sm.empty());
    assert(sm.count() == 0);

    // Single select
    sm.select(w1.id);
    assert(sm.count() == 1);
    assert(sm.isSelected(w1.id));
    assert(!sm.isSelected(w2.id));
    assert(sm.primarySelected().has_value() && *sm.primarySelected() == w1.id);

    // Toggle select
    sm.toggle(w2.id); // Add w2
    assert(sm.count() == 2);
    assert(sm.isSelected(w2.id));

    sm.toggle(w1.id); // Remove w1
    assert(sm.count() == 1);
    assert(!sm.isSelected(w1.id));
    assert(sm.isSelected(w2.id));

    // Multi select
    sm.setSelection({w1.id, w2.id, r1.id});
    assert(sm.count() == 3);

    // Summary test
    auto summary = sm.summarize(level);
    assert(summary.totalCount == 3);
    assert(summary.wallCount == 2);
    assert(summary.roomCount == 1);
    assert(summary.boundingBox.has_value());
    assert(summary.boundingBox->width() == 4000.0);
    assert(summary.boundingBox->height() == 3000.0);

    // Box / Marquee selection test
    sm.clear();
    Rect2D marquee(2000.0, -500.0, 3000.0, 2000.0); // Covers end of w1, start of w2, and part of r1
    size_t inBox = sm.selectInRect(marquee, level);
    assert(inBox >= 2);
    assert(sm.isSelected(w1.id));
    assert(sm.isSelected(w2.id));

    std::cout << "[PASS] SelectionManager tests passed.\n";
}

void testModelManipulatorMove() {
    std::cout << "[TEST] ModelManipulator move entities...\n";

    Level level("Level 1", 0.0, 3000.0);
    auto& w1 = level.addWall({0.0, 0.0}, {5000.0, 0.0});
    auto& w2 = level.addWall({5000.0, 0.0}, {5000.0, 4000.0});
    auto& dim = level.addDimensionForWall(w1);
    assert(dim.measuredDistance_mm() == 5000.0);

    // Move wall 1 by (1000, 500) mm
    Vector2D delta(1000.0, 500.0);
    size_t moved = ModelManipulator::moveEntities(level, {w1.id}, delta, false);
    assert(moved == 1);

    assert(std::abs(w1.start.x - 1000.0) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(w1.start.y - 500.0) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(w1.end.x - 6000.0) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(w1.end.y - 500.0) < Tolerances::LinearEpsilon_mm);

    // Associative dimension should automatically sync (Rule 7)
    assert(std::abs(dim.point1.x - 1000.0) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(dim.point2.x - 6000.0) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(dim.measuredDistance_mm() - 5000.0) < Tolerances::LinearEpsilon_mm);

    std::cout << "[PASS] ModelManipulator move passed.\n";
}

void testModelManipulatorRotate() {
    std::cout << "[TEST] ModelManipulator rotate entities...\n";

    Level level("Level 1", 0.0, 3000.0);
    auto& w = level.addWall({0.0, 0.0}, {4000.0, 0.0}); // Horizontal wall along +X

    // Rotate 90 degrees CCW around origin (0, 0)
    size_t rotated = ModelManipulator::rotateEntities(
        level, {w.id}, {0.0, 0.0}, Angle::fromDegrees(90.0)
    );
    assert(rotated == 1);

    // Start was at (0, 0) -> stays at (0, 0)
    assert(std::abs(w.start.x) < 0.01);
    assert(std::abs(w.start.y) < 0.01);

    // End was at (4000, 0) -> rotates to (0, 4000)
    assert(std::abs(w.end.x) < 0.01);
    assert(std::abs(w.end.y - 4000.0) < 0.01);
    assert(std::abs(w.length_mm() - 4000.0) < Tolerances::LinearEpsilon_mm);

    std::cout << "[PASS] ModelManipulator rotate passed.\n";
}

void testModelManipulatorAlign() {
    std::cout << "[TEST] ModelManipulator multi-entity alignment...\n";

    Level level("Level 1", 0.0, 3000.0);
    // Wall 1 from x = 1000 to x = 3000
    auto& w1 = level.addWall({1000.0, 0.0}, {3000.0, 0.0});
    // Wall 2 from x = 2500 to x = 4500
    auto& w2 = level.addWall({2500.0, 2000.0}, {4500.0, 2000.0});

    // Align Left: bounding box minX is 1000. Wall 2 minX (2500) should shift by -1500 to 1000
    size_t aligned = ModelManipulator::alignEntities(level, {w1.id, w2.id}, AlignmentType::AlignLeft);
    assert(aligned >= 1);

    assert(std::abs(w1.start.x - 1000.0) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(w2.start.x - 1000.0) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(w2.end.x - 3000.0) < Tolerances::LinearEpsilon_mm);

    std::cout << "[PASS] ModelManipulator align passed.\n";
}

void testModelManipulatorResize() {
    std::cout << "[TEST] ModelManipulator wall resizing...\n";

    Level level("Level 1", 0.0, 3000.0);
    auto& w = level.addWall({0.0, 0.0}, {4000.0, 0.0});
    assert(w.length_mm() == 4000.0);

    // Resize to 6000 mm with anchor at start
    bool resized = ModelManipulator::resizeWall(level, w.id, 6000.0, true);
    assert(resized);
    assert(std::abs(w.start.x) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(w.end.x - 6000.0) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(w.length_mm() - 6000.0) < Tolerances::LinearEpsilon_mm);

    // Resize to 3000 mm with anchor at end
    resized = ModelManipulator::resizeWall(level, w.id, 3000.0, false);
    assert(resized);
    assert(std::abs(w.end.x - 6000.0) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(w.start.x - 3000.0) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(w.length_mm() - 3000.0) < Tolerances::LinearEpsilon_mm);

    std::cout << "[PASS] ModelManipulator resize passed.\n";
}

void testSnappingEngine() {
    std::cout << "[TEST] SnappingEngine endpoint, midpoint, and centerline...\n";

    Level level("Level 1", 0.0, 3000.0);
    level.addWall({0.0, 0.0}, {4000.0, 0.0}); // Wall with start (0,0), end (4000,0), midpoint (2000,0)

    // 1. Test Endpoint snap (query close to 4000, 0)
    Point2D queryEnd(3950.0, 30.0);
    auto resEnd = SnappingEngine::snap(queryEnd, level, 100.0);
    assert(resEnd.snapped);
    assert(resEnd.type == SnapType::Endpoint);
    assert(std::abs(resEnd.point.x - 4000.0) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(resEnd.point.y) < Tolerances::LinearEpsilon_mm);

    // 2. Test Midpoint snap (query close to 2000, 0)
    Point2D queryMid(2020.0, -40.0);
    auto resMid = SnappingEngine::snap(queryMid, level, 100.0);
    assert(resMid.snapped);
    assert(resMid.type == SnapType::Midpoint);
    assert(std::abs(resMid.point.x - 2000.0) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(resMid.point.y) < Tolerances::LinearEpsilon_mm);

    // 3. Test Centerline snap (query along wall far from endpoints and midpoint)
    Point2D queryCenterline(1200.0, 50.0);
    auto resCenterline = SnappingEngine::snap(queryCenterline, level, 100.0);
    assert(resCenterline.snapped);
    assert(resCenterline.type == SnapType::WallCenterline);
    assert(std::abs(resCenterline.point.x - 1200.0) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(resCenterline.point.y) < Tolerances::LinearEpsilon_mm);

    // 4. Test Grid snap fallback
    Point2D queryGrid(5020.0, 980.0);
    auto resGrid = SnappingEngine::snap(queryGrid, level, 100.0, true, 1000.0);
    assert(resGrid.snapped);
    assert(resGrid.type == SnapType::Grid);
    assert(std::abs(resGrid.point.x - 5000.0) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(resGrid.point.y - 1000.0) < Tolerances::LinearEpsilon_mm);

    std::cout << "[PASS] SnappingEngine tests passed.\n";
}

int main() {
    std::cout << "--- Starting KaLara Arch Manipulation & Selection Tests ---\n";
    testSelectionManager();
    testModelManipulatorMove();
    testModelManipulatorRotate();
    testModelManipulatorAlign();
    testModelManipulatorResize();
    testSnappingEngine();
    std::cout << "--- All Selection & Manipulation Tests Passed! ---\n";
    return 0;
}
