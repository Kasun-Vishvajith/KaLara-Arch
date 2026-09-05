#pragma once
#include "kalara/architecture/library_catalog.hpp"
#include <QWidget>
#include <QListWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

namespace kalara::editor {

/// Dockable Architectural 2D Library Browser Widget (Step 11).
class LibraryBrowserWidget : public QWidget {
    Q_OBJECT

public:
    explicit LibraryBrowserWidget(QWidget *parent = nullptr);
    ~LibraryBrowserWidget() override = default;

    [[nodiscard]] const kalara::architecture::LibraryCatalog& catalog() const noexcept {
        return m_catalog;
    }

    [[nodiscard]] kalara::architecture::LibraryCatalog& catalog() noexcept {
        return m_catalog;
    }

    [[nodiscard]] const kalara::architecture::LibraryItem* selectedItem() const;

signals:
    void itemSelected(const std::string& itemId);
    void placeItemRequested(const std::string& itemId);

private slots:
    void onCategoryChanged(int index);
    void onSearchTextChanged(const QString& text);
    void onItemDoubleClicked(QListWidgetItem *item);
    void onPlaceButtonClicked();

private:
    void populateCategories();
    void refreshItemList();

    kalara::architecture::LibraryCatalog m_catalog;
    QComboBox *m_categoryCombo = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QListWidget *m_listWidget = nullptr;
    QLabel *m_detailsLabel = nullptr;
    QPushButton *m_placeButton = nullptr;
};

} // namespace kalara::editor
