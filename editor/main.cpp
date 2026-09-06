#include "editor/main_window.h"
#include "core/diagnostic.h"
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setOrganizationName("KaLara");
    app.setApplicationName("KaLara Arch");
    app.setStyle("Fusion");
    const auto logDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(logDir);
    QFile log(logDir + "/session.log");
    log.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    kalara::Logger logger([&](kalara::Severity severity, const std::string& code) {
        QTextStream stream(&log);
        stream << static_cast<int>(severity) << ' ' << QString::fromStdString(code) << '\n';
        stream.flush();
    });
    logger.record({"application.started", kalara::Severity::info});
    kalara::editor::MainWindow window;
    window.show();
    const int result = app.exec();
    logger.record({"application.closed", kalara::Severity::info});
    return result;
}
