#include "editor/session.h"
#include <utility>
namespace kalara::editor {
DocumentSession::DocumentSession(QString name, QObject* parent)
    : QObject(parent), title(std::move(name)) {
    architecture::Project project(architecture::EntityId::generate());
    project.title=title.toUtf8().toStdString();
    projectStore_=std::make_unique<runtime::ProjectStore>(std::move(project));
    commands_=std::make_unique<runtime::CommandService>(*projectStore_);
}
void DocumentSession::select(std::set<architecture::EntityId> ids) {
    if (ids == selection_) return;
    selection_ = std::move(ids);
    emit selectionChanged();
}
}
