#include "kalara/editor/main_window.hpp"
#include "kalara/editor/library_browser_widget.hpp"
#include "kalara/editor/level_manager_widget.hpp"
#include "kalara/editor/site_plan_widget.hpp"
#include "kalara/editor/validation_widget.hpp"
#include "kalara/architecture/project_serializer.hpp"
#include "kalara/exporters/dxf_exporter.hpp"
#include "kalara/exporters/svg_exporter.hpp"
#include "kalara/exporters/json_exporter.hpp"
#include "kalara/exporters/pdf_exporter.hpp"
#include "kalara/core/config.hpp"
#include "kalara/core/logging.hpp"
#include <QStatusBar>
#include <QToolBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QDockWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
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
    updateWindowTitle();

    // Menu Bar & File Actions (Step 15 & Rule 4)
    auto *menu = menuBar();
    auto *fileMenu = menu->addMenu("&File");

    fileMenu->addAction("&New Project", this, &MainWindow::newProject, QKeySequence::New);
    fileMenu->addAction("&Open Project...", this, &MainWindow::openProject, QKeySequence::Open);
    fileMenu->addSeparator();
    fileMenu->addAction("&Save Project", this, &MainWindow::saveProject, QKeySequence::Save);
    fileMenu->addAction("Save Project &As...", this, &MainWindow::saveProjectAs, QKeySequence::SaveAs);
    fileMenu->addSeparator();
    auto *exportMenu = fileMenu->addMenu("&Export");
    exportMenu->addAction("Export &DXF (AutoCAD)...", this, &MainWindow::exportDxf, QKeySequence("Ctrl+E"));
    exportMenu->addAction("Export &SVG (Vector Graphic)...", this, &MainWindow::exportSvg);
    exportMenu->addAction("Export &PDF (Documentation Sheet)...", this, &MainWindow::exportPdf, QKeySequence("Ctrl+P"));
    exportMenu->addAction("Export Structured &JSON (Interchange)...", this, &MainWindow::exportJson);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", this, &QWidget::close, QKeySequence::Quit);

    // Edit Menu & History / Undo-Redo Actions (Step 16)
    auto *editMenu = menu->addMenu("&Edit");
    m_undoAction = editMenu->addAction("&Undo", this, &MainWindow::undo, QKeySequence::Undo);
    m_redoAction = editMenu->addAction("&Redo", this, &MainWindow::redo, QKeySequence::Redo);
    m_redoAction->setShortcuts({QKeySequence::Redo, QKeySequence("Ctrl+Shift+Z")});

    m_viewport = new ViewportWidget(this);
    m_viewport->setProject(m_project.get());
    m_viewport->setEditState(&m_editState);   // S20-A: share EditState
    setCentralWidget(m_viewport);

    // =========================================================================
    // S20-A: Left Tool Rail (Arcada §3.1 / Part 2 #1 — vertical icon/tool strip)
    // =========================================================================
    auto* toolBar = new QToolBar("Architectural Tools", this);
    toolBar->setMovable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    toolBar->setOrientation(Qt::Vertical);
    addToolBar(Qt::LeftToolBarArea, toolBar);

    // Exclusive action group: only one tool active at a time (Arcada: resetTools on setTool)
    auto* toolGroup = new QActionGroup(this);
    toolGroup->setExclusive(true);

    m_toolSelect = toolGroup->addAction("▶  Select");
    m_toolSelect->setCheckable(true);
    m_toolSelect->setChecked(true); // default tool
    m_toolSelect->setToolTip("Select (V)  —  Click to select · Drag to move · Shift/Ctrl for multi-select");
    toolBar->addAction(m_toolSelect);

    m_toolDrawWall = toolGroup->addAction("╔  Draw Wall");
    m_toolDrawWall->setCheckable(true);
    m_toolDrawWall->setToolTip("Draw Wall (W)  —  Click to place nodes · Double-click or Esc to end chain");
    toolBar->addAction(m_toolDrawWall);

    m_toolAddDoor = toolGroup->addAction("▭  Add Door");
    m_toolAddDoor->setCheckable(true);
    m_toolAddDoor->setToolTip("Add Door (D)  —  Click on a wall to insert a door · Right-click to flip orientation");
    toolBar->addAction(m_toolAddDoor);

    m_toolAddWindow = toolGroup->addAction("□  Add Window");
    m_toolAddWindow->setCheckable(true);
    m_toolAddWindow->setToolTip("Add Window (I)  —  Click on a wall to insert a window · Right-click to flip orientation");
    toolBar->addAction(m_toolAddWindow);

    m_toolMeasure = toolGroup->addAction("⟷  Measure");
    m_toolMeasure->setCheckable(true);
    m_toolMeasure->setToolTip("Measure (M)  —  Drag to measure a distance · Esc to cancel (overlay only)");
    toolBar->addAction(m_toolMeasure);

    toolBar->addSeparator();

    m_snapToggle = toolBar->addAction("⊞  Snap ON");
    m_snapToggle->setCheckable(true);
    m_snapToggle->setChecked(true);
    m_snapToggle->setToolTip("Toggle grid + semantic snapping (Rule 12)");

    // Wire tool rail → viewport tool switch + toast hint
    connect(m_toolSelect,    &QAction::triggered, this, [this]{ setActiveTool(ViewportInteractionMode::Select);    });
    connect(m_toolDrawWall,  &QAction::triggered, this, [this]{ setActiveTool(ViewportInteractionMode::DrawWall);  });
    connect(m_toolAddDoor,   &QAction::triggered, this, [this]{ setActiveTool(ViewportInteractionMode::AddDoor);   });
    connect(m_toolAddWindow, &QAction::triggered, this, [this]{ setActiveTool(ViewportInteractionMode::AddWindow); });
    connect(m_toolMeasure,   &QAction::triggered, this, [this]{ setActiveTool(ViewportInteractionMode::Measure);   });
    connect(m_snapToggle,    &QAction::toggled,   this, &MainWindow::toggleSnap);

    // Wire viewport → tool rail: keyboard shortcuts emit toolChanged; sync checked state + toast
    connect(m_viewport, &ViewportWidget::toolChanged, this, &MainWindow::onToolChanged);
    // =========================================================================

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

    updateUndoRedoActions();
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

    m_transactionManager.beginTransaction(*m_project, "Place " + item->name, kalara::runtime::TransactionActor::Human);
    auto& inst = lvl->addLibraryInstance(*item, placePos);
    m_transactionManager.recordOperation("Place " + item->id);
    m_transactionManager.commitTransaction(*m_project);
    updateUndoRedoActions();

    // Select placed instance immediately
    m_viewport->selectionManager().clear();
    m_viewport->selectionManager().select(inst.id);
    m_viewport->update();
    onSelectionChanged();
}

