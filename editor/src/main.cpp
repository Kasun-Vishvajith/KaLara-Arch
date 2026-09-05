#include "kalara/editor/main_window.hpp"
#include "kalara/core/config.hpp"
#include "kalara/core/logging.hpp"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    kalara::core::Config config;
    kalara::core::Logger::info("Starting " + config.appName + " v" + config.version.toString());

    kalara::editor::MainWindow window;
    window.show();

    // Support offscreen / automated test execution if requested
    if (qEnvironmentVariableIsSet("KALARA_HEADLESS_TEST")) {
        kalara::core::Logger::info("Headless test mode detected. Exiting normally.");
        return 0;
    }

    return app.exec();
}
