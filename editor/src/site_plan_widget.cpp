#include "kalara/editor/site_plan_widget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <iomanip>
#include <sstream>

namespace kalara::editor {

using namespace kalara::architecture;

SitePlanWidget::SitePlanWidget(kalara::architecture::Project* project, QWidget* parent)
    : QWidget(parent), m_project(project)
{
    setupUI();
    refreshSiteData();
}

void SitePlanWidget::setProject(kalara::architecture::Project* project) {
    m_project = project;
    refreshSiteData();
}

kalara::architecture::Site* SitePlanWidget::currentSite() {
    return m_project ? m_project->defaultSite() : nullptr;
}

void SitePlanWidget::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(8);

    // 1. Site Metrics
    auto* metricsGroup = new QGroupBox("Site & Planning Metrics", this);
    auto* formMetrics = new QFormLayout(metricsGroup);

    m_areaLabel = new QLabel("0.0 m²", metricsGroup);
    m_perimeterLabel = new QLabel("0.0 m", metricsGroup);
    m_buildableAreaLabel = new QLabel("0.0 m²", metricsGroup);
    m_footprintLabel = new QLabel("0.0 m²", metricsGroup);
    m_coverageLabel = new QLabel("0.0%", metricsGroup);

    formMetrics->addRow("Property Area:", m_areaLabel);
    formMetrics->addRow("Perimeter:", m_perimeterLabel);
    formMetrics->addRow("Buildable Envelope:", m_buildableAreaLabel);
    formMetrics->addRow("Building Footprint:", m_footprintLabel);
    formMetrics->addRow("Site Coverage:", m_coverageLabel);
    mainLayout->addWidget(metricsGroup);

    // 2. Setback Controls
    auto* setbackGroup = new QGroupBox("Municipal Setbacks (mm)", this);
    auto* formSetbacks = new QFormLayout(setbackGroup);

    m_frontSetbackSpin = new QDoubleSpinBox(setbackGroup);
    m_frontSetbackSpin->setRange(0.0, 50000.0);
    m_frontSetbackSpin->setSingleStep(500.0);
    m_frontSetbackSpin->setSuffix(" mm");

    m_rearSetbackSpin = new QDoubleSpinBox(setbackGroup);
    m_rearSetbackSpin->setRange(0.0, 50000.0);
    m_rearSetbackSpin->setSingleStep(500.0);
    m_rearSetbackSpin->setSuffix(" mm");

    m_leftSetbackSpin = new QDoubleSpinBox(setbackGroup);
    m_leftSetbackSpin->setRange(0.0, 50000.0);
    m_leftSetbackSpin->setSingleStep(500.0);
    m_leftSetbackSpin->setSuffix(" mm");

    m_rightSetbackSpin = new QDoubleSpinBox(setbackGroup);
    m_rightSetbackSpin->setRange(0.0, 50000.0);
    m_rightSetbackSpin->setSingleStep(500.0);
    m_rightSetbackSpin->setSuffix(" mm");

    formSetbacks->addRow("Front (South):", m_frontSetbackSpin);
    formSetbacks->addRow("Rear (North):", m_rearSetbackSpin);
    formSetbacks->addRow("Side (West/Left):", m_leftSetbackSpin);
    formSetbacks->addRow("Side (East/Right):", m_rightSetbackSpin);
    mainLayout->addWidget(setbackGroup);

    connect(m_frontSetbackSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SitePlanWidget::onSetbacksChanged);
    connect(m_rearSetbackSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SitePlanWidget::onSetbacksChanged);
    connect(m_leftSetbackSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SitePlanWidget::onSetbacksChanged);
    connect(m_rightSetbackSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SitePlanWidget::onSetbacksChanged);

    // 3. Site Orientation / True North
    auto* orientGroup = new QGroupBox("True North Direction", this);
    auto* orientLayout = new QVBoxLayout(orientGroup);

    auto* spinRow = new QHBoxLayout();
    spinRow->addWidget(new QLabel("North Angle:", orientGroup));
    m_northSpin = new QDoubleSpinBox(orientGroup);
    m_northSpin->setRange(0.0, 360.0);
    m_northSpin->setSingleStep(5.0);
    m_northSpin->setSuffix("°");
    spinRow->addWidget(m_northSpin);
    orientLayout->addLayout(spinRow);

    m_northSlider = new QSlider(Qt::Horizontal, orientGroup);
    m_northSlider->setRange(0, 360);
    orientLayout->addWidget(m_northSlider);
    mainLayout->addWidget(orientGroup);

    connect(m_northSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SitePlanWidget::onNorthAngleChanged);
    connect(m_northSlider, &QSlider::valueChanged, this, &SitePlanWidget::onNorthSliderChanged);

    // 4. Site Element Quick Actions
    auto* actionsGroup = new QGroupBox("Add Site Elements", this);
    auto* actionsLayout = new QVBoxLayout(actionsGroup);

    auto* roadBtn = new QPushButton("Add Road Context (Frontage)", actionsGroup);
    auto* parkingBtn = new QPushButton("Add Driveway & Parking (2 Stalls)", actionsGroup);
    auto* poolBtn = new QPushButton("Add Swimming Pool & Deck", actionsGroup);
    auto* lawnBtn = new QPushButton("Add Front Lawn", actionsGroup);

    actionsLayout->addWidget(roadBtn);
    actionsLayout->addWidget(parkingBtn);
    actionsLayout->addWidget(poolBtn);
    actionsLayout->addWidget(lawnBtn);
    mainLayout->addWidget(actionsGroup);

