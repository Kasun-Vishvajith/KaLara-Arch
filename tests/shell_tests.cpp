#include "editor/main_window.h"
#include "editor/plan_viewport.h"
#include <QOpenGLWidget>
#include <QDockWidget>
#include <QTemporaryDir>
#include <QTabWidget>
#include <QtTest>
#include <QScreen>
#include <QLineEdit>
#include <QStatusBar>
class ShellTests : public QObject {
    Q_OBJECT
private slots:
    void nativeVisual() {
        if (QGuiApplication::platformName() != "windows") QSKIP("Native visual evidence requires windows platform");
        QTemporaryDir temp;
        kalara::editor::MainWindow window(temp.filePath("visual.ini"));
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        auto* viewport=window.findChild<kalara::editor::PlanViewport*>("planViewport");
        QVERIFY(viewport);
        QCOMPARE(viewport->activeMode(),kalara::editor::RendererMode::gpu);
        auto* gl=viewport->findChild<QOpenGLWidget*>();
        QVERIFY(gl);
        QTRY_VERIFY_WITH_TIMEOUT(gl->isValid(),5000);
        kalara::render::Scene2D scene;
        scene.fills.push_back({{{{0,-100},{6000,-100},{6000,100},{0,100}},{}},{224,226,230,255},0});
        scene.lines.push_back({{0,-100},{6000,-100},.35,{32,40,51,255},0});
        scene.lines.push_back({{0,100},{6000,100},.35,{32,40,51,255},0});
        scene.texts.push_back({{3000,350},"6000 mm",150,{32,40,51,255}});
        scene.bounds=kalara::geometry::AABB2{{0,-100},{6000,500}};
        viewport->setScene(scene);
        viewport->fitScene();
        gl->setFocus();
        QVERIFY(QTest::qWaitForWindowActive(&window));
        QTest::keyClick(gl,Qt::Key_F3);
        QVERIFY(!viewport->snapEnabled());
        QTest::keyClick(gl,Qt::Key_F3);
        QVERIFY(viewport->snapEnabled());
        QTest::keyClick(gl,Qt::Key_V);
        QVERIFY(window.statusBar()->currentMessage().startsWith("Select"));
        for (bool dark : {false, true}) {
            window.setDark(dark);
            for (const QSize size : {QSize(1366, 768), QSize(1920, 1080)}) {
                window.resize(size);
                window.resetWorkspace();
                QTest::qWait(150);
                qInfo() << "Requested DIP" << size << "Actual DIP" << window.size() << "DPR" << window.devicePixelRatioF();
                const auto file = QString("docs/evidence/step05-gpu-%1-%2.png").arg(dark ? "dark" : "light").arg(size.width());
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
        viewport=window.findChild<kalara::editor::PlanViewport*>("planViewport");
        viewport->setRendererMode(kalara::editor::RendererMode::painter);
        QCOMPARE(viewport->activeMode(),kalara::editor::RendererMode::painter);
        QTest::qWait(100);
        const auto stats=viewport->frames().statistics();
        QVERIFY(stats.count>0);
        qInfo() << "Native frame samples" << stats.count << "median" << stats.medianMs << "p95" << stats.p95Ms << "p99" << stats.p99Ms << "max" << stats.maxMs;
        QVERIFY(window.screen()->grabWindow(window.winId()).save("docs/evidence/step05-painter-native.png"));
        QLineEdit input(&window);
        input.show();input.setFocus();
        QTest::keyClick(&input,Qt::Key_W);
        QCOMPARE(input.text(),QString("w"));
        input.hide();
        window.close();
        QVERIFY(!window.isVisible());
    }
    void sessionsAndActions() {
        QTemporaryDir temp;
        kalara::editor::MainWindow window(temp.filePath("settings.ini"));
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        auto* first = window.activeSession();
        first->select({kalara::architecture::EntityId("one"),kalara::architecture::EntityId("two")});
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



