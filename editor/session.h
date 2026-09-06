#pragma once
#include <QObject>
#include <QPointF>
#include <QString>
#include <QUuid>
#include <set>
#include <cstdint>
#include "runtime/command_service.h"

namespace kalara::editor {
// Only transient editor state. Architectural state/history are introduced by
// their steps, and belong to this session rather than to the application.
class DocumentSession final : public QObject {
    Q_OBJECT
public:
    explicit DocumentSession(QString title, QObject* parent = nullptr);
    const QUuid id = QUuid::createUuid();
    QString title;
    QPointF cameraCenter;
    double pixelsPerMm = 0.1;
    const std::set<std::uint64_t>& selection() const { return selection_; }
    void select(std::set<std::uint64_t> ids);
    runtime::ProjectStore& projectStore() { return *projectStore_; }
    runtime::CommandService& commands() { return *commands_; }
signals:
    void selectionChanged();
private:
    std::set<std::uint64_t> selection_;
    std::unique_ptr<runtime::ProjectStore> projectStore_;
    std::unique_ptr<runtime::CommandService> commands_;
};
}
