#pragma once
#include "kalara/architecture/project.hpp"
#include <QWidget>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>

namespace kalara::editor {

/// Dockable panel for architectural site planning, setbacks, north orientation, and outdoor elements (Step 13).
class SitePlanWidget : public QWidget {
    Q_OBJECT

public:
    explicit SitePlanWidget(kalara::architecture::Project* project, QWidget* parent = nullptr);
    ~SitePlanWidget() override = default;

    void setProject(kalara::architecture::Project* project);
    void refreshSiteData();

signals:
    void siteModified();

private slots:
    void onSetbacksChanged();
    void onNorthAngleChanged(double deg);
    void onNorthSliderChanged(int deg);
    void onAddRoadClicked();
    void onAddDrivewayAndParkingClicked();
    void onAddPoolClicked();
    void onAddLandscapeClicked();

private:
    kalara::architecture::Site* currentSite();
    void setupUI();

    kalara::architecture::Project* m_project = nullptr;

    QLabel* m_areaLabel = nullptr;
    QLabel* m_perimeterLabel = nullptr;
    QLabel* m_buildableAreaLabel = nullptr;
    QLabel* m_footprintLabel = nullptr;
    QLabel* m_coverageLabel = nullptr;

    QDoubleSpinBox* m_frontSetbackSpin = nullptr;
    QDoubleSpinBox* m_rearSetbackSpin = nullptr;
    QDoubleSpinBox* m_leftSetbackSpin = nullptr;
    QDoubleSpinBox* m_rightSetbackSpin = nullptr;

    QDoubleSpinBox* m_northSpin = nullptr;
    QSlider* m_northSlider = nullptr;

    bool m_updating = false;
};

} // namespace kalara::editor
