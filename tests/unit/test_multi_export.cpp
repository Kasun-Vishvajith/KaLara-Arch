#include "kalara/exporters/svg_exporter.hpp"
#include "kalara/exporters/json_exporter.hpp"
#include "kalara/exporters/pdf_exporter.hpp"
#include "kalara/architecture/project.hpp"
#include "kalara/architecture/wall.hpp"
#include "kalara/architecture/opening.hpp"
#include "kalara/architecture/room.hpp"
#include "kalara/architecture/annotation.hpp"
#include "kalara/core/json.hpp"
#include <iostream>
#include <fstream>
#include <cassert>
#include <filesystem>

#ifdef _WIN32
#include <crtdbg.h>
#endif
#include <QGuiApplication>

using namespace kalara::architecture;
using namespace kalara::exporters;
using namespace kalara::core;
using namespace kalara::core::geometry;

static Project createSampleProject() {
    Project proj("Multi-Export Villa");
    auto* site = proj.defaultSite();
    site->propertyBoundary = {
        Point2D{0, 0}, Point2D{35000, 0}, Point2D{35000, 25000}, Point2D{0, 25000}
    };
    site->setbacks = Setbacks{5000.0, 4000.0, 3000.0, 3000.0};
    site->addRoad("Boulevard", Segment2D{Point2D{-5000, -2000}, Point2D{40000, -2000}}, 8000.0);

    auto& bld = site->addBuilding("Main Villa");
    auto& ground = bld.addLevel("Ground Floor", 0.0, 3200.0, LevelType::GroundFloor);

    auto& w1 = ground.addWall(Point2D{0, 0}, Point2D{6000, 0}, 200.0);
    auto& w2 = ground.addWall(Point2D{6000, 0}, Point2D{6000, 4000}, 200.0);
    auto& w3 = ground.addWall(Point2D{6000, 4000}, Point2D{0, 4000}, 200.0);
    auto& w4 = ground.addWall(Point2D{0, 4000}, Point2D{0, 0}, 200.0);

    ground.addDoor(w1.id, 2000.0, 900.0);
    ground.addWindow(w3.id, 3000.0, 1500.0);

    ground.addRoom("Living Hall", RoomType::LivingRoom, {
        Point2D{0, 0}, Point2D{6000, 0}, Point2D{6000, 4000}, Point2D{0, 4000}
    });

    ground.addDimensionForWall(w1, 500.0);
    ground.addNote(Point2D{3000, 2000}, "High ceiling area");

    return proj;
}

void testSvgExport() {
    std::cout << "[TEST] SVG Floor Plan & Site Export..." << std::endl;

    Project proj = createSampleProject();
    auto* ground = proj.defaultSite()->defaultBuilding()->defaultLevel();
    assert(ground != nullptr);

    ExportSettings settings;
    settings.scale = DrawingScale::Scale_1_100;
    settings.theme = ExportTheme::ColorPresentation;

    std::string svg = SvgExporter::exportLevelToSvg(*ground, settings);
    assert(!svg.empty());
    assert(svg.find("<?xml version=\"1.0\"") != std::string::npos);
    assert(svg.find("<svg xmlns=\"http://www.w3.org/2000/svg\"") != std::string::npos);
    assert(svg.find("class=\"wall\"") != std::string::npos);
    assert(svg.find("class=\"room\"") != std::string::npos);
    assert(svg.find("Living Hall") != std::string::npos);
    assert(svg.find("24.0 m²") != std::string::npos);
    assert(svg.find("door-leaf") != std::string::npos);
    assert(svg.find("class=\"dimension\"") != std::string::npos);
    assert(svg.find("Scale 1:100") != std::string::npos);

    // Site SVG
    std::string siteSvg = SvgExporter::exportSiteToSvg(*proj.defaultSite(), settings);
    assert(siteSvg.find("class=\"prop-boundary\"") != std::string::npos);
    assert(siteSvg.find("class=\"setback\"") != std::string::npos);
    assert(siteSvg.find("class=\"road\"") != std::string::npos);

    // Test file output
    std::string tempFile = "test_export.svg";
    bool saved = SvgExporter::exportLevelToFile(*ground, tempFile, settings);
    assert(saved);
    assert(std::filesystem::exists(tempFile));
    assert(std::filesystem::file_size(tempFile) > 500);
    std::filesystem::remove(tempFile);

    std::cout << "  -> Passed SVG export checks." << std::endl;
}

