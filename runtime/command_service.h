#pragma once
#include "architecture/model.h"
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace kalara::runtime {
struct EntityMutation {
    architecture::EntityId id;
    std::optional<architecture::Entity> replacement;
};
struct CommandRequest {
    std::string commandId;
    architecture::EntityId projectId;
    std::uint64_t expectedRevision = 0;
    std::string label;
    std::vector<EntityMutation> mutations;
    std::string scope;
    std::string policySummary;
};
struct EntityDelta {
    architecture::EntityId id;
    std::optional<architecture::Entity> before;
    std::optional<architecture::Entity> after;
};
class ChangePlan {
public:
    const architecture::EntityId& projectId() const { return projectId_; }
    std::uint64_t baseRevision() const { return baseRevision_; }
    const std::string& commandId() const { return commandId_; }
    const std::string& label() const { return label_; }
    const std::vector<EntityDelta>& deltas() const { return deltas_; }
    const std::vector<Diagnostic>& warnings() const { return warnings_; }
    const std::string& scope() const { return scope_; }
    const std::string& policySummary() const { return policySummary_; }
private:
    friend class CommandService;
    architecture::EntityId projectId_{"invalid"};
    std::uint64_t baseRevision_ = 0;
    std::string commandId_, label_, scope_, policySummary_;
    std::vector<EntityDelta> deltas_;
    std::vector<Diagnostic> warnings_;
};
struct PrepareFailure { std::vector<Diagnostic> diagnostics; };
using PrepareResult = std::variant<ChangePlan, PrepareFailure>;
struct CommitSuccess {
    std::uint64_t revision = 0;
    std::uint64_t historyId = 0;
    std::vector<architecture::EntityId> changedIds;
    bool noOp = false;
};
struct CommitFailure { std::vector<Diagnostic> diagnostics; };
using CommitResult = std::variant<CommitSuccess, CommitFailure>;
struct CommittedEvent {
    std::uint64_t revision = 0;
    std::uint64_t historyId = 0;
    std::string label;
    std::vector<architecture::EntityId> changedIds;
};
class ProjectStore {
public:
    explicit ProjectStore(architecture::Project project);
    std::shared_ptr<const architecture::Project> snapshot() const { return state_; }
private:
    friend class CommandService;
    std::shared_ptr<const architecture::Project> state_;
};
class CommandService {
public:
    using DependencyHook = std::function<std::vector<Diagnostic>(const architecture::Project&, const std::vector<EntityDelta>&)>;
    using EventSink = std::function<void(const CommittedEvent&)>;
    explicit CommandService(ProjectStore& store, std::size_t historyBudgetBytes = 256ULL*1024*1024);
    PrepareResult prepare(const CommandRequest& request) const;
    CommitResult commit(const ChangePlan& plan);
    CommitResult undo();
    CommitResult redo();
    void addDependencyHook(DependencyHook hook);
    void setEventSink(EventSink sink) { eventSink_ = std::move(sink); }
    void markSaved(std::uint64_t revision);
    bool dirty() const;
    std::size_t undoCount() const { return cursor_; }
    std::size_t redoCount() const { return history_.size()-cursor_; }
    std::size_t retainedBytes() const { return retainedBytes_; }
    std::uint64_t retainedHistoryBoundary() const { return retainedBoundary_; }
    // Tests can inject a failure after N deltas have been applied to the candidate.
    void injectFailureAfter(std::optional<std::size_t> count) { failAfter_ = count; }
private:
    struct HistoryEntry { std::uint64_t id; std::string label; std::vector<EntityDelta> deltas; std::size_t bytes; };
    CommitResult applyHistory(const HistoryEntry& entry, bool forward);
    CommitFailure fail(std::string code, std::string message) const;
    static std::size_t estimate(const std::vector<EntityDelta>& deltas);
    void publish(std::uint64_t historyId, const std::string& label, const std::vector<EntityDelta>& deltas);
    void prune();
    ProjectStore& store_;
    std::vector<HistoryEntry> history_;
    std::size_t cursor_ = 0, retainedBytes_ = 0, budget_;
    std::uint64_t nextHistoryId_ = 1, retainedBoundary_ = 0;
    std::optional<architecture::Project> savedState_;
    std::vector<DependencyHook> hooks_;
    EventSink eventSink_;
    std::optional<std::size_t> failAfter_;
};
}
