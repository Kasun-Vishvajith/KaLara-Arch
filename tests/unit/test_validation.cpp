#include "kalara/architecture/project.hpp"
#include "kalara/architecture/site.hpp"
#include "kalara/architecture/building.hpp"
#include "kalara/architecture/level.hpp"
#include "kalara/architecture/library_catalog.hpp"
#include "kalara/runtime/validation_engine.hpp"
#include "kalara/core/geometry/tolerances.hpp"
#include <cassert>
#include <iostream>

using namespace kalara::architecture;
using namespace kalara::runtime;
using namespace kalara::core::geometry;

void testGeometryValidation() {
    std::cout << "[TEST] Geometry validation (degenerate walls, invalid thickness, duplicates)...\n";

    Level level("Test Level", 0.0);

    // Valid wall
    auto& validWall = level.addWall({0.0, 0.0}, {4000.0, 0.0}, 200.0);

    // 1. Degenerate zero-length wall (length = 0)
    auto& zeroWall = level.addWall({1000.0, 1000.0}, {1000.0, 1000.0}, 200.0);

    // 2. Invalid thickness wall
    auto& badThickWall = level.addWall({0.0, 2000.0}, {3000.0, 2000.0}, 0.0);

    // 3. Duplicate overlapping wall (same endpoints as validWall)
    auto& dupWall = level.addWall({0.0, 0.0}, {4000.0, 0.0}, 200.0);

    auto report = ValidationEngine::validateLevel(level);

    assert(report.hasErrors());
    assert(report.hasWarnings());

    auto geomIssues = report.filterByCategory(ValidationCategory::Geometry);
    assert(geomIssues.size() >= 3);

    bool foundZeroLen = false;
    bool foundBadThick = false;
    bool foundDup = false;

    for (const auto& issue : geomIssues) {
        if (issue.code == "GEOM_WALL_ZERO_LENGTH") {
            foundZeroLen = true;
            assert(issue.severity == ValidationSeverity::Error);
            assert(!issue.entityIds.empty() && issue.entityIds[0] == zeroWall.id);
        } else if (issue.code == "GEOM_WALL_INVALID_THICKNESS") {
            foundBadThick = true;
            assert(issue.severity == ValidationSeverity::Error);
            assert(!issue.entityIds.empty() && issue.entityIds[0] == badThickWall.id);
        } else if (issue.code == "GEOM_WALL_DUPLICATE") {
            foundDup = true;
            assert(issue.severity == ValidationSeverity::Warning);
        }
    }

    assert(foundZeroLen);
    assert(foundBadThick);
    assert(foundDup);

    std::cout << "  -> Geometry validation passed.\n";
}

void testConnectionAndOpeningValidation() {
    std::cout << "[TEST] Connection & opening validation (out-of-bounds, overlaps)...\n";

    Level level("Connection Level", 0.0);
    auto& wall = level.addWall({0.0, 0.0}, {3000.0, 0.0}, 200.0); // 3000 mm wall

    // 1. Door extending out of wall bounds (offset 2500 + width 900 = 3400 > 3000)
    auto& badDoor = level.addDoor(wall.id, 2500.0, 900.0);

    // 2. Window with negative offset
    auto& badWin = level.addWindow(wall.id, -200.0, 800.0);

    // 3. Two overlapping openings on same wall
    auto& doorA = level.addDoor(wall.id, 500.0, 900.0);  // 500 to 1400
    auto& winB = level.addWindow(wall.id, 1000.0, 800.0); // 1000 to 1800 (overlaps from 1000 to 1400)

    auto report = ValidationEngine::validateLevel(level);

    auto connIssues = report.filterByCategory(ValidationCategory::Connection);
    assert(connIssues.size() >= 3);

    bool foundDoorOOB = false;
    bool foundWinOOB = false;
    bool foundOverlap = false;

    for (const auto& issue : connIssues) {
        if (issue.code == "CONN_DOOR_OUT_OF_BOUNDS") foundDoorOOB = true;
        if (issue.code == "CONN_WINDOW_OUT_OF_BOUNDS") foundWinOOB = true;
        if (issue.code == "CONN_OPENINGS_OVERLAP") foundOverlap = true;
    }

    assert(foundDoorOOB);
    assert(foundWinOOB);
    assert(foundOverlap);

    std::cout << "  -> Connection & opening validation passed.\n";
}

