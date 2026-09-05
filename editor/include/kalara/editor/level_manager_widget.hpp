#pragma once
#include "kalara/architecture/project.hpp"
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QSlider>
#include <QPushButton>

namespace kalara::editor {

/// Dockable widget for managing multi-level buildings, floor switching, underlay references, and roof levels.
class LevelManagerWidget : public QWidget {
    Q_OBJECT

public:
    explicit LevelManagerWidget(kalara::architecture::Project* project, QWidget* parent = nullptr);
    ~LevelManagerWidget() override = default;

    void setProject(kalara::architecture::Project* project);
    void refreshLevels();

signals:
    void activeLevelChanged(const kalara::architecture::EntityId& levelId);
    void levelStructureChanged();

private slots:
    void onTableRowSelected(int row, int column);
    void onCellChanged(int row, int column);
    void onUnderlayChanged(int index);
    void onOpacityChanged(int value);
    void onAddLevelClicked();
    void onDuplicateLevelClicked();
    void onAddRoofClicked();

private:
    kalara::architecture::Building* currentBuilding();
    void setupUI();

    kalara::architecture::Project* m_project = nullptr;
    QTableWidget* m_table = nullptr;
    QComboBox* m_underlayCombo = nullptr;
    QSlider* m_opacitySlider = nullptr;
    QPushButton* m_addLevelBtn = nullptr;
    QPushButton* m_duplicateBtn = nullptr;
    QPushButton* m_addRoofBtn = nullptr;
    bool m_updating = false;
};

} // namespace kalara::editor
