#include "kalara/architecture/project_serializer.hpp"
#include "kalara/architecture/project.hpp"
#include "kalara/core/json.hpp"
#include <iostream>
#include <cassert>
#include <filesystem>

using namespace kalara::architecture;
using namespace kalara::core;
using namespace kalara::core::geometry;

void testJsonPrimitives() {
    std::cout << "[TEST] JSON serialization primitives and DOM parser..." << std::endl;

    // Test primitive creation and types
    JsonValue vNull(nullptr);
    assert(vNull.isNull());

    JsonValue vBool(true);
    assert(vBool.isBool() && vBool.asBool() == true);

    JsonValue vInt(static_cast<int64_t>(4200));
    assert(vInt.isInt() && vInt.asInt() == 4200);

    JsonValue vDouble(3.14159);
    assert(vDouble.isDouble() && std::abs(vDouble.asDouble() - 3.14159) < 0.0001);

    JsonValue vString("KaLara \"Arch\" \n Test\\Escape");
    assert(vString.isString());

    // Serialize and parse back
    std::string dumpedStr = vString.dump(0);
    auto parsedStr = JsonValue::parse(dumpedStr);
    assert(parsedStr.has_value() && parsedStr->isString());
    assert(parsedStr->asString() == "KaLara \"Arch\" \n Test\\Escape");

    // Complex Object
    JsonObject rootObj;
    rootObj["project"] = "Sample House";
    rootObj["version"] = 1;
    rootObj["scale"] = 0.02;
    rootObj["active"] = true;

    JsonArray wallsArr;
    JsonObject w1;
    w1["thickness"] = 200.0;
    w1["length"] = 5000.0;
    wallsArr.push_back(JsonValue(std::move(w1)));
    rootObj["walls"] = JsonValue(std::move(wallsArr));

    JsonValue rootVal(std::move(rootObj));
    std::string jsonDump = rootVal.dump(2);
    assert(!jsonDump.empty());

    auto parsedRoot = JsonValue::parse(jsonDump);
    assert(parsedRoot.has_value() && parsedRoot->isObject());
    assert((*parsedRoot)["project"].asString() == "Sample House");
    assert((*parsedRoot)["version"].asInt() == 1);
    assert((*parsedRoot)["active"].asBool() == true);
    assert((*parsedRoot)["walls"].isArray());
    assert((*parsedRoot)["walls"].asArray().size() == 1);
    assert((*parsedRoot)["walls"][0]["thickness"].asDouble() == 200.0);

    std::cout << "  -> JSON primitives and parser passed." << std::endl;
}