void testRoomValidation() {
    std::cout << "[TEST] Room boundary validation (degenerate & zero-area)...\n";

    Level level("Room Level", 0.0);

    // 1. Degenerate boundary with 2 vertices
    level.addRoom("Bad Room 1", RoomType::Bedroom, {Point2D(0.0, 0.0), Point2D(1000.0, 0.0)});

    // 2. Collinear boundary with zero area
    level.addRoom("Collinear Room", RoomType::Kitchen, {
        Point2D(0.0, 0.0), Point2D(2000.0, 0.0), Point2D(4000.0, 0.0)
    });

    auto report = ValidationEngine::validateLevel(level);
    auto roomIssues = report.filterByCategory(ValidationCategory::Room);
    assert(!roomIssues.empty());

    bool foundDegen = false;
    bool foundZeroArea = false;
    for (const auto& issue : roomIssues) {
        if (issue.code == "ROOM_DEGENERATE_BOUNDARY") foundDegen = true;
        if (issue.code == "ROOM_ZERO_AREA") foundZeroArea = true;
    }
    assert(foundDegen || foundZeroArea);

    std::cout << "  -> Room boundary validation passed.\n";
}

void testClearanceValidation() {
    std::cout << "[TEST] Clearance validation (fixture obstruction & door swing collisions)...\n";

    Level level("Clearance Level", 0.0);

    // Corner with door swinging into perpendicular wall
    auto& southWall = level.addWall({0.0, 0.0}, {4000.0, 0.0}, 200.0);
    auto& westWall = level.addWall({0.0, 0.0}, {0.0, 3000.0}, 200.0);

    // Door placed right at the corner swinging into the west wall
    auto& cornerDoor = level.addDoor(southWall.id, 100.0, 900.0, 2100.0, DoorSwing::LeftInswing);

    // Bed with clearance zone placed colliding with a partition wall
    LibraryCatalog catalog;
    const auto* bed = catalog.findItem("BED_DOUBLE");
    assert(bed != nullptr);
    auto& bedInst = level.addLibraryInstance(*bed, {2000.0, 1500.0});

    // Partition wall cutting right through bed's front clearance
    level.addWall({1500.0, 1000.0}, {2500.0, 1000.0}, 150.0);

    auto report = ValidationEngine::validateLevel(level);
    auto clIssues = report.filterByCategory(ValidationCategory::Clearance);
    assert(!clIssues.empty());

    bool foundObstruction = false;
    for (const auto& issue : clIssues) {
        if (issue.code == "CLEARANCE_WALL_OBSTRUCTION") foundObstruction = true;
    }
    assert(foundObstruction);

    std::cout << "  -> Clearance validation passed.\n";
}

void testSiteAndSetbackValidation() {
    std::cout << "[TEST] Site & setback validation (property boundary & setbacks)...\n";

    Site site("Parcel 101");
    site.propertyBoundary = {
        Point2D(-10000.0, -10000.0),
        Point2D(10000.0, -10000.0),
        Point2D(10000.0, 10000.0),
        Point2D(-10000.0, 10000.0)
    };
    site.setbacks.front_mm = 4000.0;
    site.setbacks.rear_mm = 3000.0;
    site.setbacks.sideLeft_mm = 2000.0;
    site.setbacks.sideRight_mm = 2000.0;

    auto& bld = site.addBuilding("Main House");
    auto& ground = bld.addLevel("Ground", 0.0);

    // Wall 1: Encroaches into the front setback buffer (y = -7000 mm < -6000 mm setback limit)
    ground.addWall(Point2D(-5000.0, -7500.0), Point2D(5000.0, -7500.0), 200.0);

    // Wall 2: Projects completely outside property boundary (x = 12000 > 10000)
    ground.addWall(Point2D(11000.0, 0.0), Point2D(12000.0, 0.0), 200.0);

    auto report = ValidationEngine::validateSite(site);

    auto siteIssues = report.filterByCategory(ValidationCategory::SiteAndSetback);
    assert(siteIssues.size() >= 2);

    bool foundOutside = false;
    bool foundEncroach = false;

    for (const auto& issue : siteIssues) {
        if (issue.code == "SITE_OUTSIDE_PROPERTY") {
            foundOutside = true;
            assert(issue.severity == ValidationSeverity::Error);
        } else if (issue.code == "SITE_SETBACK_ENCROACHMENT") {
            foundEncroach = true;
            assert(issue.severity == ValidationSeverity::Warning);
        }
    }

    assert(foundOutside);
    assert(foundEncroach);

    // Verify Rule 16 Legal Disclaimer
    assert(!ValidationReport::LegalDisclaimer.empty());
    assert(ValidationReport::LegalDisclaimer.find("does not constitute or replace required professional") != std::string_view::npos);

    std::cout << "  -> Site & setback validation and Rule 16 disclaimer passed.\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "KaLara Arch - Unit Tests: Validation Engine\n";
    std::cout << "========================================\n";

    testGeometryValidation();
    testConnectionAndOpeningValidation();
    testRoomValidation();
    testClearanceValidation();
    testSiteAndSetbackValidation();

    std::cout << "\nAll Validation Engine unit tests passed successfully!\n";
    return 0;
}
