#pragma once
#include "kalara/architecture/project.hpp"
#include "kalara/architecture/level.hpp"
#include "kalara/architecture/site.hpp"
#include "kalara/exporters/export_settings.hpp"
#include <string>

namespace kalara::exporters {

/// PDF Architectural Drawing Sheet Exporter (Step 18 & Section 8).
/// Produces publication-ready PDF sheets (A4, A3, Landscape/Portrait) with title block, sheet scale, and vector linework.
class PdfExporter {
public:
    /// Export an architectural level / floor plan to a PDF file on disk
    static bool exportLevelToPdf(const kalara::architecture::Level& level,
                                 const std::string& filePath,
                                 const ExportSettings& settings = ExportSettings{});

    /// Export an entire project to a PDF file on disk
    static bool exportProjectToPdf(const kalara::architecture::Project& project,
                                   const std::string& filePath,
                                   const ExportSettings& settings = ExportSettings{});
};

} // namespace kalara::exporters