void testProjectRoundTrip() {
    std::cout << "[TEST] Full project round-trip serialization (.kla)..." << std::endl;

    // 1. Build a rich project
    Project proj("Villa Serene");
    proj.displayUnit = UnitSystem::Millimetres;
    proj.metadata.set("description", std::string("Two-story coastal modern villa"));
    proj.metadata.set("tag_0", std::string("coastal"));
    proj.metadata.set("tag_1", std::string("modern"));

    auto* site = proj.defaultSite();
    assert(site != nullptr);
    site->name = "Lot 42 - Palm Grove";
    site->propertyBoundary = {
        Point2D{0.0, 0.0},
        Point2D{30000.0, 0.0},
        Point2D{30000.0, 40000.0},
        Point2D{0.0, 40000.0}
    };
    site->setbacks = Setbacks{6000.0, 4500.0, 3000.0, 3000.0};
    site->northAngle_deg = 15.0;
    site->northArrowPosition = Point2D{25000.0, 35000.0};
    site->addSpotElevation(Point2D{5000.0, 5000.0}, 150.0, "FFL +150");

    // Road & Entrance
    site->addRoad("Ocean Drive", Segment2D{Point2D{-5000.0, -2000.0}, Point2D{35000.0, -2000.0}}, 12000.0);
    site->addEntrance("Main Gate", SiteEntranceType::VehicularDriveway, Point2D{10000.0, 0.0}, 4200.0);
    site->addParkingZone("Guest Parking", Point2D{15000.0, 3000.0}, 3);
    site->addLandscapeZone("Front Lawn", LandscapeType::Lawn, {
        Point2D{2000.0, 2000.0}, Point2D{8000.0, 2000.0}, Point2D{8000.0, 5000.0}, Point2D{2000.0, 5000.0}
    });
    site->addOutdoorElement("Infinity Pool", OutdoorElementType::SwimmingPool, {
        Point2D{20000.0, 25000.0}, Point2D{28000.0, 25000.0}, Point2D{28000.0, 32000.0}, Point2D{20000.0, 32000.0}
    }, 1800.0, 500.0);

    // Building & Levels
    auto& bld = site->addBuilding("Main Residence");
    auto& ground = bld.addLevel("Ground Floor", 0.0, 3200.0, LevelType::GroundFloor);

    // Walls on Ground Floor
    auto& w1 = ground.addWall(Point2D{5000.0, 8000.0}, Point2D{15000.0, 8000.0}, 250.0);
    w1.metadata.set("function", std::string("ExteriorLoadBearing"));
    w1.metadata.set("fire_rating", static_cast<int64_t>(60));

    auto& w2 = ground.addWall(Point2D{15000.0, 8000.0}, Point2D{15000.0, 16000.0}, 250.0);
    auto& w3 = ground.addWall(Point2D{15000.0, 16000.0}, Point2D{5000.0, 16000.0}, 250.0);
    auto& w4 = ground.addWall(Point2D{5000.0, 16000.0}, Point2D{5000.0, 8000.0}, 250.0);

    // Door & Window
    auto& door = ground.addDoor(w1.id, 2000.0, 1000.0, 2400.0, DoorSwing::LeftInswing);
    door.metadata.set("fire_rated", true);
    auto& win = ground.addWindow(w2.id, 3000.0, 1800.0, 1500.0, 900.0, WindowType::Casement);
    win.metadata.set("tempered", true);

    // Room
    auto& room = ground.addRoom("Living Hall", RoomType::LivingRoom, {
        Point2D{5000.0, 8000.0}, Point2D{15000.0, 8000.0}, Point2D{15000.0, 16000.0}, Point2D{5000.0, 16000.0}
    });
    room.metadata.set("floor_finish", std::string("Hardwood Oak"));

    // Dimension & Note
    ground.addDimensionForWall(w1, 600.0);
    ground.addNote(Point2D{10000.0, 12000.0}, "Main Entrance Foyer");

    // Furniture Instance
    LibraryItem bedItem("bed_king_01", "King Bed", LibraryCategory::Furniture, SemanticType::Furniture, 1930.0, 2030.0, 1200.0);
    bedItem.clearance = ClearanceProfile{800.0, 800.0, 600.0, 600.0};
    ground.addLibraryInstance(bedItem, Point2D{8000.0, 12000.0}, Angle::fromDegrees(90.0));

    // Constraints
    ground.addConstraint("Wall Corner Coincidence", ConstraintType::Coincidence, ConstraintSeverity::Hard, {w1.id, w2.id});

    // Level 2 (Roof Level)
    auto& roofLvl = bld.addRoofLevel("Roof", 2800.0);
    roofLvl.addRoof("Main Hip Roof", RoofType::Hip, {
        Point2D{4400.0, 7400.0}, Point2D{15600.0, 7400.0}, Point2D{15600.0, 16600.0}, Point2D{4400.0, 16600.0}
    }, 25.0, 600.0);

    // 2. Save to file
    std::filesystem::path testFile = "test_villa.kla";
    bool saved = ProjectSerializer::saveToFile(proj, testFile, 2);
    assert(saved);
    assert(std::filesystem::exists(testFile));

    // 3. Load from file
    std::string loadErr;
    auto loadedProj = ProjectSerializer::loadFromFile(testFile, &loadErr);
    if (!loadedProj) {
        std::cerr << "Load error: " << loadErr << std::endl;
    }
    assert(loadedProj != nullptr);

    // 4. Verify Semantic Equivalence
    bool equivalent = ProjectSerializer::areSemanticallyEquivalent(proj, *loadedProj, 0.01);
    assert(equivalent);

    // Detailed verification of loaded entities
    assert(loadedProj->name == "Villa Serene");
    assert(loadedProj->displayUnit == UnitSystem::Millimetres);

    auto* lSite = loadedProj->defaultSite();
    assert(lSite != nullptr);
    assert(lSite->name == "Lot 42 - Palm Grove");
    assert(lSite->propertyBoundary.size() == 4);
    assert(lSite->setbacks.front_mm == 6000.0);
    assert(lSite->roads().size() == 1);
    assert(lSite->entrances().size() == 1);
    assert(lSite->parkingZones().size() == 1);
    assert(lSite->landscapeZones().size() == 1);
    assert(lSite->outdoorElements().size() == 1);
    assert(lSite->outdoorElements()[0]->name == "Infinity Pool");

    assert(lSite->buildings().size() == 1);
    const auto* lBld = lSite->buildings()[0].get();
    assert(lBld->levels().size() == 2);

    const auto* lGround = lBld->levels()[0].get();
    assert(lGround->name == "Ground Floor");
    assert(lGround->walls().size() == 4);
    assert(lGround->doors().size() == 1);
    assert(lGround->doors()[0]->width_mm == 1000.0);
    assert(lGround->windows().size() == 1);
    assert(lGround->rooms().size() == 1);
    assert(lGround->dimensions().size() == 1);
    assert(lGround->notes().size() == 1);
    assert(lGround->libraryInstances().size() == 1);
    assert(lGround->libraryInstances()[0]->name == "King Bed");
    assert(lGround->constraints().size() == 1);

    const auto* lRoofLvl = lBld->levels()[1].get();
    assert(lRoofLvl->roofs().size() == 1);
    assert(lRoofLvl->roofs()[0]->name == "Main Hip Roof");
    assert(lRoofLvl->roofs()[0]->pitch_deg == 25.0);

    // Clean up temporary test file
    std::filesystem::remove(testFile);

    std::cout << "  -> Full project round-trip passed with semantic equivalence." << std::endl;
}

