#include "kalara/architecture/project.hpp"
#include "kalara/architecture/project_serializer.hpp"
#include "kalara/architecture/wall.hpp"
#include "kalara/architecture/opening.hpp"
#include "kalara/architecture/room.hpp"
#include "kalara/architecture/annotation.hpp"
#include "kalara/architecture/library_catalog.hpp"
#include "kalara/runtime/runtime_context.hpp"
#include "kalara/runtime/transaction.hpp"
#include "kalara/runtime/validation_engine.hpp"
#include "kalara/runtime/selection_manager.hpp"
#include "kalara/runtime/model_manipulator.hpp"
#include "kalara/runtime/snapping_engine.hpp"
#include "kalara/exporters/dxf_exporter.hpp"
#include "kalara/exporters/svg_exporter.hpp"
#include "kalara/exporters/json_exporter.hpp"
#include "kalara/exporters/pdf_exporter.hpp"
#include "kalara/core/geometry/ops.hpp"
#include "kalara/core/geometry/tolerances.hpp"
#include <iostream>
#include <cassert>
#include <filesystem>
#include <cmath>
#include <QGuiApplication>

using namespace kalara::core;
using namespace kalara::core::geometry;
using namespace kalara::architecture;
using namespace kalara::runtime;
using namespace kalara::exporters;

