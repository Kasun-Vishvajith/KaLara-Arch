#pragma once
#include "kalara/architecture/project.hpp"
#include "kalara/architecture/level.hpp"
#include "kalara/architecture/site.hpp"
#include <string>
#include <optional>

namespace kalara::exporters {

/// Configuration options for DXF export.
struct DxfExportOptions {
    bool exportWallOutlines = true;
    bool exportWallCenterlines = false;
    bool exportDoorsAndWindows = true;
    bool exportRooms = true;
    bool exportDimensions = true;
    bool exportAnnotations = true;
    bool exportFurniture = true;
    bool exportRoofs = true;
    bool exportSite = true;
    std::optional<kalara::architecture::EntityId> targetLevelId = std::nullopt;
};

/// High-level Architectural Model to 2D DXF Export Adapter (Step 17 & Section 9).
/// Translates architectural semantics, real mm geometry, standard CAD layers, and documentation into clean DXF.
class DxfExporter {
public:
    /// Export an entire project (or active level) to a DXF formatted string
    [[nodiscard]] static std::string exportProjectToDxf(const kalara::architecture::Project& project,
                                                        const DxfExportOptions& options = DxfExportOptions{});

    /// Export project directly to a file on disk
    static bool exportProjectToFile(const kalara::architecture::Project& project,
                                    const std::string& filePath,
                                    const DxfExportOptions& options = DxfExportOptions{});

    /// Export a single architectural level / floor plan to a DXF formatted string
    [[nodiscard]] static std::string exportLevelToDxf(const kalara::architecture::Level& level,
                                                      const DxfExportOptions& options = DxfExportOptions{});

    /// Export an architectural site plan to a DXF formatted string
    [[nodiscard]] static std::string exportSiteToDxf(const kalara::architecture::Site& site,
                                                     const DxfExportOptions& options = DxfExportOptions{});
};

} // namespace kalara::exporters
