#include "kalara/architecture/library_catalog.hpp"
#include <algorithm>
#include <cctype>

namespace kalara::architecture {

LibraryCatalog::LibraryCatalog() {
    populateBuiltinLibrary();
}

const LibraryItem* LibraryCatalog::findItem(const std::string& itemId) const noexcept {
    auto it = m_items.find(itemId);
    if (it != m_items.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<const LibraryItem*> LibraryCatalog::getItemsByCategory(LibraryCategory category) const {
    std::vector<const LibraryItem*> result;
    for (const auto& [id, item] : m_items) {
        if (item.category == category) {
            result.push_back(&item);
        }
    }
    std::sort(result.begin(), result.end(), [](const auto* a, const auto* b) {
        return a->name < b->name;
    });
    return result;
}

std::vector<const LibraryItem*> LibraryCatalog::searchItems(const std::string& query) const {
    std::vector<const LibraryItem*> result;
    if (query.empty()) {
        for (const auto& [id, item] : m_items) {
            result.push_back(&item);
        }
        return result;
    }

    auto toLower = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return s;
    };

    std::string qLower = toLower(query);

    for (const auto& [id, item] : m_items) {
        std::string idLower = toLower(item.id);
        std::string nameLower = toLower(item.name);
        if (idLower.find(qLower) != std::string::npos || nameLower.find(qLower) != std::string::npos) {
            result.push_back(&item);
        }
    }

    std::sort(result.begin(), result.end(), [](const auto* a, const auto* b) {
        return a->name < b->name;
    });
    return result;
}

bool LibraryCatalog::registerUserItem(LibraryItem item) {
    if (item.id.empty()) return false;
    item.isUserCreated = true;
    m_items[item.id] = std::move(item);
    return true;
}

void LibraryCatalog::populateBuiltinLibrary() {
    // -------------------------------------------------------------------------
    // 1. FURNITURE (Rule 11)
    // -------------------------------------------------------------------------
    {
        // BED_DOUBLE: 1600mm x 2000mm (Queen)
        LibraryItem bed("BED_DOUBLE", "Double Bed (Queen)", LibraryCategory::Furniture,
                        SemanticType::Furniture, 1600.0, 2000.0, 600.0);
        bed.clearance.front_mm = 600.0;
        bed.clearance.left_mm = 600.0;
        bed.clearance.right_mm = 600.0;
        bed.clearance.rear_mm = 0.0; // Wall-backed
        // Pillows detail linework
        bed.detailGeometry.push_back(SymbolPrimitive::makeLine({-700.0, 600.0}, {-100.0, 600.0}));
        bed.detailGeometry.push_back(SymbolPrimitive::makeLine({100.0, 600.0}, {700.0, 600.0}));
        bed.detailGeometry.push_back(SymbolPrimitive::makeLine({-700.0, 900.0}, {700.0, 900.0}));
        m_items[bed.id] = std::move(bed);
    }
    {
        // BED_SINGLE: 900mm x 1900mm
        LibraryItem bed("BED_SINGLE", "Single Bed", LibraryCategory::Furniture,
                        SemanticType::Furniture, 900.0, 1900.0, 600.0);
        bed.clearance.front_mm = 600.0;
        bed.clearance.left_mm = 600.0;
        bed.detailGeometry.push_back(SymbolPrimitive::makeLine({-350.0, 600.0}, {350.0, 600.0}));
        m_items[bed.id] = std::move(bed);
    }
    {
        // SOFA_3SEAT: 2200mm x 900mm
        LibraryItem sofa("SOFA_3SEAT", "Sofa (3-Seater)", LibraryCategory::Furniture,
                         SemanticType::Furniture, 2200.0, 900.0, 850.0);
        sofa.clearance.front_mm = 500.0;
        // Seat cushions
        sofa.detailGeometry.push_back(SymbolPrimitive::makeLine({-360.0, -450.0}, {-360.0, 250.0}));
        sofa.detailGeometry.push_back(SymbolPrimitive::makeLine({360.0, -450.0}, {360.0, 250.0}));
        m_items[sofa.id] = std::move(sofa);
    }
    {
        // TABLE_DINING_6: 1600mm x 900mm
        LibraryItem table("TABLE_DINING_6", "Dining Table (6-Seater)", LibraryCategory::Furniture,
                          SemanticType::Furniture, 1600.0, 900.0, 750.0);
        table.clearance.front_mm = 800.0; // Seating clearance
        table.clearance.rear_mm = 800.0;
        table.clearance.left_mm = 600.0;
        table.clearance.right_mm = 600.0;
        m_items[table.id] = std::move(table);
    }
    {
        // DESK_OFFICE: 1400mm x 700mm
        LibraryItem desk("DESK_OFFICE", "Office Desk", LibraryCategory::Furniture,
                         SemanticType::Furniture, 1400.0, 700.0, 750.0);
        desk.clearance.front_mm = 800.0; // Chair clearance
        m_items[desk.id] = std::move(desk);
    }

    // -------------------------------------------------------------------------
    // 2. BATHROOM FIXTURES
    // -------------------------------------------------------------------------
    {
        // TOILET_WC: 400mm x 700mm
        LibraryItem wc("TOILET_WC", "Water Closet / Toilet", LibraryCategory::BathroomFixtures,
                       SemanticType::Fixture, 400.0, 700.0, 800.0);
        wc.clearance.front_mm = 600.0; // Legroom clearance
        wc.clearance.left_mm = 250.0;
        wc.clearance.right_mm = 250.0;
        // Cistern line + bowl circle
        wc.detailGeometry.push_back(SymbolPrimitive::makeLine({-200.0, 150.0}, {200.0, 150.0}));
        wc.detailGeometry.push_back(SymbolPrimitive::makeCircle({0.0, -100.0}, 160.0));
        m_items[wc.id] = std::move(wc);
    }
    {
        // BATHTUB_STANDARD: 800mm x 1700mm
        LibraryItem tub("BATHTUB_STANDARD", "Standard Bathtub", LibraryCategory::BathroomFixtures,
                        SemanticType::Fixture, 800.0, 1700.0, 550.0);
        tub.clearance.left_mm = 700.0; // Step-in side
        tub.detailGeometry.push_back(SymbolPrimitive::makeCircle({0.0, -650.0}, 30.0)); // Drain
        m_items[tub.id] = std::move(tub);
    }
    {
        // SHOWER_CORNER: 900mm x 900mm
        LibraryItem shower("SHOWER_CORNER", "Corner Shower", LibraryCategory::BathroomFixtures,
                           SemanticType::Fixture, 900.0, 900.0, 2000.0);
        shower.clearance.front_mm = 700.0;
        shower.detailGeometry.push_back(SymbolPrimitive::makeCircle({0.0, 0.0}, 40.0)); // Center drain
        shower.detailGeometry.push_back(SymbolPrimitive::makeLine({-450.0, -450.0}, {450.0, 450.0}));
        shower.detailGeometry.push_back(SymbolPrimitive::makeLine({-450.0, 450.0}, {450.0, -450.0}));
        m_items[shower.id] = std::move(shower);
    }
    {
        // WASHBASIN: 600mm x 450mm
        LibraryItem basin("WASHBASIN", "Wash Basin / Vanity", LibraryCategory::BathroomFixtures,
                          SemanticType::Fixture, 600.0, 450.0, 850.0);
        basin.clearance.front_mm = 700.0;
        basin.detailGeometry.push_back(SymbolPrimitive::makeCircle({0.0, 0.0}, 180.0));
        m_items[basin.id] = std::move(basin);
    }

    // -------------------------------------------------------------------------
    // 3. KITCHEN OBJECTS
    // -------------------------------------------------------------------------
    {
        // SINK_DOUBLE: 1000mm x 600mm
        LibraryItem sink("SINK_DOUBLE", "Double Basin Kitchen Sink", LibraryCategory::KitchenObjects,
                         SemanticType::Fixture, 1000.0, 600.0, 850.0);
        sink.clearance.front_mm = 900.0; // Cook/prep clearance
        sink.detailGeometry.push_back(SymbolPrimitive::makeCircle({-230.0, 0.0}, 160.0));
        sink.detailGeometry.push_back(SymbolPrimitive::makeCircle({230.0, 0.0}, 160.0));
        m_items[sink.id] = std::move(sink);
    }
    {
        // COOKTOP_4BURNER: 750mm x 600mm
        LibraryItem cooktop("COOKTOP_4BURNER", "Cooktop (4-Burner)", LibraryCategory::KitchenObjects,
                            SemanticType::Fixture, 750.0, 600.0, 850.0);
        cooktop.clearance.front_mm = 900.0;
        cooktop.detailGeometry.push_back(SymbolPrimitive::makeCircle({-200.0, -120.0}, 80.0));
        cooktop.detailGeometry.push_back(SymbolPrimitive::makeCircle({200.0, -120.0}, 60.0));
        cooktop.detailGeometry.push_back(SymbolPrimitive::makeCircle({-200.0, 120.0}, 60.0));
        cooktop.detailGeometry.push_back(SymbolPrimitive::makeCircle({200.0, 120.0}, 90.0));
        m_items[cooktop.id] = std::move(cooktop);
    }
    {
        // REFRIGERATOR: 800mm x 700mm
        LibraryItem fridge("REFRIGERATOR", "Refrigerator", LibraryCategory::KitchenObjects,
                           SemanticType::Fixture, 800.0, 700.0, 1800.0);
        fridge.clearance.front_mm = 800.0; // Door swing clearance
        fridge.detailGeometry.push_back(SymbolPrimitive::makeLine({-400.0, 250.0}, {400.0, 250.0}));
        m_items[fridge.id] = std::move(fridge);
    }

    // -------------------------------------------------------------------------
    // 4. STAIRS (Section 40, Step 11)
    // -------------------------------------------------------------------------
    {
        // STAIR_STRAIGHT_RUN: 1000mm width x 3600mm run (e.g. 14 treads @ 260mm)
        LibraryItem stair("STAIR_STRAIGHT_RUN", "Straight Run Stair", LibraryCategory::Stairs,
                          SemanticType::Stair, 1000.0, 3600.0, 3000.0);
        stair.clearance.front_mm = 1000.0; // Landing bottom
        stair.clearance.rear_mm = 1000.0;  // Landing top
        // Draw step treads
        double stepLength = 3600.0 / 14.0;
        for (int i = 1; i < 14; ++i) {
            double y = -1800.0 + (i * stepLength);
            stair.detailGeometry.push_back(SymbolPrimitive::makeLine({-500.0, y}, {500.0, y}));
        }
        // Walkline & direction arrow
        stair.detailGeometry.push_back(SymbolPrimitive::makeLine({0.0, -1600.0}, {0.0, 1600.0}));
        stair.detailGeometry.push_back(SymbolPrimitive::makeLine({0.0, 1600.0}, {-120.0, 1400.0}));
        stair.detailGeometry.push_back(SymbolPrimitive::makeLine({0.0, 1600.0}, {120.0, 1400.0}));
        m_items[stair.id] = std::move(stair);
    }

    // -------------------------------------------------------------------------
    // 5. SITE / OUTDOOR OBJECTS (Rule 11)
    // -------------------------------------------------------------------------
    {
        // TREE_CANOPY: 3000mm x 3000mm circular canopy
        LibraryItem tree("TREE_CANOPY", "Landscape Tree (Canopy)", LibraryCategory::SiteOutdoor,
                         SemanticType::SiteElement, 3000.0, 3000.0, 4000.0);
        tree.detailGeometry.push_back(SymbolPrimitive::makeCircle({0.0, 0.0}, 1500.0));
        tree.detailGeometry.push_back(SymbolPrimitive::makeCircle({0.0, 0.0}, 200.0)); // Trunk
        m_items[tree.id] = std::move(tree);
    }
    {
        // PARKING_STALL_CAR: 2600mm x 5000mm standard car space
        LibraryItem carStall("PARKING_STALL_CAR", "Parking Stall (Sedan / SUV)", LibraryCategory::SiteOutdoor,
                             SemanticType::SiteElement, 2600.0, 5000.0, 1600.0);
        carStall.clearance.rear_mm = 6000.0; // Drive aisle clearance
        m_items[carStall.id] = std::move(carStall);
    }
}

} // namespace kalara::architecture