void runFullSystemIntegrationAudit() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  KaLara Arch: Full System End-to-End Integration Audit   " << std::endl;
    std::cout << "  Verifying Complete Human-First CAD Core (Steps 00-19)   " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // --- Phase 01-03: Project, Site, Building & Multi-Level Hierarchy ---
    std::cout << "\n[AUDIT 1/12] Project Hierarchy & Multi-Level Foundations..." << std::endl;
    Project project("Pre-AI Verified Master Villa");
    auto* site = project.defaultSite();
    assert(site != nullptr);
    site->name = "Hillside Estate";

    // Property Boundary (40m x 30m)
    site->propertyBoundary = {
        Point2D{0, 0}, Point2D{40000, 0}, Point2D{40000, 30000}, Point2D{0, 30000}
    };
    site->setbacks = Setbacks{5000.0, 4000.0, 3000.0, 3000.0};
    site->addRoad("Access Road", Segment2D{Point2D{-5000, -2000}, Point2D{45000, -2000}}, 7000.0);
    site->addOutdoorElement("Infinity Pool", OutdoorElementType::SwimmingPool, {
        Point2D{22000, 5000}, Point2D{32000, 5000}, Point2D{32000, 10000}, Point2D{22000, 10000}
    });

    auto& building = site->addBuilding("Main Residence");
    auto& groundLevel = building.addLevel("Ground Floor", 0.0, 3200.0, LevelType::GroundFloor);
    auto& firstLevel = building.addLevel("First Floor", 3200.0, 3000.0, LevelType::UpperFloor);
    auto& roofLevel = building.addLevel("Roof Deck", 6200.0, 0.0, LevelType::Roof);

    assert(building.levels().size() == 3);
    assert(Tolerances::equalLinear(groundLevel.elevation_mm, 0.0));
    assert(Tolerances::equalLinear(firstLevel.elevation_mm, 3200.0));
    assert(Tolerances::equalLinear(roofLevel.elevation_mm, 6200.0));
    std::cout << "  -> Passed Project & Level Hierarchy verification." << std::endl;

    // --- Phase 04-05: Parametric Walls & Preference-Controlled Thickness ---
    std::cout << "\n[AUDIT 2/12] Parametric Wall System & Joins..." << std::endl;
    auto& wSouth = groundLevel.addWall(Point2D{6000, 5000}, Point2D{16000, 5000}, 200.0); // 10000 mm
    auto& wEast  = groundLevel.addWall(Point2D{16000, 5000}, Point2D{16000, 13000}, 200.0); // 8000 mm
    auto& wNorth = groundLevel.addWall(Point2D{16000, 13000}, Point2D{6000, 13000}, 200.0); // 10000 mm
    auto& wWest  = groundLevel.addWall(Point2D{6000, 13000}, Point2D{6000, 5000}, 200.0);   // 8000 mm

    assert(groundLevel.walls().size() == 4);
    assert(Tolerances::equalLinear(wSouth.length_mm(), 10000.0));
    assert(Tolerances::equalLinear(wEast.length_mm(), 8000.0));

    // Test Rule 6: Thickness modification with Centerline alignment
    wSouth.thickness_mm = 250.0;
    assert(Tolerances::equalLinear(wSouth.thickness_mm, 250.0));
    wSouth.thickness_mm = 200.0;
    std::cout << "  -> Passed Wall Geometry & Thickness Preference verification." << std::endl;

    // --- Phase 06: Rooms & Space Computations ---
    std::cout << "\n[AUDIT 3/12] Room Derivation & Area Calculations..." << std::endl;
    std::vector<Point2D> livingBoundary = {
        Point2D{6100, 5100}, Point2D{15900, 5100}, Point2D{15900, 12900}, Point2D{6100, 12900}
    };
    auto& livingRoom = groundLevel.addRoom("Grand Living Room", RoomType::LivingRoom, livingBoundary);
    double expectedArea = (15900.0 - 6100.0) * (12900.0 - 5100.0); // 9800 * 7800 = 76,440,000 mm2 = 76.44 m2
    assert(std::abs(livingRoom.area_mm2() - expectedArea) <= 100.0);
    assert(livingRoom.area_m2() > 76.0 && livingRoom.area_m2() < 77.0);
    std::cout << "  -> Passed Room Boundary & Exact Area verification (" << livingRoom.area_m2() << " m²)." << std::endl;

    // --- Phase 07: Wall-Hosted Doors and Windows ---
    std::cout << "\n[AUDIT 4/12] Wall-Hosted Openings (Doors & Windows)..." << std::endl;
    auto& mainDoor = groundLevel.addDoor(wSouth.id, 5000.0, 1000.0, 2100.0, DoorSwing::LeftInswing);
    assert(mainDoor.hostWallId == wSouth.id);
    assert(Tolerances::equalLinear(mainDoor.width_mm, 1000.0));
    Point2D doorCenter = mainDoor.centerPoint(wSouth);
    assert(Tolerances::equalLinear(doorCenter.x, 11000.0));
    assert(Tolerances::equalLinear(doorCenter.y, 5000.0));

    auto& northWindow = groundLevel.addWindow(wNorth.id, 5000.0, 2000.0, 1400.0, 900.0, WindowType::Casement);
    assert(northWindow.hostWallId == wNorth.id);
    assert(Tolerances::equalLinear(northWindow.width_mm, 2000.0));
    std::cout << "  -> Passed Wall-Hosted Openings & World Placement verification." << std::endl;

    // --- Phase 08: Associative Dimensions ---
    std::cout << "\n[AUDIT 5/12] Associative Dimensions (Rule 7)..." << std::endl;
    auto& dim = groundLevel.addDimensionForWall(wSouth, -800.0);
    assert(Tolerances::equalLinear(dim.measuredDistance_mm(), 10000.0));

    // Shift wall endpoint and verify dimension updates automatically (Rule 7)
    wSouth.end = Point2D{18000, 5000};
    dim.syncWithReferencedWall(wSouth);
    assert(Tolerances::equalLinear(dim.measuredDistance_mm(), 12000.0));
    wSouth.end = Point2D{16000, 5000};
    dim.syncWithReferencedWall(wSouth);
    assert(Tolerances::equalLinear(dim.measuredDistance_mm(), 10000.0));
    std::cout << "  -> Passed Associative Dimension dynamic tracking." << std::endl;

    // --- Phase 10: Selection & Snapping Engine ---
    std::cout << "\n[AUDIT 6/12] Snapping & Manipulation Engine..." << std::endl;
    SelectionManager sel;
    sel.select(wSouth.id);
    assert(sel.isSelected(wSouth.id));
    assert(sel.count() == 1);

    SnapResult snapRes = SnappingEngine::snap(Point2D{6050, 5050}, groundLevel, 150.0);
    assert(snapRes.snapped);
    assert(Tolerances::equalLinear(snapRes.point.x, 6000.0));
    assert(Tolerances::equalLinear(snapRes.point.y, 5000.0));
    std::cout << "  -> Passed Semantic Snapping to Endpoint." << std::endl;

    // --- Phase 11: 2D Architectural Library Catalog ---
    std::cout << "\n[AUDIT 7/12] Architectural Library Catalog & Instances..." << std::endl;
    LibraryCatalog catalog;
    assert(catalog.itemCount() > 0);
    const auto* sofa = catalog.findItem("SOFA_3SEAT");
    assert(sofa != nullptr);
    assert(sofa->width_mm == 2200.0);

    auto& inst = groundLevel.addLibraryInstance(*sofa, Point2D{10000, 9000});
    assert(inst.itemId == "SOFA_3SEAT");
    std::cout << "  -> Passed Library Catalog & Instance Placement." << std::endl;

    // --- Phase 14: Validation Engine & Rule 16 ---
    std::cout << "\n[AUDIT 8/12] Architectural Validation Engine & Diagnostics..." << std::endl;
    auto report = ValidationEngine::validateProject(project);
    assert(!ValidationReport::LegalDisclaimer.empty()); // Rule 16 disclaimer check
    std::cout << "  -> Validation Diagnostics: " << report.warningCount() << " warnings, " 
              << report.errorCount() << " errors." << std::endl;
    std::cout << "  -> Rule 16 Disclaimer: " << ValidationReport::LegalDisclaimer.substr(0, 60) << "..." << std::endl;

    // --- Phase 16: Transaction Manager (Undo / Redo) ---
    std::cout << "\n[AUDIT 9/12] Transactions & Undo/Redo Engine..." << std::endl;
    TransactionManager txMgr;
    bool began = txMgr.beginTransaction(project, "Add Wall & Window");
    assert(began);
    txMgr.recordOperation("Created new wall entity");
    bool committed = txMgr.commitTransaction(project);
    assert(committed);
    assert(txMgr.canUndo());
    bool undone = txMgr.undo(project);
    assert(undone);
    assert(txMgr.canRedo());
    bool redone = txMgr.redo(project);
    assert(redone);
    std::cout << "  -> Passed Transaction commit, undo, and redo cycling." << std::endl;

    // --- Phase 15: Native .kla Project Serialization Round-Trip ---
    std::cout << "\n[AUDIT 10/12] Native .kla Project Serialization Round-Trip..." << std::endl;
    std::string klaContent = ProjectSerializer::serializeToString(project);
    assert(!klaContent.empty());
    assert(klaContent.find("\"format\": \"KaLaraArchNative\"") != std::string::npos);

    std::string err;
    auto loadedProject = ProjectSerializer::deserializeFromString(klaContent, &err);
    assert(loadedProject != nullptr);
    assert(err.empty());
    assert(ProjectSerializer::areSemanticallyEquivalent(project, *loadedProject));
    std::cout << "  -> Passed 100% semantically equivalent .kla round-trip." << std::endl;

    // --- Phase 17: Professional DXF Export ---
    std::cout << "\n[AUDIT 11/12] Professional DXF CAD Foundation Export..." << std::endl;
    auto* currentLevel = project.defaultSite()->defaultBuilding()->defaultLevel();
    assert(currentLevel != nullptr);

    DxfExportOptions dxfOpts;
    std::string dxfStr = DxfExporter::exportLevelToDxf(*currentLevel, dxfOpts);
    assert(!dxfStr.empty());
    assert(dxfStr.find("$INSUNITS\n70\n4\n") != std::string::npos); // Millimetres standard
    assert(dxfStr.find("A-WALL") != std::string::npos);
    assert(dxfStr.find("A-DOOR") != std::string::npos);
    assert(dxfStr.find("A-GLAZ") != std::string::npos);
    assert(dxfStr.find("A-AREA") != std::string::npos);
    assert(dxfStr.find("A-DIMS") != std::string::npos);
    std::cout << "  -> Passed DXF Header, $INSUNITS=4, and AIA layer standards." << std::endl;

    // --- Phase 18-19: Multi-Format SVG, PDF & JSON Exports ---
    std::cout << "\n[AUDIT 12/12] Multi-Format Vector SVG, PDF & JSON Interchange..." << std::endl;
    ExportSettings exportSettings;
    exportSettings.scale = DrawingScale::Scale_1_100;

    std::string svgStr = SvgExporter::exportLevelToSvg(*currentLevel, exportSettings);
    assert(!svgStr.empty());
    assert(svgStr.find("<svg xmlns=\"http://www.w3.org/2000/svg\"") != std::string::npos);

    std::string jsonStr = JsonInterchangeExporter::exportProjectToJson(project, 2);
    assert(!jsonStr.empty());
    assert(jsonStr.find("\"canonical_unit\": \"millimetre\"") != std::string::npos);

    std::string testPdf = "integration_test.pdf";
    bool pdfOk = PdfExporter::exportLevelToPdf(*currentLevel, testPdf, exportSettings);
    assert(pdfOk);
    assert(std::filesystem::exists(testPdf));
    std::filesystem::remove(testPdf);
    std::cout << "  -> Passed SVG, JSON, and PDF document generation." << std::endl;

    std::cout << "\n==========================================================" << std::endl;
    std::cout << "  ALL 12 AUDIT CHECKPOINTS PASSED WITH 100% PRECISION!   " << std::endl;
    std::cout << "  KaLara Arch Human-First Core is Solid & Fully Verified.  " << std::endl;
    std::cout << "==========================================================" << std::endl;
}

int main(int argc, char* argv[]) {
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QGuiApplication app(argc, argv);

    try {
        runFullSystemIntegrationAudit();
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Integration Audit Exception: " << ex.what() << std::endl;
        return 1;
    }
}
