#include "editor/main_window.h"
#include <QDockWidget>
#include <QTemporaryDir>
#include <QTabWidget>
#include <QtTest>
#include <QScreen>
class ShellTests : public QObject {
    Q_OBJECT
private slots:
    void nativeVisual() {
        if (QGuiApplication::platformName() != "windows") QSKIP("Native visual evidence requires windows platform");
        QTemporaryDir temp;
        kalara::editor::MainWindow window(temp.filePath("visual.ini"));
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        for (bool dark : {false, true}) {
            window.setDark(dark);
            for (const QSize size : {QSize(1366, 768), QSize(1920, 1080)}) {
                window.resize(size);
                window.resetWorkspace();
                QTest::qWait(150);
                qInfo() << "Requested DIP" << size << "Actual DIP" << window.size() << "DPR" << window.devicePixelRatioF();
                const auto file = QString("docs/evidence/step01-%1-%2.png").arg(dark ? "dark" : "light").arg(size.width());
                QVERIFY(window.screen()->grabWindow(window.winId()).save(file));
            }
        }
        window.resize(1366, 768);
        window.move(20, 20);
        window.raise();
        window.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&window));
        QTest::keyClick(&window, Qt::Key_N, Qt::ControlModifier);
        QCOMPARE(window.findChild<QTabWidget*>("documents")->count(), 2);
        QTest::qWait(100);
        QTest::keyClick(&window, Qt::Key_W, Qt::ControlModifier);
        QCOMPARE(window.findChild<QTabWidget*>("documents")->count(), 1);
        window.close();
        QVERIFY(!window.isVisible());
    }
    void sessionsAndActions() {
        QTemporaryDir temp;
        kalara::editor::MainWindow window(temp.filePath("settings.ini"));
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        auto* first = window.activeSession();
        first->select({1,2});
        window.registry().get("file.new")->trigger();
        QVERIFY(first != window.activeSession());
        QVERIFY(window.activeSession()->selection().empty());
        window.findChild<QTabWidget*>("documents")->setCurrentIndex(0);
        QCOMPARE(window.activeSession(), first);
        QCOMPARE(first->selection().size(), std::size_t(2));
        auto actions = window.registry().actions();
        QSet<QString> ids;
        for (auto* action : actions) { QVERIFY(!ids.contains(action->objectName())); ids.insert(action->objectName()); }
        window.registry().get("file.close")->trigger();
        QVERIFY(window.activeSession() != nullptr);
        window.registry().get("file.close")->trigger();
        QVERIFY(window.activeSession() == nullptr);
        QVERIFY(!window.registry().get("file.close")->isEnabled());
    }
    void persistenceAndReset() {
        QTemporaryDir temp;
        const auto path = temp.filePath("settings.ini");
        {
            kalara::editor::MainWindow window(path);
            window.show();
            QVERIFY(QTest::qWaitForWindowExposed(&window));
            auto* inspector = window.findChild<QDockWidget*>("inspectorDock");
            window.addDockWidget(Qt::LeftDockWidgetArea, inspector);
            window.setDark(true);
            window.saveWorkspace();
        }
        kalara::editor::MainWindow restored(path);
        restored.show();
        QVERIFY(QTest::qWaitForWindowExposed(&restored));
        auto* inspector = restored.findChild<QDockWidget*>("inspectorDock");
        QCOMPARE(restored.dockWidgetArea(inspector), Qt::LeftDockWidgetArea);
        QVERIFY(restored.styleSheet().contains("#151A20"));
        restored.registry().get("view.reset")->trigger();
        QCOMPARE(restored.dockWidgetArea(inspector), Qt::RightDockWidgetArea);
        QVERIFY(inspector->isVisible());
        restored.close();
    }
};
QTEST_MAIN(ShellTests)
#include "shell_tests.moc"



