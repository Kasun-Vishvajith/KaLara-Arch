#pragma once
#include <QMainWindow>

namespace kalara::editor {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;
};

} // namespace kalara::editor
