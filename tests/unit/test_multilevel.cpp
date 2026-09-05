#include "kalara/architecture/building.hpp"
#include "kalara/architecture/level.hpp"
#include "kalara/architecture/roof.hpp"
#include "kalara/runtime/selection_manager.hpp"
#include "kalara/runtime/model_manipulator.hpp"
#include "kalara/core/geometry/tolerances.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

using namespace kalara::architecture;
using namespace kalara::runtime;
using namespace kalara::core::geometry;

void testBuildingLevelHierarchyAndSorting() {
    std::cout << "[TEST] Building level hierarchy, elevations (mm), and sorting...\n";

    Building building("Multi-Story Villa");

    // Add levels in non-chronological order to test automatic elevation sorting
    auto& ground = building.addLevel("Ground Floor", 0.0, 3200.0, LevelType::GroundFloor);
    auto& upper = building.addLevel("First Floor", 3200.0, 3000.0, LevelType::UpperFloor);
    auto& basement = building.addLevel("Basement", -2800.0, 2800.0, LevelType::Basement);

    const auto& levels = building.levels();
    assert(levels.size() == 3);

    // Verify sorted order: Basement (-2800), Ground (0), Upper (3200)
    assert(levels[0]->name == "Basement");
    assert(levels[0]->elevation_mm == -2800.0);
    assert(levels[0]->type == LevelType::Basement);

    assert(levels[1]->name == "Ground Floor");
    assert(levels[1]->elevation_mm == 0.0);
    assert(levels[1]->type == LevelType::GroundFloor);

    assert(levels[2]->name == "First Floor");
    assert(levels[2]->elevation_mm == 3200.0);
    assert(levels[2]->type == LevelType::UpperFloor);

    // Test Active Level switching
    assert(building.activeLevel() != nullptr);
    building.setActiveLevel(upper.id);
    assert(building.activeLevel()->id == upper.id);
    assert(building.activeLevel()->name == "First Floor");

    building.setActiveLevel(ground.id);
    assert(building.activeLevel()->id == ground.id);
    assert(building.activeLevel()->name == "Ground Floor");

    std::cout << "  -> Hierarchy and elevation sorting passed.\n";
}

void testDuplicateLevelUpwardsAndUnderlay() {
    std::cout << "[TEST] Duplicate level upwards and underlay ghost reference...\n";

    Building building("Duplication Test Building");
    auto& ground = building.addLevel("Ground Floor", 0.0, 3000.0, LevelType::GroundFloor);

    // Add perimeter walls to ground floor
    ground.addWall({0.0, 0.0}, {6000.0, 0.0}, 200.0);
    ground.addWall({6000.0, 0.0}, {6000.0, 5000.0}, 200.0);
    ground.addWall({6000.0, 5000.0}, {0.0, 5000.0}, 200.0);
    ground.addWall({0.0, 5000.0}, {0.0, 0.0}, 200.0);
    assert(ground.walls().size() == 4);

    // Duplicate level upwards as Second Floor
    auto* secondFloor = building.duplicateLevelUpwards(ground.id, "Second Floor");
    assert(secondFloor != nullptr);
    assert(secondFloor->name == "Second Floor");
    assert(secondFloor->elevation_mm == 3000.0);
    assert(secondFloor->height_mm == 3000.0);
    assert(secondFloor->walls().size() == 4);

    // Verify underlay reference points to Ground Floor
    assert(secondFloor->underlayLevelId.has_value());
    assert(secondFloor->underlayLevelId.value() == ground.id);
    assert(secondFloor->underlayOpacity == 0.4);

    // Duplicate once more to Third Floor
    auto* thirdFloor = building.duplicateLevelUpwards(secondFloor->id, "Third Floor");
    assert(thirdFloor != nullptr);
    assert(thirdFloor->elevation_mm == 6000.0);
    assert(thirdFloor->underlayLevelId.value() == secondFloor->id);
    assert(building.levels().size() == 3);

    std::cout << "  -> Floor duplication and underlay reference passed.\n";
}

