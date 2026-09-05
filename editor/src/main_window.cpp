#include "kalara/editor/main_window.hpp"
#include "kalara/core/config.hpp"
#include "kalara/core/logging.hpp"
#include <QStatusBar>
#include <QToolBar>
#include <QAction>
#include <iomanip>
#include <sstream>

namespace kalara::editor {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    kalara::core::Config config;
    setWindowTitle(QString::fromStdString(config.appName + " v" + config.version.toString()));
    resize(1280, 800);

    // Initialize structured model instance
    m_project = std::make_unique<kalara::architecture::Project>("Demo Architectural Project");

    // Add sample property boundary polygon for 2D canvas visualization
    auto* site = m_project->defaultSite();
    if (site) {
        site->propertyBoundary = {
            kalara::core::geometry::Point2D(-10000.0, -15000.0),
            kalara::core::geometry::Point2D(10000.0, -15000.0),
            kalara::core::geometry::Point2D(10000.0, 15000.0),
            kalara::core::geometry::Point2D(-10000.0, 15000.0)
        };

        // Add demo residential building and connected ground floor walls (5m x 4m room)
        auto& building = site->addBuilding("Villa");
        auto& ground = building.addLevel("Ground Floor", 0.0, 3000.0);
        ground.addWall({-2500.0, -2000.0}, {2500.0, -2000.0}, 200.0); // South wall (5000 mm)
        ground.addWall({2500.0, -2000.0}, {2500.0, 2000.0}, 200.0);   // East wall (4000 mm)
        ground.addWall({2500.0, 2000.0}, {-2500.0, 2000.0}, 200.0);   // North wall (5000 mm)
        ground.addWall({-2500.0, 2000.0}, {-2500.0, -2000.0}, 200.0); // West wall (4000 mm)

        // Add explicit Living Room space matching the wall interior (20 m^2)
        ground.addRoom("Living Room", kalara::architecture::RoomType::LivingRoom, {
            {-2400.0, -1900.0},
            {2400.0, -1900.0},
            {2400.0, 1900.0},
            {-2400.0, 1900.0}
        });
    }

    setupUI();
    kalara::core::Logger::info("MainWindow with 2D Viewport, Walls & Rooms initialized.");
}

void MainWindow::setupUI() {
    m_viewport = new ViewportWidget(this);
    m_viewport->setProject(m_project.get());
    setCentralWidget(m_viewport);

    // Status bar with live coordinate tracking (mm) and zoom level
    auto *status = statusBar();

    m_coordLabel = new QLabel("X: 0 mm | Y: 0 mm", this);
    m_coordLabel->setMinimumWidth(220);
    status->addPermanentWidget(m_coordLabel);

    m_zoomLabel = new QLabel("Zoom: 100%", this);
    m_zoomLabel->setMinimumWidth(120);
    status->addPermanentWidget(m_zoomLabel);

    m_statusLabel = new QLabel("Ready [Pan: MMB / Alt+LMB, Zoom: Wheel, Snap: Grid/Ortho]", this);
    status->addWidget(m_statusLabel);

    connect(m_viewport, &ViewportWidget::cursorCoordinatesChanged,
            this, &MainWindow::onCursorCoordinatesChanged);
    connect(m_viewport, &ViewportWidget::zoomChanged,
            this, &MainWindow::onZoomChanged);
}

void MainWindow::onCursorCoordinatesChanged(double x_mm, double y_mm) {
    std::ostringstream ss;
    ss << "X: " << std::fixed << std::setprecision(0) << x_mm << " mm | Y: "
       << std::fixed << std::setprecision(0) << y_mm << " mm";
    m_coordLabel->setText(QString::fromStdString(ss.str()));
}

void MainWindow::onZoomChanged(double scale) {
    // 0.1 scale is 100% nominal display (10 pixels = 100mm)
    int zoomPercent = static_cast<int>(std::round((scale / 0.1) * 100.0));
    m_zoomLabel->setText(QString("Zoom: %1%").arg(zoomPercent));
}

} // namespace kalara::editor
