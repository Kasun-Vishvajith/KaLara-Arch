#include "kalara/exporters/dxf_exporter.hpp"
#include "kalara/exporters/dxf_writer.hpp"
#include "kalara/architecture/project.hpp"
#include "kalara/architecture/wall.hpp"
#include "kalara/architecture/opening.hpp"
#include "kalara/architecture/room.hpp"
#include "kalara/architecture/annotation.hpp"
#include <iostream>
#include <fstream>
#include <cassert>
#include <filesystem>

using namespace kalara::architecture;
using namespace kalara::exporters;
using namespace kalara::core::geometry;

void testDxfHeaderAndUnits() {
    std::cout << "[TEST] DXF Header & Units ($INSUNITS = 4 for mm)..." << std::endl;

    DxfWriter writer;
    writer.writeHeader(Point2D{-5000, -5000}, Point2D{25000, 25000});
    writer.writeTables({{"0", DxfColor::White, "CONTINUOUS"}});
    writer.writeBlocks();
    writer.beginEntities();
    writer.endEntities();
    writer.writeEof();

    std::string dxf = writer.str();
    assert(dxf.find("$ACADVER\n1\nAC1009\n") != std::string::npos);
    // Principle 3: Canonical internal units = millimetre ($INSUNITS = 4)
    assert(dxf.find("$INSUNITS\n70\n4\n") != std::string::npos);
    assert(dxf.find("$MEASUREMENT\n70\n1\n") != std::string::npos);
    assert(dxf.find("$LUNITS\n70\n2\n") != std::string::npos);
    assert(dxf.find("0\nEOF\n") != std::string::npos);

    std::cout << "  -> Passed header and unit check." << std::endl;
}

void testLayerTableGeneration() {
    std::cout << "[TEST] DXF Layer Table Architectural Standards..." << std::endl;

    Project proj("Layer Test");
    std::string dxf = DxfExporter::exportProjectToDxf(proj);

    // Verify presence of AIA/NCS-compliant standard architectural CAD layers
    assert(dxf.find("2\nA-WALL\n") != std::string::npos);
    assert(dxf.find("2\nA-WALL-CNTR\n") != std::string::npos);
    assert(dxf.find("2\nA-DOOR\n") != std::string::npos);
    assert(dxf.find("2\nA-GLAZ\n") != std::string::npos);
    assert(dxf.find("2\nA-AREA\n") != std::string::npos);
    assert(dxf.find("2\nA-FLOR-FIXT\n") != std::string::npos);
    assert(dxf.find("2\nA-DIMS\n") != std::string::npos);
    assert(dxf.find("2\nA-ANNO-TEXT\n") != std::string::npos);
    assert(dxf.find("2\nC-PROP\n") != std::string::npos);
    assert(dxf.find("2\nC-SETB\n") != std::string::npos);
    assert(dxf.find("2\nC-ROAD\n") != std::string::npos);
    assert(dxf.find("2\nC-SITE\n") != std::string::npos);

    std::cout << "  -> Passed standard architectural layers check." << std::endl;
}

void testWallAndOpeningExport() {
    std::cout << "[TEST] Wall outlines and Door/Window DXF geometry..." << std::endl;

    Project proj("Wall Opening Test");
    auto* site = proj.defaultSite();
    auto& bld = site->addBuilding("Main Building");
    auto& ground = bld.addLevel("Ground", 0.0, 3000.0);

    // 5000 mm x 4000 mm room with 200 mm walls
    auto& w1 = ground.addWall(Point2D{0, 0}, Point2D{5000, 0}, 200.0);
    auto& w2 = ground.addWall(Point2D{5000, 0}, Point2D{5000, 4000}, 200.0);
    auto& w3 = ground.addWall(Point2D{5000, 4000}, Point2D{0, 4000}, 200.0);
    auto& w4 = ground.addWall(Point2D{0, 4000}, Point2D{0, 0}, 200.0);

    // Add door on south wall, window on east wall
    ground.addDoor(w1.id, 2500.0, 900.0);
    ground.addWindow(w2.id, 2000.0, 1200.0);

    DxfExportOptions options;
    options.exportWallOutlines = true;
    options.exportWallCenterlines = true;
    std::string dxf = DxfExporter::exportLevelToDxf(ground, options);

    // Verify wall outlines on A-WALL
    assert(dxf.find("8\nA-WALL\n") != std::string::npos);
    // Verify wall centerlines on A-WALL-CNTR
    assert(dxf.find("8\nA-WALL-CNTR\n") != std::string::npos);
    // Verify door entities on A-DOOR
    assert(dxf.find("8\nA-DOOR\n") != std::string::npos);
    assert(dxf.find("0\nARC\n") != std::string::npos); // Door swing arc
    // Verify window entities on A-GLAZ
    assert(dxf.find("8\nA-GLAZ\n") != std::string::npos);

    std::cout << "  -> Passed wall and opening DXF export." << std::endl;
}