void MainWindow::updateWindowTitle() {
    kalara::core::Config config;
    QString baseTitle = QString::fromStdString(config.appName + " v" + config.version.toString());
    if (m_currentFilePath.isEmpty()) {
        setWindowTitle(baseTitle + " - [Untitled.kla]");
    } else {
        QFileInfo fi(m_currentFilePath);
        setWindowTitle(baseTitle + " - [" + fi.fileName() + "]");
    }
}

void MainWindow::updateUndoRedoActions() {
    if (!m_undoAction || !m_redoAction) return;

    m_undoAction->setEnabled(m_transactionManager.canUndo());
    m_redoAction->setEnabled(m_transactionManager.canRedo());

    if (m_transactionManager.canUndo()) {
        m_undoAction->setText(QString("&Undo %1").arg(QString::fromStdString(m_transactionManager.nextUndoIntent())));
    } else {
        m_undoAction->setText("&Undo");
    }

    if (m_transactionManager.canRedo()) {
        m_redoAction->setText(QString("&Redo %1").arg(QString::fromStdString(m_transactionManager.nextRedoIntent())));
    } else {
        m_redoAction->setText("&Redo");
    }
}

void MainWindow::undo() {
    if (!m_project || !m_transactionManager.canUndo()) return;

    std::string intent = m_transactionManager.nextUndoIntent();
    if (m_transactionManager.undo(*m_project)) {
        m_viewport->selectionManager().clear();
        m_viewport->update();
        if (m_levelManager) m_levelManager->refreshLevels();
        if (m_sitePlan) m_sitePlan->refreshSiteData();
        if (m_validation) m_validation->runValidation();
        updateUndoRedoActions();
        onSelectionChanged();
        statusBar()->showMessage(QString("Undid: %1").arg(QString::fromStdString(intent)), 3000);
    }
}