void testSchemaVersioningAndMigration() {
    std::cout << "[TEST] Schema version validation and migration foundation..." << std::endl;

    Project proj("Version Test");
    std::string jsonStr = ProjectSerializer::serializeToString(proj);

    // Parse into JSON object to tamper with version
    auto parsed = JsonValue::parse(jsonStr);
    assert(parsed.has_value());
    auto rootObj = parsed->asObject();

    // 1. Test future/unsupported schema version rejection
    rootObj["schema_version"] = 999;
    std::string errorMsg;
    auto invalidProj = ProjectSerializer::deserializeProject(rootObj, &errorMsg);
    assert(invalidProj == nullptr);
    assert(errorMsg.find("newer version") != std::string::npos);

    // 2. Test invalid format signature rejection
    rootObj["schema_version"] = 1;
    rootObj["format"] = "InvalidFormatXYZ";
    auto invalidSigProj = ProjectSerializer::deserializeProject(rootObj, &errorMsg);
    assert(invalidSigProj == nullptr);
    assert(errorMsg.find("Invalid or missing file format signature") != std::string::npos);

    // 3. Test migration pipeline hook
    rootObj["format"] = std::string(KLA_FORMAT_NAME);
    bool migrated = ProjectSerializer::migrateSchema(rootObj, 0, 1, &errorMsg);
    assert(migrated);
    assert(rootObj["schema_version"].asInt() == 1);

    std::cout << "  -> Schema versioning and migration tests passed." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "KaLara Arch - Unit Tests: Serialization" << std::endl;
    std::cout << "========================================" << std::endl;

    testJsonPrimitives();
    testProjectRoundTrip();
    testSchemaVersioningAndMigration();

    std::cout << "\nAll Serialization & Native Project unit tests passed successfully!\n";
    return 0;
}
