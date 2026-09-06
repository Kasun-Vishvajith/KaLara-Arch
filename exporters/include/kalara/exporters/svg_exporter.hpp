#pragma once
#include "kalara/architecture/project.hpp"
#include "kalara/architecture/level.hpp"
#include "kalara/architecture/site.hpp"
#include "kalara/exporters/export_settings.hpp"
#include <string>

namespace kalara::exporters {

/// Scalable Vector Graphics (SVG) Floor Plan and Site Exporter (Step 18 & Principle 7).
/// Generates standards-compliant SVG with architectural styling, scale-aware linework, and documentation tags.
class SvgExporter {
public:
    /// Export an architectural level / floor plan to an SVG string
    [[nodiscard]] static std::string exportLevelToSvg(const kalara::architecture::Level& level,
                                                      const ExportSettings& settings = ExportSettings{});

    /// Export an entire project to an SVG string
    [[nodiscard]] static std::string exportProjectToSvg(const kalara::architecture::Project& project,
                                                        const ExportSettings& settings = ExportSettings{});

    /// Export a site plan to an SVG string
    [[nodiscard]] static std::string exportSiteToSvg(const kalara::architecture::Site& site,
                                                     const ExportSettings& settings = ExportSettings{});

    /// Export level to a file on disk
    static bool exportLevelToFile(const kalara::architecture::Level& level,
                                  const std::string& filePath,
                                  const ExportSettings& settings = ExportSettings{});

    /// Export project to a file on disk
    static bool exportProjectToFile(const kalara::architecture::Project& project,
                                    const std::string& filePath,
                                    const ExportSettings& settings = ExportSettings{});
};

} // namespace kalara::exporters
