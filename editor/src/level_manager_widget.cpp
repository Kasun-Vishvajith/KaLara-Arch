#include "kalara/editor/level_manager_widget.hpp"
#include "kalara/core/logging.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QGroupBox>
#include <QLabel>
#include <QTableWidgetItem>
#include <QCheckBox>
#include <sstream>
#include <iomanip>

namespace kalara::editor {

LevelManagerWidget::LevelManagerWidget(kalara::architecture::Project* project, QWidget* parent)
    : QWidget(parent), m_project(project)
{
    setupUI();
    refreshLevels();
}

void LevelManagerWidget::setProject(kalara::architecture::Project* project) {
    m_project = project;
    refreshLevels();
}

kalara::architecture::Building* LevelManagerWidget::currentBuilding() {
    if (!m_project) return nullptr;
    auto* site = m_project->defaultSite();
    if (!site || site->buildings().empty()) return nullptr;
    return site->buildings().front().get();
}

void LevelManagerWidget::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(8);

    // Levels Table
    m_table = new QTableWidget(this);
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({"Active", "Level Name", "Type", "Elev (mm)", "Height (mm)"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(m_table);

    connect(m_table, &QTableWidget::cellClicked, this, &LevelManagerWidget::onTableRowSelected);

    // Underlay (Ghost Floor) Reference Section (Step 12)
    auto* underlayGroup = new QGroupBox("Underlay Reference (Ghost Floor)", this);
    auto* underlayLayout = new QVBoxLayout(underlayGroup);

    auto* comboLayout = new QHBoxLayout();
    comboLayout->addWidget(new QLabel("Show Floor Underneath:", underlayGroup));
    m_underlayCombo = new QComboBox(underlayGroup);
    m_underlayCombo->addItem("None", QString());
    comboLayout->addWidget(m_underlayCombo);
    underlayLayout->addLayout(comboLayout);

    auto* opacityLayout = new QHBoxLayout();
    opacityLayout->addWidget(new QLabel("Ghost Linework Opacity:", underlayGroup));
    m_opacitySlider = new QSlider(Qt::Horizontal, underlayGroup);
    m_opacitySlider->setRange(10, 90);
    m_opacitySlider->setValue(40);
    opacityLayout->addWidget(m_opacitySlider);
    underlayLayout->addLayout(opacityLayout);

    mainLayout->addWidget(underlayGroup);

    connect(m_underlayCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LevelManagerWidget::onUnderlayChanged);
    connect(m_opacitySlider, &QSlider::valueChanged,
            this, &LevelManagerWidget::onOpacityChanged);

    // Action Buttons
    auto* btnLayout = new QHBoxLayout();
    m_addLevelBtn = new QPushButton("Add Floor", this);
    m_duplicateBtn = new QPushButton("Duplicate Up", this);
    m_addRoofBtn = new QPushButton("Add Roof", this);

    btnLayout->addWidget(m_addLevelBtn);
    btnLayout->addWidget(m_duplicateBtn);
    btnLayout->addWidget(m_addRoofBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_addLevelBtn, &QPushButton::clicked, this, &LevelManagerWidget::onAddLevelClicked);
    connect(m_duplicateBtn, &QPushButton::clicked, this, &LevelManagerWidget::onDuplicateLevelClicked);
    connect(m_addRoofBtn, &QPushButton::clicked, this, &LevelManagerWidget::onAddRoofClicked);
}

void LevelManagerWidget::refreshLevels() {
    auto* bld = currentBuilding();
    if (!bld) {
        m_table->setRowCount(0);
        return;
    }

    m_updating = true;
    m_table->setRowCount(0);
    m_underlayCombo->clear();
    m_underlayCombo->addItem("None", QString());

    const auto& levels = bld->levels();
    auto* activeLvl = bld->activeLevel();

    int activeRow = -1;
    for (size_t i = 0; i < levels.size(); ++i) {
        const auto& lvl = levels[i];
        int row = static_cast<int>(i);
        m_table->insertRow(row);

        bool isActive = (activeLvl && activeLvl->id == lvl->id);
        if (isActive) activeRow = row;

        auto* activeItem = new QTableWidgetItem(isActive ? "★" : "");
        activeItem->setTextAlignment(Qt::AlignCenter);
        activeItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        m_table->setItem(row, 0, activeItem);

        auto* nameItem = new QTableWidgetItem(QString::fromStdString(lvl->name));
        nameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        nameItem->setData(Qt::UserRole, QVariant(QString::fromStdString(lvl->id.string())));
        m_table->setItem(row, 1, nameItem);

        auto* typeItem = new QTableWidgetItem(QString::fromUtf8(levelTypeName(lvl->type).data()));
        typeItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        m_table->setItem(row, 2, typeItem);

        auto* elevItem = new QTableWidgetItem(QString::number(lvl->elevation_mm, 'f', 0));
        elevItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        elevItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        m_table->setItem(row, 3, elevItem);

        auto* heightItem = new QTableWidgetItem(QString::number(lvl->height_mm, 'f', 0));
        heightItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        heightItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        m_table->setItem(row, 4, heightItem);

        // Add to underlay combo box if not the active level
        if (!isActive) {
            m_underlayCombo->addItem(QString::fromStdString(lvl->name), QString::fromStdString(lvl->id.string()));
        }
    }

    if (activeRow >= 0) {
        m_table->selectRow(activeRow);
    }

    // Set underlay combo selection
    if (activeLvl && activeLvl->underlayLevelId.has_value()) {
        int idx = m_underlayCombo->findData(QString::fromStdString(activeLvl->underlayLevelId.value().string()));
        if (idx >= 0) {
            m_underlayCombo->setCurrentIndex(idx);
        } else {
            m_underlayCombo->setCurrentIndex(0);
        }
        m_opacitySlider->setValue(static_cast<int>(std::round(activeLvl->underlayOpacity * 100.0)));
    } else {
        m_underlayCombo->setCurrentIndex(0);
    }

    m_updating = false;
}

void LevelManagerWidget::onTableRowSelected(int row, int /*column*/) {
    if (m_updating) return;
    auto* bld = currentBuilding();
    if (!bld) return;

    auto* nameItem = m_table->item(row, 1);
    if (!nameItem) return;

    kalara::architecture::EntityId id(nameItem->data(Qt::UserRole).toString().toStdString());
    bld->setActiveLevel(id);

    refreshLevels();
    emit activeLevelChanged(id);
    emit levelStructureChanged();
}

void LevelManagerWidget::onCellChanged(int /*row*/, int /*column*/) {
    // Reserved for inline editing
}

void LevelManagerWidget::onUnderlayChanged(int index) {
    if (m_updating) return;
    auto* bld = currentBuilding();
    if (!bld) return;
    auto* activeLvl = bld->activeLevel();
    if (!activeLvl) return;

    QString data = m_underlayCombo->itemData(index).toString();
    if (data.isEmpty()) {
        activeLvl->underlayLevelId.reset();
    } else {
        activeLvl->underlayLevelId = kalara::architecture::EntityId(data.toStdString());
    }
    emit levelStructureChanged();
}

void LevelManagerWidget::onOpacityChanged(int value) {
    if (m_updating) return;
    auto* bld = currentBuilding();
    if (!bld) return;
    auto* activeLvl = bld->activeLevel();
    if (!activeLvl) return;

    activeLvl->underlayOpacity = value / 100.0;
    emit levelStructureChanged();
}

void LevelManagerWidget::onAddLevelClicked() {
    auto* bld = currentBuilding();
    if (!bld) return;

    double topElev = 0.0;
    double defaultHeight = 3000.0;
    int floorNum = static_cast<int>(bld->levels().size());
    if (!bld->levels().empty()) {
        const auto& topLvl = bld->levels().back();
        topElev = topLvl->elevation_mm + topLvl->height_mm;
        defaultHeight = topLvl->height_mm;
    }

    std::string name = "Floor " + std::to_string(floorNum);
    auto& newLvl = bld->addLevel(name, topElev, defaultHeight, kalara::architecture::LevelType::UpperFloor);
    bld->setActiveLevel(newLvl.id);

    refreshLevels();
    emit activeLevelChanged(newLvl.id);
    emit levelStructureChanged();
}

void LevelManagerWidget::onDuplicateLevelClicked() {
    auto* bld = currentBuilding();
    if (!bld) return;
    auto* activeLvl = bld->activeLevel();
    if (!activeLvl) return;

    int floorNum = static_cast<int>(bld->levels().size());
    std::string name = "Floor " + std::to_string(floorNum) + " (Copy)";
    auto* newLvl = bld->duplicateLevelUpwards(activeLvl->id, name);
    if (newLvl) {
        bld->setActiveLevel(newLvl->id);
        refreshLevels();
        emit activeLevelChanged(newLvl->id);
        emit levelStructureChanged();
    }
}

void LevelManagerWidget::onAddRoofClicked() {
    auto* bld = currentBuilding();
    if (!bld) return;

    // Create Roof level
    auto& roofLvl = bld->addRoofLevel("Roof Plan", 2400.0);

    // Compute bounding footprint from all walls on the level below
    std::vector<kalara::core::geometry::Point2D> roofBoundary;
    if (bld->levels().size() >= 2) {
        // Floor below roof is second to last
        const auto& floorBelow = bld->levels()[bld->levels().size() - 2];
        roofLvl.underlayLevelId = floorBelow->id;

        double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
        for (const auto& w : floorBelow->walls()) {
            minX = std::min({minX, w->start.x, w->end.x});
            maxX = std::max({maxX, w->start.x, w->end.x});
            minY = std::min({minY, w->start.y, w->end.y});
            maxY = std::max({maxY, w->start.y, w->end.y});
        }

        if (minX < maxX && minY < maxY) {
            roofBoundary = {
                {minX, minY},
                {maxX, minY},
                {maxX, maxY},
                {minX, maxY}
            };
        }
    }

    if (roofBoundary.empty()) {
        roofBoundary = {{-3000.0, -2500.0}, {3000.0, -2500.0}, {3000.0, 2500.0}, {-3000.0, 2500.0}};
    }

    roofLvl.addRoof("Main Gable Roof", kalara::architecture::RoofType::Gable, roofBoundary, 22.5, 600.0);
    bld->setActiveLevel(roofLvl.id);

    refreshLevels();
    emit activeLevelChanged(roofLvl.id);
    emit levelStructureChanged();
}

} // namespace kalara::editor
