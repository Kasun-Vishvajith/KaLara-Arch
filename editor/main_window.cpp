#include "editor/main_window.h"
#include "editor/plan_viewport.h"
#include "render/scene.h"
#include <QApplication>
#include <QCloseEvent>
#include <QDialog>
#include <QDockWidget>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>
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
    auto* edit=menuBar()->addMenu("&Edit");
    edit->addAction(add("tool.select", "&Select", {}, "edit-select", [this] { if(auto* viewport=qobject_cast<PlanViewport*>(documents_->currentWidget()))viewport->setFocus();statusBar()->showMessage("Select · click, drag marquee, Shift toggles, Alt cycles"); }));
    auto* view = menuBar()->addMenu("&View");
    view->addAction(add("view.project", "Show/hide &Project", {}, "view-list-tree", [this] { hierarchy_->setVisible(!hierarchy_->isVisible()); }));
    view->addAction(add("view.inspector", "Show/hide &Inspector", {}, "document-properties", [this] { inspector_->setVisible(!inspector_->isVisible()); }));
    view->addAction(add("view.reset", "&Reset Workspace", {}, "view-restore", [this] { resetWorkspace(); }));
    view->addAction(add("view.theme", "Toggle light/&dark theme", {}, "preferences-desktop-theme", [this] { setDark(!dark_); }));
    view->addAction(add("view.fit", "&Fit plan", QKeySequence("Ctrl+0"), "zoom-fit-best", [this] { if(auto* viewport=qobject_cast<PlanViewport*>(documents_->currentWidget()))viewport->fitScene(); }));
    auto* fallback=add("view.painter", "Use &Painter fallback", {}, "video-display", [this] { if(auto* viewport=qobject_cast<PlanViewport*>(documents_->currentWidget()))viewport->setRendererMode(registry_.get("view.painter")->isChecked()?RendererMode::painter:RendererMode::gpu); });
    fallback->setCheckable(true);
    auto* snapAction=add("view.snap", "Enable &Snapping", QKeySequence(Qt::Key_F3), "snap-nodes", [this] { if(auto* viewport=qobject_cast<PlanViewport*>(documents_->currentWidget()))viewport->toggleSnap(); });snapAction->setCheckable(true);snapAction->setChecked(true);view->addAction(snapAction);
    auto* orthoAction=add("view.ortho", "Enable &Ortho", QKeySequence(Qt::Key_F8), "transform-move-horizontal", [this] { if(auto* viewport=qobject_cast<PlanViewport*>(documents_->currentWidget()))viewport->toggleOrtho(); });orthoAction->setCheckable(true);view->addAction(orthoAction);
    auto* gridAction=add("view.grid_snap", "Enable &Grid Snap", QKeySequence(Qt::Key_F9), "view-grid", [this] { if(auto* viewport=qobject_cast<PlanViewport*>(documents_->currentWidget()))viewport->toggleGrid(); });gridAction->setCheckable(true);view->addAction(gridAction);
    auto* tools = menuBar()->addMenu("&Tools");
    tools->addAction(add("tools.search", "&Find command…", QKeySequence("Ctrl+K"), "edit-find", [this] { commandSearch(); }));
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
    auto* page = new PlanViewport;
    page->setObjectName("planViewport");
    page->setAccessibleName("Empty plan canvas");
    const render::SceneBuilder builder;
    const auto snapshot=session->projectStore().snapshot();
    page->setScene(builder.build(*snapshot,{{{-100000,-100000},{100000,100000}},std::nullopt}));
    page->setSession(session.get());
    connect(page,&PlanViewport::cameraChanged,this,[this,page]{statusBar()->showMessage(QString("Ready · millimetres · %1% · %2 renderer").arg(qRound(page->camera().pixelsPerMm*1000)).arg(page->activeMode()==RendererMode::gpu?"GPU":"Painter"));});
    connect(page,&PlanViewport::inputSettingsChanged,this,[this,page]{statusBar()->showMessage(QString("Select · Snap %1 · Ortho %2 · Grid snap %3").arg(page->snapEnabled()?"on":"off",page->orthoEnabled()?"on":"off",page->gridSnapEnabled()?"on":"off"));});
    connect(page,&PlanViewport::actionRequested,this,[this](const QString& id){if(auto* action=registry_.get(id))action->trigger();});
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
void MainWindow::keyPressEvent(QKeyEvent* event){
    const bool textFocused=qobject_cast<QLineEdit*>(QApplication::focusWidget())!=nullptr;
    if(routeSingleLetterShortcut({event->key(),event->modifiers().testFlag(Qt::ShiftModifier),event->modifiers().testFlag(Qt::ControlModifier),event->modifiers().testFlag(Qt::AltModifier),textFocused,true})&&event->key()==Qt::Key_V){registry_.get("tool.select")->trigger();event->accept();return;}
    QMainWindow::keyPressEvent(event);
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
        "QWidget#planViewport, QWidget#planViewport QLabel {background:%4;}"
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

