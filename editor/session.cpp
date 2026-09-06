#include "editor/session.h"
#include <utility>
namespace kalara::editor {
DocumentSession::DocumentSession(QString name, QObject* parent)
    : QObject(parent), title(std::move(name)) {}
void DocumentSession::select(std::set<std::uint64_t> ids) {
    if (ids == selection_) return;
    selection_ = std::move(ids);
    emit selectionChanged();
}
}
