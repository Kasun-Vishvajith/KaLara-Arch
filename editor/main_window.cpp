#include "editor/main_window.h"
#include <QApplication>
#include <QCloseEvent>
#include <QDialog>
#include <QDockWidget>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QScreen>
#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>

namespace kalara::editor {
MainWindow::MainWindow(const QString& settingsFile)
    : settings_(settingsFile.isEmpty() ? std::make_unique<QSettings>() :
          std::make_unique<QSettings>(settingsFile, QSettings::IniFormat)) {
    setObjectName("mainWindow");
    setWindowTitle("KaLara Arch");
    setMinimumSize(1100, 700);
    resize(1366, 768);
    documents_ = new QTabWidget(this);
    documents_->setObjectName("documents");
    documents_->setAccessibleName("Open documents");
    documents_->setTabsClosable(true);
    setCentralWidget(documents_);
    hierarchy_ = new QDockWidget("Project", this);
    hierarchy_->setObjectName("projectDock");
    auto* hierarchyText = new QLabel("No architectural entities", hierarchy_);
    hierarchyText->setMargin(16);
    hierarchyText->setAlignment(Qt::AlignTop);
    hierarchy_->setWidget(hierarchyText);
    inspector_ = new QDockWidget("Inspector", this);
    inspector_->setObjectName("inspectorDock");
    summary_ = new QLabel("No selection", inspector_);
    summary_->setMargin(16);
    summary_->setAlignment(Qt::AlignTop);
    summary_->setAccessibleName("Selection summary");
    inspector_->setWidget(summary_);
    addDockWidget(Qt::LeftDockWidgetArea, hierarchy_);
    addDockWidget(Qt::RightDockWidgetArea, inspector_);
    auto always = [] { return true; };
    auto add = [&](QString id, QString label, QKeySequence key, QString icon,
                   std::function<void()> handler) {
        return registry_.add({id, label, key, icon, always, std::move(handler)});
    };
    auto* file = menuBar()->addMenu("&File");
    file->addAction(add("file.new", "&New empty document", QKeySequence::New, "document-new", [this] { newDocument(); }));
    file->addAction(add("file.close", "&Close document", QKeySequence("Ctrl+W"), "window-close", [this] {
        const int index = documents_->currentIndex();
        if (index < 0) return;
        auto* page = documents_->widget(index);
        documents_->removeTab(index);
        sessions_.erase(sessions_.begin() + index);
        delete page;
        updateSession();
    }));
    file->addSeparator();
    file->addAction(add("file.exit", "E&xit", QKeySequence::Quit, "application-exit", [this] { close(); }));
    auto* view = menuBar()->addMenu("&View");
    view->addAction(add("view.project", "Show/hide &Project", {}, "view-list-tree", [this] { hierarchy_->setVisible(!hierarchy_->isVisible()); }));
    view->addAction(add("view.inspector", "Show/hide &Inspector", {}, "document-properties", [this] { inspector_->setVisible(!inspector_->isVisible()); }));
    view->addAction(add("view.reset", "&Reset Workspace", {}, "view-restore", [this] { resetWorkspace(); }));
    view->addAction(add("view.theme", "Toggle light/&dark theme", {}, "preferences-desktop-theme", [this] { setDark(!dark_); }));
    auto* tools = menuBar()->addMenu("&Tools");
    tools->addAction(add("tools.search", "&Find command…", QKeySequence("Ctrl+Shift+P"), "edit-find", [this] { commandSearch(); }));
    auto* help = menuBar()->addMenu("&Help");
    help->addAction(add("help.about", "&About KaLara Arch", QKeySequence::HelpContents, "help-about", [this] {
        QMessageBox::about(this, "KaLara Arch", "Local 2D architectural workbench\nDevelopment build: application foundation\nDrafting tools are not available in this build.");
    }));
    statusBar()->setMinimumHeight(28);
    statusBar()->showMessage("Ready · Empty document · millimetres");
    connect(documents_, &QTabWidget::currentChanged, this, [this] { updateSession(); });
    connect(documents_, &QTabWidget::tabCloseRequested, this, [this](int index) {
        documents_->setCurrentIndex(index);
        registry_.get("file.close")->trigger();
    });
    resetWorkspace();
    restoreGeometry(settings_->value("workspace/geometry").toByteArray());
    restoreState(settings_->value("workspace/state").toByteArray(), 1);
    bool onScreen = false;
    for (auto* screen : QApplication::screens()) {
        if (screen->availableGeometry().contains(frameGeometry().topLeft() + QPoint(60, 20))) onScreen = true;
    }
    if (!onScreen) move(QApplication::primaryScreen()->availableGeometry().topLeft() + QPoint(24, 24));
    setDark(settings_->value("workspace/dark", false).toBool());
    newDocument();
}
MainWindow::~MainWindow() = default;
void MainWindow::newDocument() {
    auto session = std::make_unique<DocumentSession>(QString("Untitled %1").arg(nextDocument_++));
    auto* page = new QWidget;
    page->setObjectName("planCanvas");
    page->setAccessibleName("Empty plan canvas");
    auto* layout = new QVBoxLayout(page);
    auto* label = new QLabel("Empty plan", page);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    const auto title = session->title;
    connect(session.get(), &DocumentSession::selectionChanged, this, &MainWindow::updateSession);
    sessions_.push_back(std::move(session));
    documents_->setCurrentIndex(documents_->addTab(page, title));
    updateSession();
}
DocumentSession* MainWindow::activeSession() const {
    const auto index = documents_->currentIndex();
    return index >= 0 && index < static_cast<int>(sessions_.size()) ? sessions_[index].get() : nullptr;
}
void MainWindow::updateSession() {
    auto* session = activeSession();
    summary_->setText(session ? QString("%1\n\n%2 selected").arg(session->title).arg(session->selection().size()) : "No open document");
    setWindowTitle(session ? session->title + " — KaLara Arch" : "KaLara Arch");
    registry_.refresh();
    if (auto* action = registry_.get("file.close")) action->setEnabled(session != nullptr);
}
void MainWindow::resetWorkspace() {
    hierarchy_->setFloating(false);
    inspector_->setFloating(false);
    addDockWidget(Qt::LeftDockWidgetArea, hierarchy_);
    addDockWidget(Qt::RightDockWidgetArea, inspector_);
    hierarchy_->setVisible(width() > 1400);
    inspector_->show();
    resizeDocks({hierarchy_, inspector_}, {240, 300}, Qt::Horizontal);
}
void MainWindow::saveWorkspace() {
    settings_->setValue("workspace/geometry", saveGeometry());
    settings_->setValue("workspace/state", saveState(1));
    settings_->setValue("workspace/dark", dark_);
    settings_->sync();
    if (settings_->status() != QSettings::NoError) statusBar()->showMessage("Could not save workspace settings. Check your settings folder permissions.");
}
void MainWindow::closeEvent(QCloseEvent* event) {
    saveWorkspace();
    QMainWindow::closeEvent(event);
}
void MainWindow::setDark(bool dark) {
    dark_ = dark;
    const QString chrome = dark ? "#181C22" : "#F3F4F6";
    const QString panel = dark ? "#222832" : "#FFFFFF";
    const QString canvas = dark ? "#151A20" : "#FBFAF7";
    const QString text = dark ? "#E8EDF4" : "#202833";
    const QString divider = dark ? "#394352" : "#D9DFE7";
    setStyleSheet(QString("QWidget {color:%1; background:%2; font-family:'Segoe UI'; font-size:14px;}"
        "QDockWidget > QWidget, QMenu, QLineEdit, QListWidget {background:%3;}"
        "QWidget#planCanvas, QWidget#planCanvas QLabel {background:%4;}"
        "QMenuBar::item, QMenu::item {padding:8px 12px;} QMenu::item:selected {background:#2563EB; color:white;}"
        "QDockWidget::title {padding:10px; border-bottom:1px solid %5;}"
        "QTabBar::tab {padding:10px 16px;} QTabBar::tab:selected {border-bottom:2px solid #2563EB;}"
        "QLineEdit {padding:8px; border:1px solid %5;} QListWidget::item {padding:8px;}")
        .arg(text, chrome, panel, canvas, divider));
}
void MainWindow::commandSearch() {
    QDialog dialog(this);
    dialog.setWindowTitle("Find command");
    dialog.resize(480, 360);
    auto* layout = new QVBoxLayout(&dialog);
    auto* input = new QLineEdit(&dialog);
    input->setAccessibleName("Search commands");
    input->setPlaceholderText("Type a command name");
    auto* list = new QListWidget(&dialog);
    list->setAccessibleName("Matching commands");
    layout->addWidget(input);
    layout->addWidget(list);
    auto populate = [&] {
        list->clear();
        for (auto* action : registry_.actions()) {
            if (action->objectName() == "tools.search" || !action->isEnabled()) continue;
            const auto label = action->text().remove('&');
            if (!label.contains(input->text(), Qt::CaseInsensitive)) continue;
            auto* item = new QListWidgetItem(label + "   " + action->shortcut().toString(), list);
            item->setData(Qt::UserRole, action->objectName());
        }
        if (list->count()) list->setCurrentRow(0);
    };
    QString chosen;
    auto choose = [&] { if (auto* item = list->currentItem()) { chosen = item->data(Qt::UserRole).toString(); dialog.accept(); } };
    connect(input, &QLineEdit::textChanged, &dialog, populate);
    connect(input, &QLineEdit::returnPressed, &dialog, choose);
    connect(list, &QListWidget::itemActivated, &dialog, choose);
    populate();
    input->setFocus();
    if (dialog.exec() == QDialog::Accepted && !chosen.isEmpty()) registry_.get(chosen)->trigger();
}
}