    connect(roadBtn, &QPushButton::clicked, this, &SitePlanWidget::onAddRoadClicked);
    connect(parkingBtn, &QPushButton::clicked, this, &SitePlanWidget::onAddDrivewayAndParkingClicked);
    connect(poolBtn, &QPushButton::clicked, this, &SitePlanWidget::onAddPoolClicked);
    connect(lawnBtn, &QPushButton::clicked, this, &SitePlanWidget::onAddLandscapeClicked);

    mainLayout->addStretch();
}

void SitePlanWidget::refreshSiteData() {
    auto* site = currentSite();
    if (!site) return;

    m_updating = true;

    // Refresh Metrics
    m_areaLabel->setText(QString("%1 m²").arg(site->propertyArea_m2(), 0, 'f', 1));
    m_perimeterLabel->setText(QString("%1 m").arg(site->propertyPerimeter_mm() / 1000.0, 0, 'f', 1));
    m_buildableAreaLabel->setText(QString("%1 m²").arg(site->buildableArea_mm2() / 1000000.0, 0, 'f', 1));
    m_footprintLabel->setText(QString("%1 m²").arg(site->buildingFootprintArea_mm2() / 1000000.0, 0, 'f', 1));
    m_coverageLabel->setText(QString("%1%").arg(site->siteCoveragePercentage(), 0, 'f', 1));

    // Refresh Setbacks
    m_frontSetbackSpin->setValue(site->setbacks.front_mm);
    m_rearSetbackSpin->setValue(site->setbacks.rear_mm);
    m_leftSetbackSpin->setValue(site->setbacks.sideLeft_mm);
    m_rightSetbackSpin->setValue(site->setbacks.sideRight_mm);

    // Refresh North
    m_northSpin->setValue(site->northAngle_deg);
    m_northSlider->setValue(static_cast<int>(std::round(site->northAngle_deg)));

    m_updating = false;
}

void SitePlanWidget::onSetbacksChanged() {
    if (m_updating) return;
    auto* site = currentSite();
    if (!site) return;

    site->setbacks.front_mm = m_frontSetbackSpin->value();
    site->setbacks.rear_mm = m_rearSetbackSpin->value();
    site->setbacks.sideLeft_mm = m_leftSetbackSpin->value();
    site->setbacks.sideRight_mm = m_rightSetbackSpin->value();

    refreshSiteData();
    emit siteModified();
}

void SitePlanWidget::onNorthAngleChanged(double deg) {
    if (m_updating) return;
    auto* site = currentSite();
    if (!site) return;

    site->northAngle_deg = deg;
    m_updating = true;
    m_northSlider->setValue(static_cast<int>(std::round(deg)));
    m_updating = false;

    emit siteModified();
}

void SitePlanWidget::onNorthSliderChanged(int deg) {
    if (m_updating) return;
    auto* site = currentSite();
    if (!site) return;

    site->northAngle_deg = static_cast<double>(deg);
    m_updating = true;
    m_northSpin->setValue(static_cast<double>(deg));
    m_updating = false;

    emit siteModified();
}

void SitePlanWidget::onAddRoadClicked() {
    auto* site = currentSite();
    if (!site) return;

    // Add road along the South front boundary (y = -17000 mm)
    kalara::core::geometry::Segment2D roadCenter({-15000.0, -17500.0}, {15000.0, -17500.0});
    site->addRoad("South Maple Avenue", roadCenter, 10000.0);

    refreshSiteData();
    emit siteModified();
}

void SitePlanWidget::onAddDrivewayAndParkingClicked() {
    auto* site = currentSite();
    if (!site) return;

    // Add vehicular driveway entrance along south boundary
    site->addEntrance("Main Driveway", SiteEntranceType::VehicularDriveway, {5000.0, -15000.0}, 4000.0);

    // Add parking zone (2 stalls) on the East front yard
    site->addParkingZone("Guest Parking", {4000.0, -12000.0}, 2, kalara::core::geometry::Angle{}, 2600.0, 5200.0, true);

    // Add paved driveway landscape zone leading to parking
    std::vector<kalara::core::geometry::Point2D> drivewayBoundary = {
        {3500.0, -15000.0},
        {8000.0, -15000.0},
        {8000.0, -6000.0},
        {3500.0, -6000.0}
    };
    site->addLandscapeZone("Paved Driveway", LandscapeType::PavedDriveway, drivewayBoundary);

    refreshSiteData();
    emit siteModified();
}

void SitePlanWidget::onAddPoolClicked() {
    auto* site = currentSite();
    if (!site) return;

    // Add 8m x 4m swimming pool in the rear garden (North side)
    std::vector<kalara::core::geometry::Point2D> poolBoundary = {
        {-4000.0, 6000.0},
        {4000.0, 6000.0},
        {4000.0, 10000.0},
        {-4000.0, 10000.0}
    };
    site->addOutdoorElement("Inground Lap Pool", OutdoorElementType::SwimmingPool, poolBoundary, 1600.0, 600.0);

    refreshSiteData();
    emit siteModified();
}

void SitePlanWidget::onAddLandscapeClicked() {
    auto* site = currentSite();
    if (!site) return;

    // Add front lawn on the West side of the front yard
    std::vector<kalara::core::geometry::Point2D> lawnBoundary = {
        {-9000.0, -14000.0},
        {2000.0, -14000.0},
        {2000.0, -4000.0},
        {-9000.0, -4000.0}
    };
    site->addLandscapeZone("Front Yard Lawn", LandscapeType::Lawn, lawnBoundary);

    refreshSiteData();
    emit siteModified();
}

} // namespace kalara::editor
