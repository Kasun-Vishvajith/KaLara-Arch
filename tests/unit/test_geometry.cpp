#include "kalara/core/geometry/tolerances.hpp"
#include "kalara/core/geometry/units.hpp"
#include "kalara/core/geometry/angle.hpp"
#include "kalara/core/geometry/point_vector.hpp"
#include "kalara/core/geometry/transform.hpp"
#include "kalara/core/geometry/segment.hpp"
#include "kalara/core/geometry/rect.hpp"
#include "kalara/core/geometry/ops.hpp"
#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>

using namespace kalara::core::geometry;

void testUnitsAndPrecision() {
    std::cout << "Testing Units & 1 mm Precision..." << std::endl;
    // Canonical unit is millimetre
    double wallLength_mm = 4200.0; // 4200 mm
    
    // Display conversions
    assert(Tolerances::equalLinear(UnitConverter::toDisplay(wallLength_mm, UnitSystem::Metres), 4.2));
    assert(Tolerances::equalLinear(UnitConverter::toDisplay(wallLength_mm, UnitSystem::Centimetres), 420.0));
    assert(Tolerances::equalLinear(UnitConverter::fromDisplay(4.2, UnitSystem::Metres), 4200.0));
    assert(Tolerances::equalLinear(UnitConverter::fromDisplay(100.0, UnitSystem::Inches), 2540.0));

    // String formatting
    std::string formatted = UnitConverter::format(wallLength_mm, UnitSystem::Metres, 2);
    assert(formatted == "4.20 m");

    // Precision tolerance
    assert(Tolerances::equalLinear(1000.0, 1000.00001));
    assert(!Tolerances::equalLinear(1000.0, 1001.0));
    std::cout << "-> Units & Precision tests passed." << std::endl;
}

void testPointsAndVectors() {
    std::cout << "Testing Points & Vectors..." << std::endl;
    Point2D p1(1000.0, 2000.0);
    Point2D p2(4000.0, 6000.0);

    // Distance calculation: 3-4-5 triangle * 1000 = 5000 mm
    double dist = p1.distanceTo(p2);
    assert(Tolerances::equalLinear(dist, 5000.0));
    assert(Tolerances::equalLinear(p1.distanceSquaredTo(p2), 25000000.0));

    Vector2D v = p2 - p1;
    assert(Tolerances::equalLinear(v.dx, 3000.0));
    assert(Tolerances::equalLinear(v.dy, 4000.0));
    assert(Tolerances::equalLinear(v.length(), 5000.0));

    Vector2D vNorm = v.normalized();
    assert(Tolerances::equalLinear(vNorm.length(), 1.0));
    assert(Tolerances::equalLinear(vNorm.dx, 0.6));
    assert(Tolerances::equalLinear(vNorm.dy, 0.8));

    // Perpendicular vector
    Vector2D vPerp = vNorm.perpendicular();
    assert(Tolerances::equalLinear(vNorm.dot(vPerp), 0.0));

    // Point translation by vector
    Point2D p3 = p1 + v;
    assert(p3 == p2);
    std::cout << "-> Points & Vectors tests passed." << std::endl;
}

void testAnglesAndOrthogonality() {
    std::cout << "Testing Angles & Orthogonality (Rule 14)..." << std::endl;
    Angle zero = Angle::fromDegrees(0.0);
    Angle right = Angle::fromDegrees(90.0);
    Angle straight = Angle::fromDegrees(180.0);
    Angle oblique = Angle::fromDegrees(45.0);

    assert(zero.isOrthogonal());
    assert(right.isOrthogonal());
    assert(straight.isOrthogonal());
    assert(Angle::fromDegrees(270.0).isOrthogonal());
    assert(Angle::fromDegrees(360.0).isOrthogonal());
    assert(!oblique.isOrthogonal());

    // Normalized
    Angle neg = Angle::fromDegrees(-90.0).normalized();
    assert(Tolerances::equalAngular(neg.degrees(), 270.0));
    assert(neg.isOrthogonal());
    std::cout << "-> Angles & Orthogonality tests passed." << std::endl;
}

