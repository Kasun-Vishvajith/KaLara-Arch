#include "kalara/editor/viewport_state.hpp"
#include "kalara/editor/grid_settings.hpp"
#include "kalara/runtime/selection_manager.hpp"
#include "kalara/architecture/entity_id.hpp"
#include "kalara/core/geometry/tolerances.hpp"
#include <iostream>
#include <cassert>

using namespace kalara::editor;
using namespace kalara::runtime;
using namespace kalara::architecture;
using namespace kalara::core::geometry;

void testViewportTransformations() {
    std::cout << "Testing Viewport Screen <-> World Transformations..." << std::endl;
    ViewportState state;
    state.scale = 0.1; // 10 mm = 1 px
    state.panOffset_mm = Point2D(0.0, 0.0);
    state.viewportWidth = 1000;
    state.viewportHeight = 1000;

    // Viewport center is at (500, 500)
    Point2D worldOrigin = state.screenToWorld(500.0, 500.0);
    assert(Tolerances::equalLinear(worldOrigin.x, 0.0));
    assert(Tolerances::equalLinear(worldOrigin.y, 0.0));

    Point2D screenOrigin = state.worldToScreen(0.0, 0.0);
    assert(Tolerances::equalLinear(screenOrigin.x, 500.0));
    assert(Tolerances::equalLinear(screenOrigin.y, 500.0));

    // World point (2000, 3000) mm
    // screenX = 2000 * 0.1 + 500 = 700
    // screenY = -3000 * 0.1 + 500 = 200
    Point2D ptScreen = state.worldToScreen(2000.0, 3000.0);
    assert(Tolerances::equalLinear(ptScreen.x, 700.0));
    assert(Tolerances::equalLinear(ptScreen.y, 200.0));

    Point2D ptWorld = state.screenToWorld(700.0, 200.0);
    assert(Tolerances::equalLinear(ptWorld.x, 2000.0));
    assert(Tolerances::equalLinear(ptWorld.y, 3000.0));

    // Pan test
    state.panByScreenDelta(100.0, -50.0);
    assert(Tolerances::equalLinear(state.panOffset_mm.x, -1000.0));
    assert(Tolerances::equalLinear(state.panOffset_mm.y, -500.0));

    // Zoom test (factor 2.0 around center)
    double scaleBefore = state.scale;
    state.zoomAtScreenPoint(2.0, 500.0, 500.0);
    assert(Tolerances::equalLinear(state.scale, scaleBefore * 2.0));
    std::cout << "-> Viewport transformations passed." << std::endl;
}

void testGridAndSnapping() {
    std::cout << "Testing Grid & Snapping Settings..." << std::endl;
    GridSettings grid;
    grid.primarySpacing_mm = 1000.0;
    grid.secondarySpacing_mm = 100.0;
    grid.snapEnabled = true;

    Point2D raw(1234.4, 5678.9);
    Point2D snapped = grid.snap(raw);
    assert(Tolerances::equalLinear(snapped.x, 1200.0));
    assert(Tolerances::equalLinear(snapped.y, 5700.0));

    grid.snapEnabled = false;
    Point2D noSnap = grid.snap(raw);
    assert(noSnap == raw);
    std::cout << "-> Grid & Snapping passed." << std::endl;
}

void testSelectionManager() {
    std::cout << "Testing SelectionManager..." << std::endl;
    SelectionManager sm;
    EntityId w1 = EntityId::generate("wall");
    EntityId w2 = EntityId::generate("wall");

    assert(sm.count() == 0);
    sm.select(w1);
    assert(sm.count() == 1);
    assert(sm.isSelected(w1));
    assert(!sm.isSelected(w2));

    sm.select(w2);
    assert(sm.count() == 2);

    sm.deselect(w1);
    assert(sm.count() == 1);
    assert(!sm.isSelected(w1));
    assert(sm.isSelected(w2));

    sm.clear();
    assert(sm.count() == 0);
    std::cout << "-> SelectionManager passed." << std::endl;
}

int main() {
    std::cout << "=========================================" << std::endl;
    std::cout << "KaLara Arch Step 04: Viewport & Grid Test" << std::endl;
    std::cout << "=========================================" << std::endl;

    testViewportTransformations();
    testGridAndSnapping();
    testSelectionManager();

    std::cout << "=========================================" << std::endl;
    std::cout << "ALL VIEWPORT & GRID TESTS PASSED (100%)" << std::endl;
    std::cout << "=========================================" << std::endl;
    return 0;
}
