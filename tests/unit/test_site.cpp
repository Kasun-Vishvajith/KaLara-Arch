#include "kalara/architecture/site.hpp"
#include "kalara/architecture/site_elements.hpp"
#include "kalara/core/geometry/tolerances.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

using namespace kalara::architecture;
using namespace kalara::core::geometry;

void testPropertyBoundaryAndMetrics() {
    std::cout << "[TEST] Property boundary area, perimeter, and point containment...\n";

    Site site("Hillside Estate");
    site.propertyBoundary = {
        Point2D(-10000.0, -15000.0),
        Point2D(10000.0, -15000.0),
        Point2D(10000.0, 15000.0),
        Point2D(-10000.0, 15000.0)
    };

    // 20,000 mm x 30,000 mm = 600,000,000 mm^2 = 600 m^2
    double area = site.propertyArea_mm2();
    assert(std::abs(area - 600000000.0) < 1.0);
    assert(std::abs(site.propertyArea_m2() - 600.0) < 0.01);

    // Perimeter = 2 * (20m + 30m) = 100m = 100,000 mm
    double perim = site.propertyPerimeter_mm();
    assert(std::abs(perim - 100000.0) < 1.0);

    // Point containment
    assert(site.containsPoint(Point2D(0.0, 0.0)));
    assert(site.containsPoint(Point2D(5000.0, -10000.0)));
    assert(!site.containsPoint(Point2D(15000.0, 0.0)));
    assert(!site.containsPoint(Point2D(0.0, -20000.0)));

    std::cout << "  -> Property boundary metrics and containment passed.\n";
}

void testSetbacksAndBuildableEnvelope() {
    std::cout << "[TEST] Municipal setbacks and buildable envelope polygon...\n";

    Site site("Suburban Parcel");
    site.propertyBoundary = {
        Point2D(-10000.0, -15000.0),
        Point2D(10000.0, -15000.0),
        Point2D(10000.0, 15000.0),
        Point2D(-10000.0, 15000.0)
    };

    site.setbacks.front_mm = 5000.0;     // South
    site.setbacks.rear_mm = 3000.0;      // North
    site.setbacks.sideLeft_mm = 2000.0;  // West
    site.setbacks.sideRight_mm = 2000.0; // East

    auto envelope = site.buildableEnvelope();
    assert(envelope.size() == 4);

    // Width = 20000 - 2000 - 2000 = 16000 mm (-8000 to +8000)
    assert(std::abs(envelope[0].x - (-8000.0)) < 1.0);
    assert(std::abs(envelope[1].x - 8000.0) < 1.0);

    // Length = 30000 - 5000 - 3000 = 22000 mm (-10000 to +12000)
    assert(std::abs(envelope[0].y - (-10000.0)) < 1.0);
    assert(std::abs(envelope[2].y - 12000.0) < 1.0);

    // Buildable area = 16000 * 22000 = 352,000,000 mm^2 (352 m^2)
    double bArea = site.buildableArea_mm2();
    assert(std::abs(bArea - 352000000.0) < 1.0);

    // Test building footprint coverage
    auto& bld = site.addBuilding("Residence");
    auto& ground = bld.addLevel("Ground Floor", 0.0);
    ground.addWall(Point2D(-4000.0, -3000.0), Point2D(4000.0, -3000.0), 200.0);
    ground.addWall(Point2D(4000.0, -3000.0), Point2D(4000.0, 3000.0), 200.0);
    ground.addWall(Point2D(4000.0, 3000.0), Point2D(-4000.0, 3000.0), 200.0);
    ground.addWall(Point2D(-4000.0, 3000.0), Point2D(-4000.0, -3000.0), 200.0);

    // Footprint: 8m x 6m = 48,000,000 mm^2 = 48 m^2
    double fp = site.buildingFootprintArea_mm2();
    assert(std::abs(fp - 48000000.0) < 1.0);

    // Site coverage: 48 / 600 = 8.0%
    double cov = site.siteCoveragePercentage();
    assert(std::abs(cov - 8.0) < 0.01);

    std::cout << "  -> Setbacks, buildable envelope, and coverage ratio passed.\n";
}

