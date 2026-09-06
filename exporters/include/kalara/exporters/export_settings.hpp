#pragma once
#include "kalara/architecture/annotation.hpp"
#include <string>

namespace kalara::exporters {

/// ISO Standard Paper Sizes for Documentation.
enum class PaperSize {
    A4,
    A3,
    A2,
    A1,
    Custom
};

/// Sheet layout orientation.
enum class SheetOrientation {
    Landscape,
    Portrait
};

/// Color presentation themes for architectural documentation.
enum class ExportTheme {
    ArchitecturalMonochrome,
    ColorPresentation,
    Blueprint
};

/// General documentation sheet and presentation settings.
struct ExportSettings {
    kalara::architecture::DrawingScale scale = kalara::architecture::DrawingScale::Scale_1_100;
    PaperSize paperSize = PaperSize::A4;
    SheetOrientation orientation = SheetOrientation::Landscape;
    ExportTheme theme = ExportTheme::ColorPresentation;

    bool includeTitleBlock = true;
    bool includeDimensions = true;
    bool includeRoomTags = true;
    bool includeDoorSwings = true;
    bool includeFurniture = true;
    bool includeRoofs = true;
    bool includeSite = true;
    bool includeNorthArrow = true;

    double margin_mm = 15.0; // Standard sheet margin in mm

    [[nodiscard]] std::pair<double, double> paperDimensions_mm() const noexcept {
        double w = 297.0;
        double h = 210.0;
        switch (paperSize) {
            case PaperSize::A4: w = 297.0; h = 210.0; break;
            case PaperSize::A3: w = 420.0; h = 297.0; break;
            case PaperSize::A2: w = 594.0; h = 420.0; break;
            case PaperSize::A1: w = 841.0; h = 594.0; break;
            case PaperSize::Custom: w = 297.0; h = 210.0; break;
        }
        if (orientation == SheetOrientation::Portrait) {
            std::swap(w, h);
        }
        return {w, h};
    }
};

} // namespace kalara::exporters
