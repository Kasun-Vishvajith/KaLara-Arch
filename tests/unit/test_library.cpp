#include "kalara/architecture/library_catalog.hpp"
#include "kalara/architecture/level.hpp"
#include "kalara/runtime/selection_manager.hpp"
#include "kalara/runtime/model_manipulator.hpp"
#include "kalara/core/geometry/tolerances.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

using namespace kalara::architecture;
using namespace kalara::runtime;
using namespace kalara::core::geometry;

void testLibraryCatalogBuiltins() {
    std::cout << "[TEST] LibraryCatalog builtins and queries...\n";

    LibraryCatalog catalog;
    assert(catalog.itemCount() >= 10);

    // 1. Verify standard items exist
    const auto* bed = catalog.findItem("BED_DOUBLE");
    assert(bed != nullptr);
    assert(bed->name == "Double Bed (Queen)");
    assert(bed->category == LibraryCategory::Furniture);
    assert(bed->semanticType == SemanticType::Furniture);
    assert(bed->width_mm == 1600.0);
    assert(bed->length_mm == 2000.0);
    assert(bed->clearance.front_mm == 600.0);
    assert(bed->clearance.hasClearance());

    const auto* wc = catalog.findItem("TOILET_WC");
    assert(wc != nullptr);
    assert(wc->category == LibraryCategory::BathroomFixtures);
    assert(wc->semanticType == SemanticType::Fixture);
    assert(wc->width_mm == 400.0);
    assert(wc->length_mm == 700.0);

    const auto* stair = catalog.findItem("STAIR_STRAIGHT_RUN");
    assert(stair != nullptr);
    assert(stair->category == LibraryCategory::Stairs);
    assert(stair->semanticType == SemanticType::Stair);
    assert(stair->length_mm == 3600.0);

    const auto* tree = catalog.findItem("TREE_CANOPY");
    assert(tree != nullptr);
    assert(tree->category == LibraryCategory::SiteOutdoor);
    assert(tree->semanticType == SemanticType::SiteElement);

    // 2. Test Category queries
    auto furnitures = catalog.getItemsByCategory(LibraryCategory::Furniture);
    assert(!furnitures.empty());
    for (const auto* f : furnitures) {
        assert(f->category == LibraryCategory::Furniture);
    }

    auto bathrooms = catalog.getItemsByCategory(LibraryCategory::BathroomFixtures);
    assert(bathrooms.size() >= 4);

    // 3. Test search
    auto searchBed = catalog.searchItems("bed");
    assert(searchBed.size() >= 2); // Double and single

    auto searchSofa = catalog.searchItems("SOFA");
    assert(!searchSofa.empty());

    std::cout << "[PASS] LibraryCatalog builtins verified.\n";
}

void testUserDefinedLibraryItem() {
    std::cout << "[TEST] User-defined library item registration...\n";

    LibraryCatalog catalog;
    size_t beforeCount = catalog.itemCount();

    LibraryItem customItem("CUSTOM_ISLAND", "Kitchen Island with Seating",
                           LibraryCategory::KitchenObjects, SemanticType::Fixture,
                           2400.0, 1200.0, 900.0);
    customItem.clearance.front_mm = 900.0;
    customItem.clearance.rear_mm = 900.0;

    bool ok = catalog.registerUserItem(customItem);
    assert(ok);
    assert(catalog.itemCount() == beforeCount + 1);

    const auto* found = catalog.findItem("CUSTOM_ISLAND");
    assert(found != nullptr);
    assert(found->isUserCreated);
    assert(found->width_mm == 2400.0);
    assert(found->length_mm == 1200.0);

    std::cout << "[PASS] User-defined library item registered.\n";
}

void testLibraryInstanceGeometryAndSelection() {
    std::cout << "[TEST] LibraryInstance world transforms, clearances, and manipulation...\n";

    LibraryCatalog catalog;
    const auto* bedItem = catalog.findItem("BED_DOUBLE");
    assert(bedItem != nullptr);

    Level level("Level 1", 0.0, 3000.0);

    // Place bed instance at (2000, 1500) mm
    auto& bedInst = level.addLibraryInstance(*bedItem, {2000.0, 1500.0});
    assert(bedInst.width_mm == 1600.0);
    assert(bedInst.length_mm == 2000.0);

    // Test bounding box
    auto bb = bedInst.boundingBox();
    assert(std::abs(bb.center().x - 2000.0) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(bb.center().y - 1500.0) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(bb.width() - 1600.0) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(bb.height() - 2000.0) < Tolerances::LinearEpsilon_mm);

    // Test hit testing
    assert(bedInst.containsPoint({2000.0, 1500.0})); // Center
    assert(bedInst.containsPoint({2000.0 + 700.0, 1500.0 + 900.0})); // Inside near corner
    assert(!bedInst.containsPoint({2000.0 + 1000.0, 1500.0})); // Outside

    // Test clearance zone
    auto clearPoly = bedInst.worldClearanceZone();
    assert(clearPoly.size() == 4);

    // Test selection manager integration
    SelectionManager sm;
    sm.select(bedInst.id);
    assert(sm.count() == 1);
    assert(sm.isSelected(bedInst.id));

    auto summary = sm.summarize(level);
    assert(summary.totalCount == 1);
    assert(summary.libraryInstanceCount == 1);
    assert(summary.boundingBox.has_value());

    // Test moving library instance via ModelManipulator (Principle 6 & Rule 3)
    ModelManipulator::moveEntities(level, {bedInst.id}, Vector2D(500.0, -300.0));
    assert(std::abs(bedInst.position.x - 2500.0) < Tolerances::LinearEpsilon_mm);
    assert(std::abs(bedInst.position.y - 1200.0) < Tolerances::LinearEpsilon_mm);

    // Test rotating library instance via ModelManipulator
    ModelManipulator::rotateEntities(level, {bedInst.id}, bedInst.position, Angle::fromDegrees(90.0));
    assert(std::abs(bedInst.rotation.degrees() - 90.0) < Tolerances::AngularEpsilon_rad);

    std::cout << "[PASS] LibraryInstance geometry, selection, and manipulation verified.\n";
}

int main() {
    std::cout << "--- Starting KaLara Arch Architectural 2D Library Tests ---\n";
    testLibraryCatalogBuiltins();
    testUserDefinedLibraryItem();
    testLibraryInstanceGeometryAndSelection();
    std::cout << "--- All Architectural 2D Library Tests Passed! ---\n";
    return 0;
}