void testRoadContextAndEntrances() {
    std::cout << "[TEST] Road frontage and site access entrances...\n";

    Site site("Street Access Site");

    // Add frontage road along South
    auto& road = site.addRoad("Oakwood Boulevard", Segment2D({-20000.0, -18000.0}, {20000.0, -18000.0}), 12000.0);
    assert(road.name == "Oakwood Boulevard");
    assert(road.width_mm == 12000.0);
    assert(road.semanticType() == SemanticType::SiteElement);
    assert(site.roads().size() == 1);

    // Add vehicular driveway entrance and pedestrian gate
    auto& driveway = site.addEntrance("Main Driveway", SiteEntranceType::VehicularDriveway, Point2D(4000.0, -15000.0), 3800.0);
    assert(driveway.type == SiteEntranceType::VehicularDriveway);
    assert(driveway.width_mm == 3800.0);

    auto& gate = site.addEntrance("Garden Gate", SiteEntranceType::PedestrianGate, Point2D(-5000.0, -15000.0), 1000.0);
    assert(gate.type == SiteEntranceType::PedestrianGate);
    assert(gate.width_mm == 1000.0);
    assert(site.entrances().size() == 2);

    std::cout << "  -> Road context and entrances passed.\n";
}

void testParkingAndLandscapeZones() {
    std::cout << "[TEST] Parking stall layouts and landscape zoning...\n";

    Site site("Site Amenities");

    // Add 2-stall parking zone (2600mm x 5200mm per stall)
    auto& prk = site.addParkingZone("Guest Parking", Point2D(3000.0, -12000.0), 2, Angle{}, 2600.0, 5200.0, true);
    assert(prk.stallCount == 2);
    assert(prk.isAccessible);
    assert(std::abs(prk.totalArea_mm2() - (2.0 * 2600.0 * 5200.0)) < 1.0);

    auto stalls = prk.stallOutlines();
    assert(stalls.size() == 2);
    assert(stalls[0].size() == 4);
    assert(stalls[1].size() == 4);

    // Add landscape zones
    std::vector<Point2D> lawnOutline = {
        Point2D(-8000.0, -14000.0),
        Point2D(1000.0, -14000.0),
        Point2D(1000.0, -8000.0),
        Point2D(-8000.0, -8000.0)
    };
    auto& lawn = site.addLandscapeZone("Front Lawn", LandscapeType::Lawn, lawnOutline);
    assert(lawn.type == LandscapeType::Lawn);
    // 9m x 6m = 54 m^2 = 54,000,000 mm^2
    assert(std::abs(lawn.area_mm2() - 54000000.0) < 1.0);
    assert(site.landscapeZones().size() == 1);

    std::cout << "  -> Parking stalls and landscape zones passed.\n";
}

void testPoolOutdoorElementsAndOrientation() {
    std::cout << "[TEST] Swimming pool volume, deck coping, True North orientation, and spot elevations...\n";

    Site site("Luxury Villa Site");

    // Add 10m x 4m swimming pool with depth 1500 mm and 500 mm coping
    std::vector<Point2D> poolBoundary = {
        Point2D(-5000.0, 8000.0),
        Point2D(5000.0, 8000.0),
        Point2D(5000.0, 12000.0),
        Point2D(-5000.0, 12000.0)
    };
    auto& pool = site.addOutdoorElement("Infinity Pool", OutdoorElementType::SwimmingPool, poolBoundary, 1500.0, 500.0);
    assert(pool.type == OutdoorElementType::SwimmingPool);

    // Water area: 10000 * 4000 = 40,000,000 mm^2 (40 m^2)
    assert(std::abs(pool.waterArea_mm2() - 40000000.0) < 1.0);

    // Water volume: 40 m^2 * 1.5 m = 60 m^3 = 60,000 Litres
    assert(std::abs(pool.waterVolume_liters() - 60000.0) < 1.0);

    // Coping boundary: expanded by 500 mm on all sides (-5500 to +5500, 7500 to 12500)
    auto coping = pool.copingBoundary();
    assert(coping.size() == 4);
    assert(std::abs(coping[0].x - (-5500.0)) < 1.0);
    assert(std::abs(coping[0].y - 7500.0) < 1.0);
    assert(std::abs(coping[2].x - 5500.0) < 1.0);
    assert(std::abs(coping[2].y - 12500.0) < 1.0);

    // True North orientation
    site.northAngle_deg = 35.5;
    assert(site.northAngle_deg == 35.5);

    // Spot elevations
    site.addSpotElevation(Point2D(0.0, -15000.0), 0.0, "Datum ±0.00");
    site.addSpotElevation(Point2D(0.0, 0.0), 250.0, "Finish Floor +250");
    assert(site.spotElevations.size() == 2);
    assert(site.spotElevations[0].elevation_mm == 0.0);
    assert(site.spotElevations[1].elevation_mm == 250.0);

    std::cout << "  -> Pool volume, coping, North angle, and spot elevations passed.\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "KaLara Arch - Unit Tests: Site Planning\n";
    std::cout << "========================================\n";

    testPropertyBoundaryAndMetrics();
    testSetbacksAndBuildableEnvelope();
    testRoadContextAndEntrances();
    testParkingAndLandscapeZones();
    testPoolOutdoorElementsAndOrientation();

    std::cout << "\nAll Site Planning unit tests passed successfully!\n";
    return 0;
}
