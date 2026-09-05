#pragma once
#include "kalara/architecture/project.hpp"
#include "kalara/architecture/project_serializer.hpp"
#include "kalara/core/json.hpp"
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <optional>
#include <string_view>

namespace kalara::runtime {

/// Entity initiating an architectural mutation (Rule 6, Section 11, Section 13).
enum class TransactionActor {
    Human,
    AIAgent,
    System
};

constexpr std::string_view actorName(TransactionActor actor) noexcept {
    switch (actor) {
        case TransactionActor::Human:   return "Human";
        case TransactionActor::AIAgent: return "AI Agent";
        case TransactionActor::System:  return "System";
    }
    return "Unknown";
}

/// Lifecycle state of a transaction.
enum class TransactionStatus {
    Active,
    Committed,
    RolledBack
};

/// A logical architectural transaction grouping operations with before/after state tracking (Step 16 & Section 13).
struct Transaction {
    std::string id;                                     // Stable unique transaction ID (e.g. "tx_1042")
    TransactionActor actor = TransactionActor::Human;   // Who performed the action
    std::string intent;                                 // High-level design intent (e.g. "Create master bedroom")
    std::vector<std::string> operations;                // Grouped semantic operations
    std::chrono::system_clock::time_point timestamp;    // When executed
    TransactionStatus status = TransactionStatus::Active;
    bool isSuccessful = true;

    // Before and after full architectural state snapshots (Step 15 serializer foundation)
    kalara::core::JsonObject beforeState;
    kalara::core::JsonObject afterState;

    [[nodiscard]] size_t operationCount() const noexcept { return operations.size(); }
};

/// Manages the transaction lifecycle, grouped human/AI operations, before/after snapshots, and Undo/Redo stacks.
class TransactionManager {
public:
    explicit TransactionManager(size_t maxHistorySize = 100);

    /// Begin a new logical transaction, capturing the pre-mutation state.
    bool beginTransaction(const kalara::architecture::Project& project,
                          std::string intent,
                          TransactionActor actor = TransactionActor::Human);

    /// Record a sub-operation within the active transaction.
    void recordOperation(std::string operationDescription);

    /// Commit the active transaction, capturing the post-mutation state and pushing to the undo stack.
    bool commitTransaction(const kalara::architecture::Project& project);

    /// Rollback active uncommitted transaction and restore project to its before-state.
    bool rollbackTransaction(kalara::architecture::Project& project);

    /// Check if an undo action is available.
    [[nodiscard]] bool canUndo() const noexcept;

    /// Check if a redo action is available.
    [[nodiscard]] bool canRedo() const noexcept;

    /// Revert the last committed transaction and restore its before-state.
    bool undo(kalara::architecture::Project& project);

    /// Re-apply the last undone transaction and restore its after-state.
    bool redo(kalara::architecture::Project& project);

    /// Clear all undo and redo history.
    void clear() noexcept;

    /// Access history stacks.
    [[nodiscard]] const std::vector<Transaction>& undoStack() const noexcept { return m_undoStack; }
    [[nodiscard]] const std::vector<Transaction>& redoStack() const noexcept { return m_redoStack; }

    /// Active transaction pointer (nullptr if none active).
    [[nodiscard]] const Transaction* activeTransaction() const noexcept {
        return m_activeTransaction.has_value() ? &m_activeTransaction.value() : nullptr;
    }

    /// Intent descriptions for UI menus.
    [[nodiscard]] std::string nextUndoIntent() const;
    [[nodiscard]] std::string nextRedoIntent() const;

private:
    size_t m_maxHistorySize;
    uint64_t m_transactionCounter = 1;
    std::optional<Transaction> m_activeTransaction;
    std::vector<Transaction> m_undoStack;
    std::vector<Transaction> m_redoStack;

    std::string generateTransactionId();
    bool applyState(kalara::architecture::Project& target, const kalara::core::JsonObject& state);
};

/// RAII convenience scope guard for transactions.
/// Automatically calls rollbackTransaction if destroyed without being committed.
class TransactionScope {
public:
    TransactionScope(TransactionManager& tm,
                     kalara::architecture::Project& project,
                     std::string intent,
                     TransactionActor actor = TransactionActor::Human)
        : m_tm(tm), m_project(project)
    {
        m_tm.beginTransaction(m_project, std::move(intent), actor);
    }

    ~TransactionScope() {
        if (!m_committed && m_tm.activeTransaction()) {
            m_tm.rollbackTransaction(m_project);
        }
    }

    TransactionScope(const TransactionScope&) = delete;
    TransactionScope& operator=(const TransactionScope&) = delete;

    void record(std::string op) {
        m_tm.recordOperation(std::move(op));
    }

    bool commit() {
        m_committed = m_tm.commitTransaction(m_project);
        return m_committed;
    }

private:
    TransactionManager& m_tm;
    kalara::architecture::Project& m_project;
    bool m_committed = false;
};

} // namespace kalara::runtime
