#include "kalara/editor/main_window.hpp"
#include "kalara/core/config.hpp"
#include "kalara/core/logging.hpp"
#include <QApplication>

int main(int argc, char *argv[]) {
    bool isHeadless = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string_view(argv[i]) == "--headless-test") {
            isHeadless = true;
            break;
        }
    }

    if (isHeadless || qEnvironmentVariableIsSet("KALARA_HEADLESS_TEST")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
        QApplication app(argc, argv);
        kalara::core::Config config;
        kalara::core::Logger::info("Starting " + config.appName + " v" + config.version.toString() + " (Headless Mode)");
        kalara::editor::MainWindow window;
        kalara::core::Logger::info("Headless test mode: MainWindow instantiated successfully. Exiting normally.");
        return 0;
    }

    QApplication app(argc, argv);
    kalara::core::Config config;
    kalara::core::Logger::info("Starting " + config.appName + " v" + config.version.toString());

    kalara::editor::MainWindow window;
    window.show();

    return app.exec();
}
