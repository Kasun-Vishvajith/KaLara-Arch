#pragma once
#include "kalara/architecture/project.hpp"
#include "kalara/runtime/validation_engine.hpp"
#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>

namespace kalara::editor {

/// Dockable Diagnostics and Building Validation Panel (Rule 16 & Step 14).
/// Displays real-time geometric, connection, room, clearance, and setback diagnostic issues.
class ValidationWidget : public QWidget {
    Q_OBJECT

public:
    explicit ValidationWidget(kalara::architecture::Project* project, QWidget* parent = nullptr);
    ~ValidationWidget() override = default;

    void setProject(kalara::architecture::Project* project);
    void runValidation();
    [[nodiscard]] const kalara::runtime::ValidationReport& lastReport() const noexcept { return m_lastReport; }

signals:
    void issueSelected(const std::vector<kalara::architecture::EntityId>& entityIds,
                       const std::optional<kalara::core::geometry::Point2D>& location);

private slots:
    void onRowDoubleClicked(int row, int column);
    void onFilterChanged(int index);

private:
    void setupUI();
    void populateTable();

    kalara::architecture::Project* m_project = nullptr;
    kalara::runtime::ValidationReport m_lastReport;

    QLabel* m_summaryLabel = nullptr;
    QComboBox* m_filterCombo = nullptr;
    QTableWidget* m_table = nullptr;
    QPushButton* m_validateBtn = nullptr;
};

} // namespace kalara::editor
