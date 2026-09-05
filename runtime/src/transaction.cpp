#include "kalara/runtime/transaction.hpp"
#include <sstream>

namespace kalara::runtime {

TransactionManager::TransactionManager(size_t maxHistorySize)
    : m_maxHistorySize(maxHistorySize) {
}

std::string TransactionManager::generateTransactionId() {
    std::ostringstream oss;
    oss << "tx_" << m_transactionCounter++;
    return oss.str();
}

bool TransactionManager::beginTransaction(const kalara::architecture::Project& project,
                                          std::string intent,
                                          TransactionActor actor) {
    if (m_activeTransaction.has_value()) {
        // A transaction is already active. Nested or uncommitted transaction cannot begin.
        return false;
    }

    Transaction tx;
    tx.id = generateTransactionId();
    tx.actor = actor;
    tx.intent = std::move(intent);
    tx.timestamp = std::chrono::system_clock::now();
    tx.status = TransactionStatus::Active;
    tx.isSuccessful = true;
    tx.beforeState = kalara::architecture::ProjectSerializer::serializeProject(project);

    m_activeTransaction = std::move(tx);
    return true;
}

void TransactionManager::recordOperation(std::string operationDescription) {
    if (m_activeTransaction.has_value()) {
        m_activeTransaction->operations.push_back(std::move(operationDescription));
    }
}

bool TransactionManager::commitTransaction(const kalara::architecture::Project& project) {
    if (!m_activeTransaction.has_value()) {
        return false;
    }

    Transaction tx = std::move(*m_activeTransaction);
    m_activeTransaction.reset();

    tx.afterState = kalara::architecture::ProjectSerializer::serializeProject(project);
    tx.status = TransactionStatus::Committed;

    // Pushing a new transaction invalidates the redo stack
    m_redoStack.clear();

    m_undoStack.push_back(std::move(tx));

    if (m_undoStack.size() > m_maxHistorySize) {
        m_undoStack.erase(m_undoStack.begin());
    }

    return true;
}

bool TransactionManager::rollbackTransaction(kalara::architecture::Project& project) {
    if (!m_activeTransaction.has_value()) {
        return false;
    }

    Transaction tx = std::move(*m_activeTransaction);
    m_activeTransaction.reset();

    tx.status = TransactionStatus::RolledBack;
    tx.isSuccessful = false;

    // Restore beforeState
    return applyState(project, tx.beforeState);
}

bool TransactionManager::canUndo() const noexcept {
    return !m_undoStack.empty() && !m_activeTransaction.has_value();
}

bool TransactionManager::canRedo() const noexcept {
    return !m_redoStack.empty() && !m_activeTransaction.has_value();
}

bool TransactionManager::applyState(kalara::architecture::Project& target, const kalara::core::JsonObject& state) {
    auto restored = kalara::architecture::ProjectSerializer::deserializeProject(state);
    if (!restored) {
        return false;
    }

    target.id = restored->id;
    target.name = restored->name;
    target.displayUnit = restored->displayUnit;
    target.metadata = restored->metadata;
    target.setSites(restored->releaseSites());

    return true;
}

bool TransactionManager::undo(kalara::architecture::Project& project) {
    if (!canUndo()) {
        return false;
    }

    Transaction tx = std::move(m_undoStack.back());
    m_undoStack.pop_back();

    if (!applyState(project, tx.beforeState)) {
        // State restoration failed, re-push to undo stack
        m_undoStack.push_back(std::move(tx));
        return false;
    }

    m_redoStack.push_back(std::move(tx));
    return true;
}

bool TransactionManager::redo(kalara::architecture::Project& project) {
    if (!canRedo()) {
        return false;
    }

    Transaction tx = std::move(m_redoStack.back());
    m_redoStack.pop_back();

    if (!applyState(project, tx.afterState)) {
        // State restoration failed, re-push to redo stack
        m_redoStack.push_back(std::move(tx));
        return false;
    }

    m_undoStack.push_back(std::move(tx));
    return true;
}

void TransactionManager::clear() noexcept {
    m_activeTransaction.reset();
    m_undoStack.clear();
    m_redoStack.clear();
}

std::string TransactionManager::nextUndoIntent() const {
    if (m_undoStack.empty()) return "";
    return m_undoStack.back().intent;
}

std::string TransactionManager::nextRedoIntent() const {
    if (m_redoStack.empty()) return "";
    return m_redoStack.back().intent;
}

} // namespace kalara::runtime
