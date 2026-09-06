#pragma once
#include "kalara/architecture/project.hpp"
#include <string>

namespace kalara::exporters {

/// Structured JSON Interchange Exporter for CAD/BIM interoperability and external AI agents (Step 18 & Principle 7).
/// Exports machine-readable open architectural schema in canonical millimetres (distinct from native .kla project format).
class JsonInterchangeExporter {
public:
    /// Export project model into standardized JSON interchange string
    [[nodiscard]] static std::string exportProjectToJson(const kalara::architecture::Project& project,
                                                         int indent = 2);

    /// Export project model into standardized JSON interchange file on disk
    static bool exportProjectToFile(const kalara::architecture::Project& project,
                                    const std::string& filePath,
                                    int indent = 2);
};

} // namespace kalara::exporters
