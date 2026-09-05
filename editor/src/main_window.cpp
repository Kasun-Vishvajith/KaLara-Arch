#include "kalara/editor/main_window.hpp"
#include "kalara/editor/library_browser_widget.hpp"
#include "kalara/editor/level_manager_widget.hpp"
#include "kalara/editor/site_plan_widget.hpp"
#include "kalara/editor/validation_widget.hpp"
#include "kalara/core/config.hpp"
#include "kalara/core/logging.hpp"
#include <QStatusBar>
#include <QToolBar>
#include <QAction>
#include <QDockWidget>
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

        // Add demo door on South wall (wall 0) and demo window on North wall (wall 2)
        const auto& walls = ground.walls();
        if (walls.size() >= 4) {
            ground.addDoor(walls[0]->id, 2500.0, 900.0, 2100.0, kalara::architecture::DoorSwing::LeftInswing);
            ground.addWindow(walls[2]->id, 2500.0, 1500.0, 1200.0, 900.0, kalara::architecture::WindowType::Casement);

            // Add associative dimension along South wall (Rule 7)
            ground.addDimensionForWall(*walls[0], -600.0);

            // Add note annotation
            ground.addNote({-2300.0, 1600.0}, "Living Room Area: 18.2 m² [Finish: Parquet]");
        }

        // Add demo library instances (Step 11)
        kalara::architecture::LibraryCatalog tempCatalog;
        const auto* sofaItem = tempCatalog.findItem("SOFA_3SEAT");
        if (sofaItem) {
            ground.addLibraryInstance(*sofaItem, {0.0, -1200.0});
        }
        const auto* tableItem = tempCatalog.findItem("TABLE_DINING_6");
        if (tableItem) {
            ground.addLibraryInstance(*tableItem, {0.0, 800.0});
        }
    }

    setupUI();
    kalara::core::Logger::info("MainWindow with 2D Viewport, Walls, Rooms, Openings & Dimensions initialized.");
}

void MainWindow::setupUI() {
    m_viewport = new ViewportWidget(this);
    m_viewport->setProject(m_project.get());
    setCentralWidget(m_viewport);

    // Dockable 2D Architectural Library Browser Panel
    auto *dock = new QDockWidget("Architectural Library", this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_libraryBrowser = new LibraryBrowserWidget(dock);
    dock->setWidget(m_libraryBrowser);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    connect(m_libraryBrowser, &LibraryBrowserWidget::placeItemRequested,
            this, &MainWindow::onPlaceLibraryItem);

    // Dockable Levels & Stories Manager Panel (Step 12)
    auto *levelDock = new QDockWidget("Levels & Stories", this);
    levelDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_levelManager = new LevelManagerWidget(m_project.get(), levelDock);
    levelDock->setWidget(m_levelManager);
    addDockWidget(Qt::RightDockWidgetArea, levelDock);

    connect(m_levelManager, &LevelManagerWidget::activeLevelChanged, this, [this](const auto&) {
        m_viewport->selectionManager().clear();
        m_viewport->update();
        onSelectionChanged();
    });
    connect(m_levelManager, &LevelManagerWidget::levelStructureChanged, this, [this]() {
        m_viewport->update();
    });

    // Dockable Site Planning & Setbacks Panel (Step 13)
    auto *siteDock = new QDockWidget("Site Planning & Setbacks", this);
    siteDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_sitePlan = new SitePlanWidget(m_project.get(), siteDock);
    siteDock->setWidget(m_sitePlan);
    addDockWidget(Qt::RightDockWidgetArea, siteDock);
    tabifyDockWidget(levelDock, siteDock);

    connect(m_sitePlan, &SitePlanWidget::siteModified, this, [this]() {
        m_viewport->update();
    });

    // Dockable Diagnostics & Validation Panel (Step 14 & Rule 16)
    auto *validationDock = new QDockWidget("Diagnostics & Validation", this);
    validationDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    m_validation = new ValidationWidget(m_project.get(), validationDock);
    validationDock->setWidget(m_validation);
    addDockWidget(Qt::BottomDockWidgetArea, validationDock);

    connect(m_validation, &ValidationWidget::issueSelected, this, [this](const auto& entityIds, const auto&) {
        m_viewport->selectionManager().clear();
        for (const auto& id : entityIds) {
            m_viewport->selectionManager().select(id);
        }
        m_viewport->update();
        onSelectionChanged();
    });

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
    connect(m_viewport, &ViewportWidget::selectionChanged,
            this, &MainWindow::onSelectionChanged);
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

void MainWindow::onSelectionChanged() {
    auto& sel = m_viewport->selectionManager();
    if (sel.empty()) {
        m_statusLabel->setText("Ready [Pan: MMB / Alt+LMB, Zoom: Wheel, Snap: Grid/Ortho, Select: LMB / Drag Box]");
    } else {
        auto* site = m_project ? m_project->defaultSite() : nullptr;
        auto* bld = (site && !site->buildings().empty()) ? site->buildings().front().get() : nullptr;
        auto* lvl = bld ? bld->activeLevel() : nullptr;

        if (lvl) {
            auto summary = sel.summarize(*lvl);
            QString info = QString("Selected: %1 entities (Walls: %2, Rooms: %3, Openings: %4, Library: %5, Roofs: %6)")
                .arg(summary.totalCount)
                .arg(summary.wallCount)
                .arg(summary.roomCount)
                .arg(summary.doorCount + summary.windowCount)
                .arg(summary.libraryInstanceCount)
                .arg(summary.roofCount);
            m_statusLabel->setText(info + " [Drag: Move, Arrow keys: Nudge, R: Rotate 90°]");
        } else {
            m_statusLabel->setText(QString("Selected: %1 entities").arg(sel.count()));
        }
    }
}

void MainWindow::onPlaceLibraryItem(const std::string& itemId) {
    if (!m_project || !m_libraryBrowser) return;
    const auto* item = m_libraryBrowser->catalog().findItem(itemId);
    if (!item) return;

    auto* site = m_project->defaultSite();
    if (!site || site->buildings().empty()) return;
    auto* bld = site->buildings().front().get();
    if (!bld) return;
    auto* lvl = bld->activeLevel();
    if (!lvl) return;

    // Place at current cursor position in world coordinates (or viewport center)
    auto placePos = m_viewport->cursorWorld_mm();
    auto& inst = lvl->addLibraryInstance(*item, placePos);

    // Select placed instance immediately
    m_viewport->selectionManager().clear();
    m_viewport->selectionManager().select(inst.id);
    m_viewport->update();
    onSelectionChanged();
}

} // namespace kalara::editor