void MainWindow::redo() {
    if (!m_project || !m_transactionManager.canRedo()) return;

    std::string intent = m_transactionManager.nextRedoIntent();
    if (m_transactionManager.redo(*m_project)) {
        m_viewport->selectionManager().clear();
        m_viewport->update();
        if (m_levelManager) m_levelManager->refreshLevels();
        if (m_sitePlan) m_sitePlan->refreshSiteData();
        if (m_validation) m_validation->runValidation();
        updateUndoRedoActions();
        onSelectionChanged();
        statusBar()->showMessage(QString("Redid: %1").arg(QString::fromStdString(intent)), 3000);
    }
}

void MainWindow::newProject() {
    m_project = std::make_unique<kalara::architecture::Project>("Untitled Project");
    m_currentFilePath.clear();
    updateWindowTitle();

    m_transactionManager.clear();
    updateUndoRedoActions();

    m_viewport->setProject(m_project.get());
    m_viewport->selectionManager().clear();
    m_levelManager->setProject(m_project.get());
    m_sitePlan->setProject(m_project.get());
    m_validation->setProject(m_project.get());
    m_viewport->update();
    onSelectionChanged();
    statusBar()->showMessage("Created new project", 3000);
}

void MainWindow::openProject() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open Architectural Project",
        m_currentFilePath.isEmpty() ? QString() : QFileInfo(m_currentFilePath).absolutePath(),
        "KaLara Arch Projects (*.kla);;All Files (*.*)"
    );

    if (fileName.isEmpty()) return;

    std::string errorMsg;
    auto loaded = kalara::architecture::ProjectSerializer::loadFromFile(fileName.toStdString(), &errorMsg);
    if (!loaded) {
        QMessageBox::critical(this, "Error Opening Project",
                              QString("Failed to open '%1':\n%2")
                              .arg(QFileInfo(fileName).fileName())
                              .arg(QString::fromStdString(errorMsg)));
        return;
    }

    m_project = std::move(loaded);
    m_currentFilePath = fileName;
    updateWindowTitle();

    m_transactionManager.clear();
    updateUndoRedoActions();

    m_viewport->setProject(m_project.get());
    m_viewport->selectionManager().clear();
    m_levelManager->setProject(m_project.get());
    m_sitePlan->setProject(m_project.get());
    m_validation->setProject(m_project.get());
    m_viewport->update();
    onSelectionChanged();
    statusBar()->showMessage("Loaded project from " + QFileInfo(fileName).fileName(), 3000);
}

bool MainWindow::saveProject() {
    if (m_currentFilePath.isEmpty()) {
        return saveProjectAs();
    }

    bool success = kalara::architecture::ProjectSerializer::saveToFile(*m_project, m_currentFilePath.toStdString(), 2);
    if (!success) {
        QMessageBox::critical(this, "Save Error", "Failed to save project to " + m_currentFilePath);
        return false;
    }

    updateWindowTitle();
    statusBar()->showMessage("Saved project to " + QFileInfo(m_currentFilePath).fileName(), 3000);
    return true;
}

bool MainWindow::saveProjectAs() {
    QString defaultName = m_currentFilePath.isEmpty() ? "house.kla" : m_currentFilePath;
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save Architectural Project",
        defaultName,
        "KaLara Arch Projects (*.kla);;All Files (*.*)"
    );

    if (fileName.isEmpty()) return false;
    if (!fileName.endsWith(".kla", Qt::CaseInsensitive)) {
        fileName += ".kla";
    }

    m_currentFilePath = fileName;
    return saveProject();
}

