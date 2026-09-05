#include "kalara/editor/validation_widget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableWidgetItem>

namespace kalara::editor {

using namespace kalara::runtime;

ValidationWidget::ValidationWidget(kalara::architecture::Project* project, QWidget* parent)
    : QWidget(parent), m_project(project)
{
    setupUI();
    runValidation();
}

void ValidationWidget::setProject(kalara::architecture::Project* project) {
    m_project = project;
    runValidation();
}

void ValidationWidget::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(6);

    // Top control bar: Summary + Filter + Run Button
    auto* topRow = new QHBoxLayout();
    m_summaryLabel = new QLabel("Ready", this);
    m_summaryLabel->setStyleSheet("font-weight: bold;");
    topRow->addWidget(m_summaryLabel);

    topRow->addStretch();

    topRow->addWidget(new QLabel("Filter:", this));
    m_filterCombo = new QComboBox(this);
    m_filterCombo->addItems({"All Issues", "Errors Only", "Warnings Only", "Geometry", "Connection", "Room", "Clearance", "Site & Setback"});
    topRow->addWidget(m_filterCombo);

    m_validateBtn = new QPushButton("Run Validation", this);
    topRow->addWidget(m_validateBtn);
    mainLayout->addLayout(topRow);

    connect(m_validateBtn, &QPushButton::clicked, this, &ValidationWidget::runValidation);
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ValidationWidget::onFilterChanged);

    // Diagnostic table
    m_table = new QTableWidget(this);
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({"Severity", "Category", "Code", "Message", "Suggested Remedy"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(m_table);

    connect(m_table, &QTableWidget::cellDoubleClicked, this, &ValidationWidget::onRowDoubleClicked);

    // Rule 16 Legal Disclaimer Footer
    auto* disclaimerLabel = new QLabel(QString::fromUtf8(ValidationReport::LegalDisclaimer.data()), this);
    disclaimerLabel->setWordWrap(true);
    disclaimerLabel->setStyleSheet("color: #88909d; font-size: 10px; font-style: italic; border-top: 1px solid #333a44; padding-top: 4px;");
    mainLayout->addWidget(disclaimerLabel);
}

void ValidationWidget::runValidation() {
    if (!m_project) {
        m_lastReport = ValidationReport{};
        populateTable();
        return;
    }

    m_lastReport = ValidationEngine::validateProject(*m_project);
    populateTable();
}

void ValidationWidget::populateTable() {
    m_table->setRowCount(0);

    size_t errCount = m_lastReport.errorCount();
    size_t warnCount = m_lastReport.warningCount();
    size_t infoCount = m_lastReport.infoCount();

    QString summary = QString("Issues: %1 (❌ %2 Errors, ⚠️ %3 Warnings, ℹ️ %4 Info)")
        .arg(m_lastReport.issues.size())
        .arg(errCount)
        .arg(warnCount)
        .arg(infoCount);
    m_summaryLabel->setText(summary);

    int filterIdx = m_filterCombo->currentIndex();

    int row = 0;
    for (size_t i = 0; i < m_lastReport.issues.size(); ++i) {
        const auto& issue = m_lastReport.issues[i];

        // Apply filters
        if (filterIdx == 1 && issue.severity != ValidationSeverity::Error) continue;
        if (filterIdx == 2 && issue.severity != ValidationSeverity::Warning) continue;
        if (filterIdx == 3 && issue.category != ValidationCategory::Geometry) continue;
        if (filterIdx == 4 && issue.category != ValidationCategory::Connection) continue;
        if (filterIdx == 5 && issue.category != ValidationCategory::Room) continue;
        if (filterIdx == 6 && issue.category != ValidationCategory::Clearance) continue;
        if (filterIdx == 7 && issue.category != ValidationCategory::SiteAndSetback) continue;

        m_table->insertRow(row);

        QString sevStr;
        QColor sevColor;
        switch (issue.severity) {
            case ValidationSeverity::Error:
                sevStr = "❌ Error";
                sevColor = QColor(240, 80, 80);
                break;
            case ValidationSeverity::Warning:
                sevStr = "⚠️ Warning";
                sevColor = QColor(240, 190, 70);
                break;
            case ValidationSeverity::Info:
                sevStr = "ℹ️ Info";
                sevColor = QColor(80, 180, 240);
                break;
        }

        auto* sevItem = new QTableWidgetItem(sevStr);
        sevItem->setForeground(sevColor);
        sevItem->setData(Qt::UserRole, static_cast<int>(i)); // index in m_lastReport
        m_table->setItem(row, 0, sevItem);

        auto* catItem = new QTableWidgetItem(QString::fromUtf8(validationCategoryName(issue.category).data()));
        m_table->setItem(row, 1, catItem);

        auto* codeItem = new QTableWidgetItem(QString::fromStdString(issue.code));
        m_table->setItem(row, 2, codeItem);

        auto* msgItem = new QTableWidgetItem(QString::fromStdString(issue.message));
        m_table->setItem(row, 3, msgItem);

        auto* remedyItem = new QTableWidgetItem(QString::fromStdString(issue.suggestedRemedy));
        m_table->setItem(row, 4, remedyItem);

        ++row;
    }
}

void ValidationWidget::onRowDoubleClicked(int row, int /*column*/) {
    auto* item = m_table->item(row, 0);
    if (!item) return;

    int idx = item->data(Qt::UserRole).toInt();
    if (idx >= 0 && idx < static_cast<int>(m_lastReport.issues.size())) {
        const auto& issue = m_lastReport.issues[idx];
        emit issueSelected(issue.entityIds, issue.location);
    }
}

void ValidationWidget::onFilterChanged(int /*index*/) {
    populateTable();
}

} // namespace kalara::editor
