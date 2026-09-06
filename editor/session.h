#pragma once
#include <QObject>
#include <QPointF>
#include <QString>
#include <QUuid>
#include <set>
#include <cstdint>

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
signals:
    void selectionChanged();
private:
    std::set<std::uint64_t> selection_;
};
}