void testJsonInterchangeExport() {
    std::cout << "[TEST] Structured JSON Interchange Export..." << std::endl;

    Project proj = createSampleProject();
    std::string jsonStr = JsonInterchangeExporter::exportProjectToJson(proj, 2);
    assert(!jsonStr.empty());

    auto parsed = JsonValue::parse(jsonStr);
    assert(parsed.has_value() && parsed->isObject());

    const auto& root = *parsed;
    assert(root["format"].asString() == "KaLara-Interchange-JSON");
    assert(root["schema_version"].asString() == "1.0");
    assert(root["canonical_unit"].asString() == "millimetre");
    assert(root["unit_symbol"].asString() == "mm");

    assert(root["project"].isObject());
    assert(root["project"]["name"].asString() == "Multi-Export Villa");

    const auto& sites = root["project"]["sites"];
    assert(sites.isArray() && !sites.asArray().empty());

    const auto& site = sites.asArray().front();
    assert(site["property_boundary"].isArray());
    assert(site["buildings"].isArray() && !site["buildings"].asArray().empty());

    const auto& bld = site["buildings"].asArray().front();
    assert(bld["levels"].isArray() && !bld["levels"].asArray().empty());

    const auto& lvl = bld["levels"].asArray().front();
    assert(lvl["name"].asString() == "Ground Floor");
    assert(lvl["walls"].isArray() && lvl["walls"].asArray().size() == 4);
    assert(lvl["rooms"].isArray() && lvl["rooms"].asArray().size() == 1);
    assert(lvl["doors"].isArray() && lvl["doors"].asArray().size() == 1);
    assert(lvl["windows"].isArray() && lvl["windows"].asArray().size() == 1);
    assert(lvl["dimensions"].isArray() && lvl["dimensions"].asArray().size() == 1);

    // Test file output
    std::string tempFile = "test_export.json";
    bool saved = JsonInterchangeExporter::exportProjectToFile(proj, tempFile);
    assert(saved);
    assert(std::filesystem::exists(tempFile));
    assert(std::filesystem::file_size(tempFile) > 500);
    std::filesystem::remove(tempFile);

    std::cout << "  -> Passed JSON interchange checks." << std::endl;
}

void testPdfDocumentationExport() {
    std::cout << "[TEST] PDF Documentation Sheet Export..." << std::endl;

    Project proj = createSampleProject();
    auto* ground = proj.defaultSite()->defaultBuilding()->defaultLevel();
    assert(ground != nullptr);

    ExportSettings settings;
    settings.paperSize = PaperSize::A4;
    settings.orientation = SheetOrientation::Landscape;
    settings.scale = DrawingScale::Scale_1_100;

    std::string tempFile = "test_sheet.pdf";
    bool saved = PdfExporter::exportLevelToPdf(*ground, tempFile, settings);
    assert(saved);
    assert(std::filesystem::exists(tempFile));
    assert(std::filesystem::file_size(tempFile) > 1000);

    // Verify PDF header magic bytes "%PDF-"
    std::ifstream in(tempFile, std::ios::binary);
    char magic[5];
    in.read(magic, 5);
    in.close();
    assert(std::string(magic, 5) == "%PDF-");

    std::filesystem::remove(tempFile);
    std::cout << "  -> Passed PDF documentation checks." << std::endl;
}

void testScaleAwareSettings() {
    std::cout << "[TEST] Scale-Aware Documentation (1:50, 1:100, 1:200)..." << std::endl;

    Project proj = createSampleProject();
    auto* ground = proj.defaultSite()->defaultBuilding()->defaultLevel();

    ExportSettings s50;
    s50.scale = DrawingScale::Scale_1_50;
    std::string svg50 = SvgExporter::exportLevelToSvg(*ground, s50);
    assert(svg50.find("Scale 1:50") != std::string::npos);

    ExportSettings s200;
    s200.scale = DrawingScale::Scale_1_200;
    std::string svg200 = SvgExporter::exportLevelToSvg(*ground, s200);
    assert(svg200.find("Scale 1:200") != std::string::npos);

    std::cout << "  -> Passed scale-aware settings checks." << std::endl;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
#endif
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QGuiApplication app(argc, argv);

    std::cout << "=== KaLara Arch: SVG + PDF + JSON Export Tests ===" << std::endl;

    testSvgExport();
    testJsonInterchangeExport();
    testPdfDocumentationExport();
    testScaleAwareSettings();

    std::cout << "=== All Multi-Format Export Tests Passed Successfully! ===" << std::endl;
    return 0;
}
