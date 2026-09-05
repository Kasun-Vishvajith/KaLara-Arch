#include "kalara/editor/library_browser_widget.hpp"
#include <QGroupBox>

namespace kalara::editor {

LibraryBrowserWidget::LibraryBrowserWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    // Title / Header
    auto *header = new QLabel("<b>2D Architectural Library</b>", this);
    layout->addWidget(header);

    // Search bar
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search objects (e.g. bed, toilet, stair)...");
    layout->addWidget(m_searchEdit);

    // Category combo box
    m_categoryCombo = new QComboBox(this);
    populateCategories();
    layout->addWidget(m_categoryCombo);

    // Item List
    m_listWidget = new QListWidget(this);
    layout->addWidget(m_listWidget);

    // Details box
    m_detailsLabel = new QLabel("Select an architectural symbol to view dimensions and clearances.", this);
    m_detailsLabel->setWordWrap(true);
    m_detailsLabel->setStyleSheet("color: #b0b4c0; font-size: 11px; padding: 4px;");
    layout->addWidget(m_detailsLabel);

    // Place button
    m_placeButton = new QPushButton("Place in Drawing", this);
    m_placeButton->setEnabled(false);
    layout->addWidget(m_placeButton);

    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LibraryBrowserWidget::onCategoryChanged);
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &LibraryBrowserWidget::onSearchTextChanged);
    connect(m_listWidget, &QListWidget::itemSelectionChanged, this, [this]() {
        const auto* item = selectedItem();
        if (item) {
            m_placeButton->setEnabled(true);
            QString details = QString("<b>%1</b> (%2)<br>Dim: %3 x %4 mm<br>Clearance: F:%5 R:%6 L:%7 R:%8 mm")
                .arg(QString::fromStdString(item->name))
                .arg(QString::fromStdString(item->id))
                .arg(item->width_mm, 0, 'f', 0)
                .arg(item->length_mm, 0, 'f', 0)
                .arg(item->clearance.front_mm, 0, 'f', 0)
                .arg(item->clearance.rear_mm, 0, 'f', 0)
                .arg(item->clearance.left_mm, 0, 'f', 0)
                .arg(item->clearance.right_mm, 0, 'f', 0);
            m_detailsLabel->setText(details);
            emit itemSelected(item->id);
        } else {
            m_placeButton->setEnabled(false);
            m_detailsLabel->setText("Select an architectural symbol to view dimensions and clearances.");
        }
    });
    connect(m_listWidget, &QListWidget::itemDoubleClicked,
            this, &LibraryBrowserWidget::onItemDoubleClicked);
    connect(m_placeButton, &QPushButton::clicked,
            this, &LibraryBrowserWidget::onPlaceButtonClicked);

    refreshItemList();
}

void LibraryBrowserWidget::populateCategories() {
    m_categoryCombo->addItem("All Categories");
    m_categoryCombo->addItem(QString::fromStdString(std::string(kalara::architecture::libraryCategoryName(kalara::architecture::LibraryCategory::Furniture))));
    m_categoryCombo->addItem(QString::fromStdString(std::string(kalara::architecture::libraryCategoryName(kalara::architecture::LibraryCategory::BathroomFixtures))));
    m_categoryCombo->addItem(QString::fromStdString(std::string(kalara::architecture::libraryCategoryName(kalara::architecture::LibraryCategory::KitchenObjects))));
    m_categoryCombo->addItem(QString::fromStdString(std::string(kalara::architecture::libraryCategoryName(kalara::architecture::LibraryCategory::Stairs))));
    m_categoryCombo->addItem(QString::fromStdString(std::string(kalara::architecture::libraryCategoryName(kalara::architecture::LibraryCategory::SiteOutdoor))));
}

void LibraryBrowserWidget::refreshItemList() {
    m_listWidget->clear();

    std::string query = m_searchEdit->text().toStdString();
    std::vector<const kalara::architecture::LibraryItem*> items;

    if (!query.empty()) {
        items = m_catalog.searchItems(query);
    } else {
        int catIndex = m_categoryCombo->currentIndex();
        if (catIndex <= 0) {
            items = m_catalog.searchItems("");
        } else {
            auto cat = static_cast<kalara::architecture::LibraryCategory>(catIndex - 1);
            items = m_catalog.getItemsByCategory(cat);
        }
    }

    for (const auto* item : items) {
        auto *listItem = new QListWidgetItem(QString::fromStdString(item->name), m_listWidget);
        listItem->setData(Qt::UserRole, QString::fromStdString(item->id));
    }
}

const kalara::architecture::LibraryItem* LibraryBrowserWidget::selectedItem() const {
    auto *cur = m_listWidget->currentItem();
    if (!cur) return nullptr;
    std::string id = cur->data(Qt::UserRole).toString().toStdString();
    return m_catalog.findItem(id);
}

void LibraryBrowserWidget::onCategoryChanged(int) {
    refreshItemList();
}

void LibraryBrowserWidget::onSearchTextChanged(const QString&) {
    refreshItemList();
}

void LibraryBrowserWidget::onItemDoubleClicked(QListWidgetItem *item) {
    if (item) {
        std::string id = item->data(Qt::UserRole).toString().toStdString();
        emit placeItemRequested(id);
    }
}

void LibraryBrowserWidget::onPlaceButtonClicked() {
    const auto* item = selectedItem();
    if (item) {
        emit placeItemRequested(item->id);
    }
}

} // namespace kalara::editor
