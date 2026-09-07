#pragma once
#include <QObject>
#include <QPointF>
#include <QString>
#include <QUuid>
#include <set>
#include <cstdint>
#include "runtime/command_service.h"
#include "editor/input_system.h"

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
    const std::set<architecture::EntityId>& selection() const { return selection_; }
    void select(std::set<architecture::EntityId> ids);
    runtime::ProjectStore& projectStore() { return *projectStore_; }
    runtime::CommandService& commands() { return *commands_; }
    ToolController& tools() { return tools_; }
signals:
    void selectionChanged();
private:
    std::set<architecture::EntityId> selection_;
    std::unique_ptr<runtime::ProjectStore> projectStore_;
    std::unique_ptr<runtime::CommandService> commands_;
    ToolController tools_;
};
}