void testRoomAndDimensionExport() {
    std::cout << "[TEST] Room polygon, area text, and dimension DXF entities..." << std::endl;

    Project proj("Room Dim Test");
    auto* site = proj.defaultSite();
    auto& bld = site->addBuilding("Main Building");
    auto& ground = bld.addLevel("Ground", 0.0, 3000.0);

    auto& w1 = ground.addWall(Point2D{0, 0}, Point2D{6000, 0}, 200.0);

    // Room with boundary
    ground.addRoom("Master Suite", RoomType::Bedroom, {
        Point2D{0, 0}, Point2D{6000, 0}, Point2D{6000, 4000}, Point2D{0, 4000}
    });

    // Linear dimension along wall
    ground.addDimensionForWall(w1, 600.0);
    ground.addNote(Point2D{3000, 2000}, "High ceiling area");

    std::string dxf = DxfExporter::exportLevelToDxf(ground);

    // Room polyline and text
    assert(dxf.find("8\nA-AREA\n") != std::string::npos);
    assert(dxf.find("Master Suite") != std::string::npos);
    assert(dxf.find("24.0 m2") != std::string::npos); // 6m * 4m = 24.0 m2

    // Dimension
    assert(dxf.find("8\nA-DIMS\n") != std::string::npos);
    assert(dxf.find("6000") != std::string::npos); // Dimension text length

    // Note annotation
    assert(dxf.find("8\nA-ANNO-TEXT\n") != std::string::npos);
    assert(dxf.find("High ceiling area") != std::string::npos);

    std::cout << "  -> Passed room and dimension DXF export." << std::endl;
}

void testSitePlanningExport() {
    std::cout << "[TEST] Site planning DXF entities (Property, Setback, Road, Pool)..." << std::endl;

    Site site("Export Site");
    site.propertyBoundary = {
        Point2D{0, 0}, Point2D{40000, 0}, Point2D{40000, 30000}, Point2D{0, 30000}
    };
    site.setbacks = Setbacks{6000.0, 4000.0, 3000.0, 3000.0};
    site.addRoad("Main Boulevard", Segment2D{Point2D{-5000, -2000}, Point2D{45000, -2000}}, 10000.0);
    site.addOutdoorElement("Lap Pool", OutdoorElementType::SwimmingPool, {
        Point2D{25000, 15000}, Point2D{35000, 15000}, Point2D{35000, 20000}, Point2D{25000, 20000}
    });

    std::string dxf = DxfExporter::exportSiteToDxf(site);

    assert(dxf.find("8\nC-PROP\n") != std::string::npos);
    assert(dxf.find("8\nC-SETB\n") != std::string::npos);
    assert(dxf.find("8\nC-ROAD\n") != std::string::npos);
    assert(dxf.find("8\nC-SITE\n") != std::string::npos);
    assert(dxf.find("Lap Pool") != std::string::npos);

    std::cout << "  -> Passed site planning DXF export." << std::endl;
}

void testFileExportAndScaleVerification() {
    std::cout << "[TEST] File writing and exact 1:1 millimetre coordinate verification..." << std::endl;

    Project proj("Scale Verification Project");
    auto* site = proj.defaultSite();
    auto& bld = site->addBuilding("Main Building");
    auto& ground = bld.addLevel("Ground", 0.0, 3000.0);

    // Wall of exact length 4800 mm
    ground.addWall(Point2D{1200.0, 3400.0}, Point2D{6000.0, 3400.0}, 200.0);

    std::string tempPath = "test_output.dxf";
    DxfExportOptions options;
    options.exportWallCenterlines = true;
    options.exportWallOutlines = true;
    bool saved = DxfExporter::exportProjectToFile(proj, tempPath, options);
    assert(saved);
    assert(std::filesystem::exists(tempPath));
    assert(std::filesystem::file_size(tempPath) > 500);

    // Verify exact coordinates in file (both centerline and wall face coordinates)
    std::ifstream in(tempPath);
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    assert(content.find("1200.00") != std::string::npos);
    assert(content.find("6000.00") != std::string::npos);
    assert(content.find("3400.00") != std::string::npos); // Centerline Y
    assert(content.find("3500.00") != std::string::npos); // Top face Y (3400 + 100)
    assert(content.find("3300.00") != std::string::npos); // Bottom face Y (3400 - 100)

    in.close();
    std::filesystem::remove(tempPath);
    std::cout << "  -> Passed file export and 1:1 scale verification." << std::endl;
}

int main() {
#ifdef _WIN32
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
#endif
    std::cout << "=== KaLara Arch: DXF Exporter Tests ===" << std::endl;

    testDxfHeaderAndUnits();
    testLayerTableGeneration();
    testWallAndOpeningExport();
    testRoomAndDimensionExport();
    testSitePlanningExport();
    testFileExportAndScaleVerification();

    std::cout << "=== All DXF Exporter Tests Passed Successfully! ===" << std::endl;
    return 0;
}
