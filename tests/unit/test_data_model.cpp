#include "kalara/architecture/project.hpp"
#include "kalara/architecture/site.hpp"
#include "kalara/architecture/building.hpp"
#include "kalara/architecture/level.hpp"
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/metadata.hpp"
#include "kalara/architecture/semantics.hpp"
#include "kalara/core/geometry/point_vector.hpp"
#include "kalara/core/geometry/ops.hpp"
#include <iostream>
#include <cassert>
#include <unordered_set>

using namespace kalara::architecture;
using namespace kalara::core::geometry;

void testEntityIds() {
    std::cout << "Testing Entity IDs (Rules 1 & 2)..." << std::endl;
    EntityId id1 = EntityId::generate("wall");
    EntityId id2 = EntityId::generate("wall");
    EntityId id3 = EntityId::generate("door");

    assert(id1.isValid());
    assert(id2.isValid());
    assert(id3.isValid());
    assert(id1 != id2);
    assert(id1.string().rfind("wall_", 0) == 0);
    assert(id3.string().rfind("door_", 0) == 0);

    std::unordered_set<EntityId> set;
    set.insert(id1);
    set.insert(id2);
    assert(set.size() == 2);
    assert(set.find(id1) != set.end());
    std::cout << "-> Entity ID tests passed." << std::endl;
}

void testMetadataAndSemantics() {
    std::cout << "Testing Extensible Metadata & Semantics (Principle 2)..." << std::endl;
    Metadata meta;
    meta.set("occupancy", int64_t(4));
    meta.set("target_area_m2", 180.5);
    meta.set("primary_material", std::string("Reinforced Concrete"));
    meta.set("is_insulated", true);

    assert(meta.has("occupancy"));
    assert(meta.has("target_area_m2"));
    assert(meta.getAs<int64_t>("occupancy").value() == 4);
    assert(meta.getAs<double>("target_area_m2").value() == 180.5);
    assert(meta.getAs<std::string>("primary_material").value() == "Reinforced Concrete");
    assert(meta.getAs<bool>("is_insulated").value() == true);

    // Semantics
    assert(semanticTypeName(SemanticType::Project) == "Project");
    assert(semanticTypeName(SemanticType::Level) == "Level");
    assert(semanticTypeName(SemanticType::Wall) == "Wall");
    std::cout << "-> Metadata & Semantics tests passed." << std::endl;
}

void testModelHierarchy() {
    std::cout << "Testing Project -> Site -> Building -> Level Hierarchy (Principle 5 & Section 6)..." << std::endl;
    Project project("KaLara Villa Project");
    assert(project.id.isValid());
    assert(project.name == "KaLara Villa Project");
    assert(project.semanticType() == SemanticType::Project);

    // Site
    Site* site = project.defaultSite();
    assert(site != nullptr);
    assert(site->id.isValid());
    assert(site->semanticType() == SemanticType::Site);

    // Set site property boundary polygon (20m x 30m = 20,000 mm x 30,000 mm = 600 m^2)
    site->propertyBoundary = {
        Point2D(0.0, 0.0),
        Point2D(20000.0, 0.0),
        Point2D(20000.0, 30000.0),
        Point2D(0.0, 30000.0)
    };
    double siteArea = GeometricOps::polygonArea(site->propertyBoundary);
    assert(siteArea == 600000000.0); // 600 m^2 in mm^2

    // Building
    Building& building = site->addBuilding("Main Residential Block");
    assert(building.id.isValid());
    assert(building.semanticType() == SemanticType::Building);

    // Multi-level with canonical elevations in mm (Principle 5)
    Level& ground = building.addLevel("Ground Floor", 0.0, 3200.0);
    Level& first = building.addLevel("First Floor", 3200.0, 3000.0);
    Level& roof = building.addLevel("Roof Level", 6200.0, 1000.0);

    assert(ground.id.isValid());
    assert(first.id.isValid());
    assert(roof.id.isValid());
    assert(ground.id != first.id);

    assert(ground.elevation_mm == 0.0);
    assert(first.elevation_mm == 3200.0);
    assert(roof.elevation_mm == 6200.0);

    // Find level by ID
    Level* foundFirst = building.findLevel(first.id);
    assert(foundFirst != nullptr);
    assert(foundFirst->name == "First Floor");

    // Level metadata
    ground.metadata.set("finish_floor_level_mm", 150.0);
    assert(ground.metadata.getAs<double>("finish_floor_level_mm").value() == 150.0);

    assert(building.levels().size() == 3);
    std::cout << "-> Hierarchy tests passed." << std::endl;
}

int main() {
    std::cout << "=========================================" << std::endl;
    std::cout << "KaLara Arch Step 03: Data Model Test" << std::endl;
    std::cout << "=========================================" << std::endl;

    testEntityIds();
    testMetadataAndSemantics();
    testModelHierarchy();

    std::cout << "=========================================" << std::endl;
    std::cout << "ALL ARCHITECTURAL DATA MODEL TESTS PASSED" << std::endl;
    std::cout << "=========================================" << std::endl;
    return 0;
}
