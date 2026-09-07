#pragma once
#include "editor/action_registry.h"
#include "editor/session.h"
#include <QMainWindow>
#include <QSettings>
#include <memory>
#include <vector>
class QTabWidget;
class QDockWidget;
class QLabel;
class QLineEdit;
class QComboBox;
namespace kalara::editor {
class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(const QString& settingsFile = {});
    ~MainWindow() override;
    ActionRegistry& registry() { return registry_; }
    DocumentSession* activeSession() const;
    void resetWorkspace();
    void saveWorkspace();
    void setDark(bool dark);
protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
private:
    void newDocument();
    void updateSession();
    void commandSearch();
    std::unique_ptr<QSettings> settings_;
    ActionRegistry registry_{this};
    QTabWidget* documents_;
    QDockWidget* hierarchy_;
    QDockWidget* inspector_;
    QLabel* summary_;
    QLineEdit* wallThickness_;
    QComboBox* wallType_;
    QComboBox* wallReference_;
    std::vector<std::unique_ptr<DocumentSession>> sessions_;
    bool dark_ = false;
    int nextDocument_ = 1;
};
}
