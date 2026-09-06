#pragma once
#include "kalara/editor/viewport_widget.hpp"
#include "kalara/editor/edit_state.hpp"
#include "kalara/architecture/project.hpp"
#include "kalara/runtime/transaction.hpp"
#include <QMainWindow>
#include <QLabel>
#include <QStatusBar>
#include <QAction>
#include <memory>

namespace kalara::editor {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

    [[nodiscard]] ViewportWidget* viewportWidget() const noexcept { return m_viewport; }
    [[nodiscard]] kalara::architecture::Project* project() const noexcept { return m_project.get(); }
    [[nodiscard]] kalara::runtime::TransactionManager& transactionManager() noexcept { return m_transactionManager; }

public slots:
    void newProject();
    void openProject();
    bool saveProject();
    bool saveProjectAs();
    void exportDxf();
    void exportSvg();
    void exportPdf();
    void exportJson();
    void undo();
    void redo();

private slots:
    void onCursorCoordinatesChanged(double x_mm, double y_mm);
    void onZoomChanged(double scale);
    void onSelectionChanged();
    void onPlaceLibraryItem(const std::string& itemId);

    // S20-A: Tool rail slots
    void setActiveTool(ViewportInteractionMode mode);   ///< Called by tool rail buttons
    void onToolChanged(ViewportInteractionMode mode);   ///< Receives toolChanged signal from viewport; syncs rail + shows toast
    void toggleSnap(bool enabled);                      ///< Snap toggle action

private:
    void setupUI();
    void updateUndoRedoActions();
    void updateWindowTitle();

    // --- Widgets ---
    ViewportWidget *m_viewport       = nullptr;
    class LibraryBrowserWidget *m_libraryBrowser = nullptr;
    class LevelManagerWidget   *m_levelManager   = nullptr;
    class SitePlanWidget       *m_sitePlan       = nullptr;
    class ValidationWidget     *m_validation     = nullptr;
    QLabel *m_coordLabel  = nullptr;
    QLabel *m_zoomLabel   = nullptr;
    QLabel *m_statusLabel = nullptr;

    // --- Menu actions ---
    QAction *m_undoAction = nullptr;
    QAction *m_redoAction = nullptr;

    // --- S20-A: Tool rail actions (kept alive for checked-state sync) ---
    QAction *m_toolSelect    = nullptr;
    QAction *m_toolDrawWall  = nullptr;
    QAction *m_toolAddDoor   = nullptr;
    QAction *m_toolAddWindow = nullptr;
    QAction *m_toolMeasure   = nullptr;
    QAction *m_snapToggle    = nullptr;

    // --- S20-A: Shared editor state (owned here; referenced by viewport) ---
    EditState m_editState;

    QString m_currentFilePath;
    std::unique_ptr<kalara::architecture::Project> m_project;
    kalara::runtime::TransactionManager m_transactionManager;
};

} // namespace kalara::editor