void MainWindow::exportDxf() {
    if (!m_project) return;

    QString defaultName = "drawing.dxf";
    if (!m_currentFilePath.isEmpty()) {
        QFileInfo fi(m_currentFilePath);
        defaultName = fi.completeBaseName() + ".dxf";
    }

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export CAD Drawing (DXF)",
        defaultName,
        "AutoCAD DXF Files (*.dxf);;All Files (*.*)"
    );

    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".dxf", Qt::CaseInsensitive)) {
        fileName += ".dxf";
    }

    kalara::exporters::DxfExportOptions options;
    options.exportWallOutlines = true;
    options.exportWallCenterlines = true;
    options.exportDoorsAndWindows = true;
    options.exportRooms = true;
    options.exportDimensions = true;
    options.exportAnnotations = true;
    options.exportFurniture = true;
    options.exportRoofs = true;
    options.exportSite = true;

    bool success = kalara::exporters::DxfExporter::exportProjectToFile(*m_project, fileName.toStdString(), options);
    if (!success) {
        QMessageBox::critical(this, "Export Error", "Failed to export DXF file to " + fileName);
        return;
    }

    statusBar()->showMessage("Successfully exported DXF to " + QFileInfo(fileName).fileName(), 3000);
}

void MainWindow::exportSvg() {
    if (!m_project) return;

    QString defaultName = "drawing.svg";
    if (!m_currentFilePath.isEmpty()) {
        QFileInfo fi(m_currentFilePath);
        defaultName = fi.completeBaseName() + ".svg";
    }

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export Scalable Vector Graphic (SVG)",
        defaultName,
        "SVG Files (*.svg);;All Files (*.*)"
    );

    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".svg", Qt::CaseInsensitive)) {
        fileName += ".svg";
    }

    kalara::exporters::ExportSettings settings;
    settings.scale = kalara::architecture::DrawingScale::Scale_1_100;
    settings.theme = kalara::exporters::ExportTheme::ColorPresentation;

    bool success = kalara::exporters::SvgExporter::exportProjectToFile(*m_project, fileName.toStdString(), settings);
    if (!success) {
        QMessageBox::critical(this, "Export Error", "Failed to export SVG file to " + fileName);
        return;
    }

    statusBar()->showMessage("Successfully exported SVG to " + QFileInfo(fileName).fileName(), 3000);
}

void MainWindow::exportPdf() {
    if (!m_project) return;

    QString defaultName = "drawing_sheet.pdf";
    if (!m_currentFilePath.isEmpty()) {
        QFileInfo fi(m_currentFilePath);
        defaultName = fi.completeBaseName() + "_sheet.pdf";
    }

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export PDF Documentation Sheet",
        defaultName,
        "PDF Files (*.pdf);;All Files (*.*)"
    );

    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".pdf", Qt::CaseInsensitive)) {
        fileName += ".pdf";
    }

    kalara::exporters::ExportSettings settings;
    settings.paperSize = kalara::exporters::PaperSize::A4;
    settings.orientation = kalara::exporters::SheetOrientation::Landscape;
    settings.scale = kalara::architecture::DrawingScale::Scale_1_100;

    bool success = kalara::exporters::PdfExporter::exportProjectToPdf(*m_project, fileName.toStdString(), settings);
    if (!success) {
        QMessageBox::critical(this, "Export Error", "Failed to export PDF documentation sheet to " + fileName);
        return;
    }

    statusBar()->showMessage("Successfully exported PDF sheet to " + QFileInfo(fileName).fileName(), 3000);
}

