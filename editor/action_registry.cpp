#include "editor/action_registry.h"
#include <QIcon>
#include <stdexcept>
namespace kalara::editor {
QAction* ActionRegistry::add(ActionSpec spec) {
    if (entries_.contains(spec.id)) throw std::logic_error("Duplicate action ID");
    auto* action = new QAction(spec.label, this);
    action->setObjectName(spec.id);
    action->setShortcut(spec.shortcut);
    action->setIcon(QIcon::fromTheme(spec.iconKey));
    connect(action, &QAction::triggered, this, [handler = std::move(spec.handler)] { handler(); });
    entries_.insert(spec.id, {action, std::move(spec.enabled)});
    refresh();
    return action;
}
QAction* ActionRegistry::get(const QString& id) const {
    auto it = entries_.constFind(id);
    return it == entries_.cend() ? nullptr : it->action;
}
void ActionRegistry::refresh() {
    for (const auto& entry : entries_) entry.action->setEnabled(!entry.enabled || entry.enabled());
}
QList<QAction*> ActionRegistry::actions() const {
    QList<QAction*> result;
    for (const auto& entry : entries_) result.push_back(entry.action);
    return result;
}
}