void testRoofLevelAndGeometryCalculations() {
    std::cout << "[TEST] Roof level, pitch, overhang, eave boundary, and surface area...\n";

    Building building("Roof Test");
    auto& ground = building.addLevel("Ground Floor", 0.0, 3000.0);
    ground.addWall({0.0, 0.0}, {10000.0, 0.0});
    ground.addWall({10000.0, 0.0}, {10000.0, 8000.0});
    ground.addWall({10000.0, 8000.0}, {0.0, 8000.0});
    ground.addWall({0.0, 8000.0}, {0.0, 0.0});

    // Add Roof level on top of the building
    auto& roofLevel = building.addRoofLevel("Roof Plan", 2500.0);
    assert(roofLevel.elevation_mm == 3000.0);
    assert(roofLevel.type == LevelType::Roof);

    // Add a Gable Roof: 10m x 8m footprint, 30 degree pitch, 500 mm overhang
    std::vector<Point2D> boundary = {
        {0.0, 0.0},
        {10000.0, 0.0},
        {10000.0, 8000.0},
        {0.0, 8000.0}
    };
    auto& roof = roofLevel.addRoof("Main Villa Roof", RoofType::Gable, boundary, 30.0, 500.0);

    assert(roof.name == "Main Villa Roof");
    assert(roof.type == RoofType::Gable);
    assert(roof.pitch_deg == 30.0);
    assert(roof.overhang_mm == 500.0);

    // Verify footprint area: 10000 * 8000 = 80,000,000 mm^2 (80 m^2)
    double fpArea = roof.footprintArea_mm2();
    assert(std::abs(fpArea - 80000000.0) < 1.0);

    // Verify surface area: fpArea / cos(30 deg)
    // cos(30 deg) = sqrt(3)/2 ≈ 0.8660254
    // 80,000,000 / 0.8660254 ≈ 92,376,043 mm^2
    double surfArea = roof.surfaceArea_mm2();
    assert(surfArea > fpArea);
    double expectedSurf = fpArea / std::cos(30.0 * 3.14159265358979323846 / 180.0);
    assert(std::abs(surfArea - expectedSurf) < 10.0);

    // Verify eave boundary expands boundary outward by overhang (500 mm)
    auto eaves = roof.eaveBoundary;
    assert(eaves.size() == 4);
    assert(std::abs(eaves[0].x - (-500.0)) < 1.0);
    assert(std::abs(eaves[0].y - (-500.0)) < 1.0);
    assert(std::abs(eaves[1].x - 10500.0) < 1.0);
    assert(std::abs(eaves[1].y - (-500.0)) < 1.0);
    assert(std::abs(eaves[2].x - 10500.0) < 1.0);
    assert(std::abs(eaves[2].y - 8500.0) < 1.0);
    assert(std::abs(eaves[3].x - (-500.0)) < 1.0);
    assert(std::abs(eaves[3].y - 8500.0) < 1.0);

    // Test point containment
    assert(roof.containsPoint({5000.0, 4000.0}));     // Center inside
    assert(roof.containsPoint({-200.0, -200.0}));     // Inside eave overhang
    assert(!roof.containsPoint({-800.0, -800.0}));    // Outside overhang

    // Verify ridge line computation for Gable
    auto ridges = roof.ridgeLines;
    assert(ridges.size() == 1);
    assert(std::abs(ridges[0].start.y - 4000.0) < 1.0);
    assert(std::abs(ridges[0].end.y - 4000.0) < 1.0);

    std::cout << "  -> Roof geometry, pitch, and eave calculations passed.\n";
}

void testRoofManipulationAndSelection() {
    std::cout << "[TEST] Roof selection and transformation...\n";

    Level roofLevel("Roof", 3000.0, 2400.0, LevelType::Roof);
    std::vector<Point2D> boundary = {
        {0.0, 0.0}, {4000.0, 0.0}, {4000.0, 3000.0}, {0.0, 3000.0}
    };
    auto& roof = roofLevel.addRoof("Test Hip Roof", RoofType::Hip, boundary, 25.0, 400.0);

    // Test selection hit
    SelectionManager sel;
    sel.selectInRect(Rect2D{-1000.0, -1000.0, 6000.0, 5000.0}, roofLevel);
    assert(sel.isSelected(roof.id));
    auto summary = sel.summarize(roofLevel);
    assert(summary.roofCount == 1);
    assert(summary.totalCount == 1);

    // Test ModelManipulator move
    size_t moved = ModelManipulator::moveEntities(roofLevel, sel.selectedList(), {1000.0, 500.0});
    assert(moved > 0);
    // wallFootprint[0] moved from (0, 0) to (1000, 500)
    assert(std::abs(roof.wallFootprint[0].x - 1000.0) < 1.0);
    assert(std::abs(roof.wallFootprint[0].y - 500.0) < 1.0);
    // eaveBoundary[0] moved from (-400, -400) to (600, 100)
    assert(std::abs(roof.eaveBoundary[0].x - 600.0) < 1.0);
    assert(std::abs(roof.eaveBoundary[0].y - 100.0) < 1.0);

    // Test ModelManipulator rotate 90 deg around (1000, 500)
    size_t rotated = ModelManipulator::rotateEntities(roofLevel, sel.selectedList(), {1000.0, 500.0}, Angle::fromDegrees(90.0));
    assert(rotated > 0);

    std::cout << "  -> Roof selection and manipulation passed.\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "KaLara Arch - Unit Tests: Multi-Level & Roofs\n";
    std::cout << "========================================\n";

    testBuildingLevelHierarchyAndSorting();
    testDuplicateLevelUpwardsAndUnderlay();
    testRoofLevelAndGeometryCalculations();
    testRoofManipulationAndSelection();

    std::cout << "\nAll Multi-Level & Roof unit tests passed successfully!\n";
    return 0;
}
