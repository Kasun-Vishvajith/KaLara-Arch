#include "kalara/architecture/annotation.hpp"
#include "kalara/architecture/wall.hpp"
#include "kalara/core/geometry/tolerances.hpp"
#include <iostream>
#include <cassert>

using namespace kalara::architecture;
using namespace kalara::core::geometry;

void testAssociativeDimensions() {
    std::cout << "Testing Associative Dimensions (Rule 7 & Step 08)..." << std::endl;
    // Base wall: 4000 mm long, from (0, 0) to (4000, 0)
    Wall wall(Point2D(0.0, 0.0), Point2D(4000.0, 0.0), 200.0);

    // Dimension associated with the wall
    Dimension dim(wall.id, wall, 500.0);
    assert(dim.id.isValid());
    assert(dim.semanticType() == SemanticType::Dimension);
    assert(dim.referencedEntityId.has_value());
    assert(dim.referencedEntityId.value() == wall.id);
    assert(Tolerances::equalLinear(dim.measuredDistance_mm(), 4000.0));
    assert(dim.formattedText(UnitSystem::Millimetres) == "4000 mm");

    // Dimension line points offset by +500 mm North (normal is (0, 1))
    auto [d1, d2] = dim.dimensionLine();
    assert(Tolerances::equalLinear(d1.x, 0.0) && Tolerances::equalLinear(d1.y, 500.0));
    assert(Tolerances::equalLinear(d2.x, 4000.0) && Tolerances::equalLinear(d2.y, 500.0));

    // Simulate Wall modification (e.g. wall lengthened to 4500 mm)
    wall.end = Point2D(4500.0, 0.0);
    assert(Tolerances::equalLinear(wall.length_mm(), 4500.0));

    // Re-sync dimension with changed wall geometry (Rule 7)
    dim.syncWithReferencedWall(wall);
    assert(Tolerances::equalLinear(dim.measuredDistance_mm(), 4500.0));
    assert(dim.formattedText(UnitSystem::Millimetres) == "4500 mm");

    auto [d1_new, d2_new] = dim.dimensionLine();
    assert(Tolerances::equalLinear(d2_new.x, 4500.0));

    std::cout << "-> Associative dimensions passed." << std::endl;
}

void testDrawingScaleAndNotes() {
    std::cout << "Testing Drawing Scales & Notes (Section 8)..." << std::endl;
    assert(Tolerances::equalLinear(scaleRatio(DrawingScale::Scale_1_50), 0.02));
    assert(Tolerances::equalLinear(scaleRatio(DrawingScale::Scale_1_100), 0.01));
    assert(Tolerances::equalLinear(scaleRatio(DrawingScale::Scale_1_200), 0.005));

    NoteAnnotation note(Point2D(100.0, 200.0), "Structural Column C1");
    assert(note.id.isValid());
    assert(note.semanticType() == SemanticType::Annotation);
    assert(note.text == "Structural Column C1");

    NorthArrow north;
    assert(north.id.isValid());
    assert(north.semanticType() == SemanticType::Annotation);
    assert(Tolerances::equalLinear(north.angleDegrees, 0.0));

    std::cout << "-> Drawing scales & notes passed." << std::endl;
}

int main() {
    std::cout << "=========================================" << std::endl;
    std::cout << "KaLara Arch Step 08: Dimensions & Annotations Test" << std::endl;
    std::cout << "=========================================" << std::endl;

    testAssociativeDimensions();
    testDrawingScaleAndNotes();

    std::cout << "=========================================" << std::endl;
    std::cout << "ALL DIMENSIONS & ANNOTATIONS TESTS PASSED (100%)" << std::endl;
    std::cout << "=========================================" << std::endl;
    return 0;
}
