#pragma once
#include "kalara/architecture/entity_id.hpp"
#include <unordered_set>
#include <vector>
#include <optional>

namespace kalara::runtime {

/// Manages selection state of entities using stable IDs (Rule 1).
class SelectionManager {
public:
    void select(const kalara::architecture::EntityId& id) {
        m_selected.insert(id);
    }

    void deselect(const kalara::architecture::EntityId& id) {
        m_selected.erase(id);
    }

    void clear() noexcept {
        m_selected.clear();
    }

    void setSelection(const std::vector<kalara::architecture::EntityId>& ids) {
        m_selected.clear();
        for (const auto& id : ids) {
            m_selected.insert(id);
        }
    }

    [[nodiscard]] bool isSelected(const kalara::architecture::EntityId& id) const noexcept {
        return m_selected.find(id) != m_selected.end();
    }

    [[nodiscard]] size_t count() const noexcept {
        return m_selected.size();
    }

    [[nodiscard]] const std::unordered_set<kalara::architecture::EntityId>& selectedIds() const noexcept {
        return m_selected;
    }

    [[nodiscard]] std::optional<kalara::architecture::EntityId> primarySelected() const noexcept {
        if (m_selected.empty()) return std::nullopt;
        return *m_selected.begin();
    }

private:
    std::unordered_set<kalara::architecture::EntityId> m_selected;
};

} // namespace kalara::runtime