void MainWindow::exportJson() {
    if (!m_project) return;

    QString defaultName = "project.json";
    if (!m_currentFilePath.isEmpty()) {
        QFileInfo fi(m_currentFilePath);
        defaultName = fi.completeBaseName() + ".json";
    }

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export Structured Interchange JSON",
        defaultName,
        "JSON Files (*.json);;All Files (*.*)"
    );

    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".json", Qt::CaseInsensitive)) {
        fileName += ".json";
    }

    bool success = kalara::exporters::JsonInterchangeExporter::exportProjectToFile(*m_project, fileName.toStdString(), 2);
    if (!success) {
        QMessageBox::critical(this, "Export Error", "Failed to export JSON interchange file to " + fileName);
        return;
    }

    statusBar()->showMessage("Successfully exported JSON to " + QFileInfo(fileName).fileName(), 3000);
}

// =============================================================================
// S20-A: Tool rail slots — setActiveTool, onToolChanged, toggleSnap
// =============================================================================

void MainWindow::setActiveTool(ViewportInteractionMode mode) {
    // Update shared state and propagate to viewport (viewport will emit toolChanged back)
    m_editState.activeTool = mode;
    m_viewport->setActiveTool(mode);
    // Toast is shown in onToolChanged (triggered by viewport's toolChanged signal)
}

void MainWindow::onToolChanged(ViewportInteractionMode mode) {
    // --- Sync tool rail checked state (block re-entrant triggered signals) ---
    if (m_toolSelect    && m_toolDrawWall && m_toolAddDoor &&
        m_toolAddWindow && m_toolMeasure) {

        const QSignalBlocker b1(m_toolSelect);
        const QSignalBlocker b2(m_toolDrawWall);
        const QSignalBlocker b3(m_toolAddDoor);
        const QSignalBlocker b4(m_toolAddWindow);
        const QSignalBlocker b5(m_toolMeasure);

        // Transient sub-states (RubberbandSelect / DragMove) visually map to Select
        bool selectActive =
            (mode == ViewportInteractionMode::Select       ||
             mode == ViewportInteractionMode::RubberbandSelect ||
             mode == ViewportInteractionMode::DragMove);

        m_toolSelect->setChecked(selectActive);
        m_toolDrawWall->setChecked(mode  == ViewportInteractionMode::DrawWall);
        m_toolAddDoor->setChecked(mode   == ViewportInteractionMode::AddDoor);
        m_toolAddWindow->setChecked(mode == ViewportInteractionMode::AddWindow);
        m_toolMeasure->setChecked(mode   == ViewportInteractionMode::Measure);
    }

    // --- Toast hint (Arcada Part 2 #12: every mode switch fires a non-blocking hint) ---
    // Uses QStatusBar::showMessage which temporarily overlays m_statusLabel for 2500 ms
    // then reverts — zero extra widget overhead (Q2 answer: best-suited approach).
    switch (mode) {
        case ViewportInteractionMode::Select:
            statusBar()->showMessage(
                "Select  —  Click to select · Drag to move · Shift/Ctrl adds to selection", 2500);
            break;
        case ViewportInteractionMode::DrawWall:
            statusBar()->showMessage(
                "Draw Wall  —  Click to place nodes · Double-click or Esc to end chain", 2500);
            break;
        case ViewportInteractionMode::AddDoor:
            statusBar()->showMessage(
                "Add Door  —  Click on a wall to place a door · Right-click to flip orientation", 2500);
            break;
        case ViewportInteractionMode::AddWindow:
            statusBar()->showMessage(
                "Add Window  —  Click on a wall to place a window · Right-click to flip orientation", 2500);
            break;
        case ViewportInteractionMode::Measure:
            statusBar()->showMessage(
                "Measure  —  Drag to measure a distance · Esc to cancel", 2500);
            break;
        default:
            break; // Transient sub-states: no hint (avoids spam during drag operations)
    }
}

void MainWindow::toggleSnap(bool enabled) {
    m_editState.snapEnabled = enabled;
    m_viewport->gridSettings().snapEnabled = enabled;

    // Update toggle label text
    if (m_snapToggle) {
        m_snapToggle->setText(enabled ? "⊞  Snap ON" : "⊠  Snap OFF");
    }
    statusBar()->showMessage(enabled ? "Snap enabled" : "Snap disabled", 2000);
}

} // namespace kalara::editor
