#pragma once
#include <QAction>
#include <QMap>
#include <functional>
namespace kalara::editor {
struct ActionSpec {
    QString id, label;
    QKeySequence shortcut;
    QString iconKey;
    std::function<bool()> enabled;
    std::function<void()> handler;
};
class ActionRegistry final : public QObject {
public:
    explicit ActionRegistry(QObject* parent) : QObject(parent) {}
    QAction* add(ActionSpec spec);
    QAction* get(const QString& id) const;
    void refresh();
    QList<QAction*> actions() const;
private:
    struct Entry { QAction* action; std::function<bool()> enabled; };
    QMap<QString, Entry> entries_;
};
}
