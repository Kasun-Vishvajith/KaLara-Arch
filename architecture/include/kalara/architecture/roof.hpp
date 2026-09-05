#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/metadata.hpp"
#include "kalara/architecture/semantics.hpp"
#include "kalara/core/geometry/point_vector.hpp"
#include "kalara/core/geometry/angle.hpp"
#include "kalara/core/geometry/ops.hpp"
#include "kalara/core/geometry/rect.hpp"
#include <string>
#include <vector>
#include <string_view>

namespace kalara::architecture {

/// Roof design style.
enum class RoofType {
    Flat,
    Gable,
    Hip,
    Shed,
    Mansard
};

constexpr std::string_view roofTypeName(RoofType type) noexcept {
    switch (type) {
        case RoofType::Flat:    return "Flat";
        case RoofType::Gable:   return "Gable";
        case RoofType::Hip:     return "Hip";
        case RoofType::Shed:    return "Shed";
        case RoofType::Mansard: return "Mansard";
    }
    return "Flat";
}

/// Architectural Roof definition (Step 12).
/// Represents roof footprint/eave outline, pitch angle, overhang, and ridge linework.
class Roof {
public:
    EntityId id;
    std::string name;
    RoofType type = RoofType::Gable;
    double pitch_deg = 22.5;         // Roof slope in degrees
    double overhang_mm = 600.0;       // Eave overhang beyond wall footprint in mm
    double fasciaHeight_mm = 200.0;   // Fascia edge thickness in mm
    std::vector<kalara::core::geometry::Point2D> wallFootprint; // Base wall outline in world mm
    std::vector<kalara::core::geometry::Point2D> eaveBoundary;  // Outer eave outline with overhang
    std::vector<kalara::core::geometry::Segment2D> ridgeLines;  // Ridge & hip lines in mm
    Metadata metadata;

    Roof()
        : id(EntityId::generate("roof")),
          name("Roof") {}

    Roof(std::string roofName, RoofType roofType, std::vector<kalara::core::geometry::Point2D> footprint,
         double pitch = 22.5, double overhang = 600.0)
        : id(EntityId::generate("roof")),
          name(std::move(roofName)),
          type(roofType),
          pitch_deg(pitch),
          overhang_mm(overhang),
          wallFootprint(std::move(footprint))
    {
        generateEavesAndRidges();
    }

    [[nodiscard]] SemanticType semanticType() const noexcept { return SemanticType::Roof; }

    /// Recompute eave boundary and ridge lines from current wallFootprint, overhang, and type.
    void generateEavesAndRidges() {
        if (wallFootprint.empty()) {
            return;
        }

        auto bbox = kalara::core::geometry::GeometricOps::boundingBox(wallFootprint);
        double minX = bbox.min.x - overhang_mm;
        double maxX = bbox.max.x + overhang_mm;
        double minY = bbox.min.y - overhang_mm;
        double maxY = bbox.max.y + overhang_mm;

        eaveBoundary = {
            kalara::core::geometry::Point2D(minX, minY),
            kalara::core::geometry::Point2D(maxX, minY),
            kalara::core::geometry::Point2D(maxX, maxY),
            kalara::core::geometry::Point2D(minX, maxY)
        };

        ridgeLines.clear();
        double midY = (minY + maxY) * 0.5;
        double midX = (minX + maxX) * 0.5;

        if (type == RoofType::Gable) {
            // Horizontal ridge along the longer axis
            ridgeLines.emplace_back(
                kalara::core::geometry::Point2D(minX, midY),
                kalara::core::geometry::Point2D(maxX, midY)
            );
        } else if (type == RoofType::Hip) {
            double hipOffset = std::min((maxX - minX) * 0.25, (maxY - minY) * 0.5);
            kalara::core::geometry::Point2D r1(minX + hipOffset, midY);
            kalara::core::geometry::Point2D r2(maxX - hipOffset, midY);
            // Center ridge
            ridgeLines.emplace_back(r1, r2);
            // 4 hip rafters to eave corners
            ridgeLines.emplace_back(r1, kalara::core::geometry::Point2D(minX, minY));
            ridgeLines.emplace_back(r1, kalara::core::geometry::Point2D(minX, maxY));
            ridgeLines.emplace_back(r2, kalara::core::geometry::Point2D(maxX, minY));
            ridgeLines.emplace_back(r2, kalara::core::geometry::Point2D(maxX, maxY));
        }
    }

    /// Base wall footprint area in square millimetres (mm^2).
    [[nodiscard]] double footprintArea_mm2() const noexcept {
        if (!wallFootprint.empty()) {
            return kalara::core::geometry::GeometricOps::polygonArea(wallFootprint);
        }
        return kalara::core::geometry::GeometricOps::polygonArea(eaveBoundary);
    }

    /// Actual sloped roof surface area in mm^2 (Step 12).
    [[nodiscard]] double surfaceArea_mm2() const noexcept {
        double planArea = footprintArea_mm2();
        double rad = pitch_deg * (3.14159265358979323846 / 180.0);
        double cosPitch = std::cos(rad);
        if (cosPitch <= 0.01) return planArea;
        return planArea / cosPitch;
    }

    /// Plan bounding box.
    [[nodiscard]] kalara::core::geometry::Rect2D boundingBox() const noexcept {
        return kalara::core::geometry::GeometricOps::boundingBox(eaveBoundary);
    }

    /// Hit-test check if world point is inside roof boundary.
    [[nodiscard]] bool containsPoint(const kalara::core::geometry::Point2D& p) const noexcept {
        return kalara::core::geometry::GeometricOps::pointInPolygon(p, eaveBoundary);
    }
};

} // namespace kalara::architecture
