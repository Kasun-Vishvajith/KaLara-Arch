#include "kalara/editor/main_window.hpp"
#include "kalara/core/config.hpp"
#include "kalara/core/logging.hpp"
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

namespace kalara::editor {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    kalara::core::Config config;
    setWindowTitle(QString::fromStdString(config.appName + " v" + config.version.toString()));
    resize(1280, 800);

    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);

    auto *label = new QLabel(
        QString::fromStdString(config.appName + " — Structured Architectural Planning System (Foundation)"),
        centralWidget
    );
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    setCentralWidget(centralWidget);
    kalara::core::Logger::info("MainWindow initialized.");
}

} // namespace kalara::editor