void testSegmentsAndIntersections() {
    std::cout << "Testing Line Segments & Intersections..." << std::endl;
    Segment2D seg1(Point2D(0.0, 0.0), Point2D(5000.0, 0.0)); // 5000 mm horizontal
    assert(Tolerances::equalLinear(seg1.length(), 5000.0));
    assert(seg1.midpoint() == Point2D(2500.0, 0.0));
    assert(seg1.isOrthogonal());

    // Closest point and projection
    Point2D p(2500.0, 1500.0);
    assert(seg1.closestPoint(p) == Point2D(2500.0, 0.0));
    assert(Tolerances::equalLinear(seg1.distanceToPoint(p), 1500.0));

    // Outside bounds clamping
    Point2D pOutside(7000.0, 1000.0);
    assert(seg1.closestPoint(pOutside) == Point2D(5000.0, 0.0));

    // Crossing segment intersection
    Segment2D seg2(Point2D(2500.0, -1000.0), Point2D(2500.0, 3000.0));
    auto inter = seg1.intersect(seg2);
    assert(inter.has_value());
    assert(inter.value() == Point2D(2500.0, 0.0));

    // Parallel disjoint segments
    Segment2D seg3(Point2D(0.0, 1000.0), Point2D(5000.0, 1000.0));
    assert(!seg1.intersect(seg3).has_value());
    std::cout << "-> Segments & Intersections tests passed." << std::endl;
}

void testTransforms() {
    std::cout << "Testing 2D Transforms..." << std::endl;
    Point2D origin(0.0, 0.0);
    Point2D p(1000.0, 0.0);

    // Translation
    Transform2D tTrans = Transform2D::translation(500.0, 200.0);
    Point2D pTrans = tTrans.map(p);
    assert(pTrans == Point2D(1500.0, 200.0));

    // Rotation 90 degrees CCW
    Transform2D tRot = Transform2D::rotation(Angle::fromDegrees(90.0));
    Point2D pRot = tRot.map(p);
    assert(pRot == Point2D(0.0, 1000.0));

    // Composite transform
    Transform2D tComp = tTrans * tRot;
    Point2D pComp = tComp.map(p);
    assert(pComp == Point2D(500.0, 1200.0));

    // Inversion
    Transform2D tInv = tComp.inverted();
    Point2D pRestored = tInv.map(pComp);
    assert(pRestored.coincidesWith(p));
    std::cout << "-> Transforms tests passed." << std::endl;
}

void testRectanglesAndOps() {
    std::cout << "Testing Rectangles & Geometric Operations..." << std::endl;
    // 4000 mm x 3000 mm Room Box
    Rect2D room(1000.0, 1000.0, 4000.0, 3000.0);
    assert(Tolerances::equalLinear(room.width(), 4000.0));
    assert(Tolerances::equalLinear(room.height(), 3000.0));
    assert(Tolerances::equalLinear(room.area(), 12000000.0)); // 12 m^2
    assert(Tolerances::equalLinear(room.perimeter(), 14000.0)); // 14 m
    assert(room.center() == Point2D(3000.0, 2500.0));

    assert(room.contains(Point2D(2000.0, 2000.0)));
    assert(!room.contains(Point2D(6000.0, 2000.0)));

    // Polygon Shoelace Area test: 5000 mm x 4000 mm polygon = 20,000,000 mm^2 (20 m^2)
    std::vector<Point2D> poly = {
        Point2D(0.0, 0.0),
        Point2D(5000.0, 0.0),
        Point2D(5000.0, 4000.0),
        Point2D(0.0, 4000.0)
    };
    double area = GeometricOps::polygonArea(poly);
    assert(Tolerances::equalLinear(area, 20000000.0));
    assert(Tolerances::equalLinear(GeometricOps::polygonPerimeter(poly), 18000.0));
    assert(GeometricOps::pointInPolygon(Point2D(2500.0, 2000.0), poly));
    assert(!GeometricOps::pointInPolygon(Point2D(6000.0, 2000.0), poly));

    // Snapping tests
    Point2D ref(1000.0, 1000.0);
    Point2D drawn(3500.0, 1200.0);
    Point2D snappedOrtho = GeometricOps::snapToOrthogonal(ref, drawn);
    assert(snappedOrtho == Point2D(3500.0, 1000.0));

    Point2D freePt(1234.0, 5678.0);
    Point2D snappedGrid = GeometricOps::snapToGrid(freePt, 100.0); // 100 mm grid
    assert(snappedGrid == Point2D(1200.0, 5700.0));

    std::cout << "-> Rectangles & Geometric Operations tests passed." << std::endl;
}

int main() {
    std::cout << "=========================================" << std::endl;
    std::cout << "KaLara Arch Step 02: Geometry & Units Test" << std::endl;
    std::cout << "=========================================" << std::endl;

    testUnitsAndPrecision();
    testPointsAndVectors();
    testAnglesAndOrthogonality();
    testSegmentsAndIntersections();
    testTransforms();
    testRectanglesAndOps();

    std::cout << "=========================================" << std::endl;
    std::cout << "ALL NUMERICAL GEOMETRY TESTS PASSED (100%)" << std::endl;
    std::cout << "=========================================" << std::endl;
    return 0;
}
