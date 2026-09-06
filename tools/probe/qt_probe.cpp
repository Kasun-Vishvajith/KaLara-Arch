#include <QApplication>
#include <QLabel>
#include <QTimer>
#include <QScreen>
#include <QPixmap>
#include <iostream>
int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QLabel window("KaLara Arch — Windows Qt environment probe");
    window.resize(640, 240);
    window.show();
    QTimer::singleShot(700, &app, [&] { const bool saved = window.screen()->grabWindow(window.winId()).save("docs/evidence/step00-window.png"); std::cout << "visible=" << window.isVisible() << " capture=" << saved << " platform=" << app.platformName().toStdString() << std::endl; app.exit(saved && window.isVisible() ? 0 : 1); });
    return app.exec();
}

