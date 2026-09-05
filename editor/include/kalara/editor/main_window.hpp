#pragma once
#include "kalara/editor/viewport_widget.hpp"
#include "kalara/architecture/project.hpp"
#include <QMainWindow>
#include <QLabel>
#include <QStatusBar>
#include <memory>

namespace kalara::editor {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

    [[nodiscard]] ViewportWidget* viewportWidget() const noexcept { return m_viewport; }
    [[nodiscard]] kalara::architecture::Project* project() const noexcept { return m_project.get(); }

private slots:
    void onCursorCoordinatesChanged(double x_mm, double y_mm);
    void onZoomChanged(double scale);
    void onSelectionChanged();
    void onPlaceLibraryItem(const std::string& itemId);

private:
    void setupUI();

    ViewportWidget *m_viewport = nullptr;
    class LibraryBrowserWidget *m_libraryBrowser = nullptr;
    class LevelManagerWidget *m_levelManager = nullptr;
    class SitePlanWidget *m_sitePlan = nullptr;
    class ValidationWidget *m_validation = nullptr;
    QLabel *m_coordLabel = nullptr;
    QLabel *m_zoomLabel = nullptr;
    QLabel *m_statusLabel = nullptr;

    std::unique_ptr<kalara::architecture::Project> m_project;
};

} // namespace kalara::editor
