#include "kalara/core/config.hpp"
#include "kalara/core/logging.hpp"
#include "kalara/architecture/model_info.hpp"
#include "kalara/runtime/runtime_context.hpp"
#include "kalara/exporters/export_format.hpp"
#include <iostream>
#include <cassert>

int main() {
    kalara::core::Config config;
    assert(!config.appName.empty());
    assert(config.defaultUnits == "mm");

    kalara::core::Logger::info("Running foundation unit tests...");

    kalara::architecture::ModelInfo model;
    assert(model.schemaVersion == "1.0.0");

    kalara::runtime::RuntimeContext runtime;
    assert(!runtime.isInitialized);

    kalara::exporters::ExportFormatInfo exporterInfo;
    exporterInfo.name = "DXF";
    assert(exporterInfo.name == "DXF");

    std::cout << "All foundation layer boundary tests passed successfully." << std::endl;
    return 0;
}
