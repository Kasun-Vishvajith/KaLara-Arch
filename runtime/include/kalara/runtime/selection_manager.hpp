#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/level.hpp"
#include "kalara/core/geometry/rect.hpp"
#include "kalara/core/geometry/ops.hpp"
#include <unordered_set>
#include <vector>
#include <optional>

namespace kalara::runtime {

/// Semantic description of selection content.
struct SelectionSummary {
    size_t totalCount = 0;
    size_t wallCount = 0;
    size_t roomCount = 0;
    size_t doorCount = 0;
    size_t windowCount = 0;
    size_t dimensionCount = 0;
    size_t noteCount = 0;
    size_t libraryInstanceCount = 0;
    bool hasSite = false;
    std::optional<kalara::core::geometry::Rect2D> boundingBox;
};

/// Manages selection state of entities using stable IDs (Rule 1).
/// Supports single, toggle, additive, and rectangular marquee selection.
class SelectionManager {
public:
    void select(const kalara::architecture::EntityId& id) {
        m_selected.insert(id);
    }

    void deselect(const kalara::architecture::EntityId& id) {
        m_selected.erase(id);
    }

    void toggle(const kalara::architecture::EntityId& id) {
        if (isSelected(id)) {
            deselect(id);
        } else {
            select(id);
        }
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

    [[nodiscard]] bool empty() const noexcept {
        return m_selected.empty();
    }

    [[nodiscard]] const std::unordered_set<kalara::architecture::EntityId>& selectedIds() const noexcept {
        return m_selected;
    }

    [[nodiscard]] std::vector<kalara::architecture::EntityId> selectedList() const {
        return std::vector<kalara::architecture::EntityId>(m_selected.begin(), m_selected.end());
    }

    [[nodiscard]] std::optional<kalara::architecture::EntityId> primarySelected() const noexcept {
        if (m_selected.empty()) return std::nullopt;
        return *m_selected.begin();
    }

    /// Select all entities intersecting or contained inside a world bounding box (Marquee selection).
    size_t selectInRect(const kalara::core::geometry::Rect2D& rect,
                        const kalara::architecture::Level& level,
                        bool additive = false) {
        if (!additive) {
            clear();
        }

        size_t added = 0;

        // Check walls
        for (const auto& w : level.walls()) {
            if (rect.contains(w->start) || rect.contains(w->end) ||
                rect.contains(w->centerline().midpoint())) {
                if (m_selected.insert(w->id).second) {
                    ++added;
                }
            }
        }

        // Check doors
        for (const auto& d : level.doors()) {
            auto* host = level.findWall(d->hostWallId);
            if (host && rect.contains(d->centerPoint(*host))) {
                if (m_selected.insert(d->id).second) {
                    ++added;
                }
            }
        }

        // Check windows
        for (const auto& win : level.windows()) {
            auto* host = level.findWall(win->hostWallId);
            if (host && rect.contains(win->centerPoint(*host))) {
                if (m_selected.insert(win->id).second) {
                    ++added;
                }
            }
        }

        // Check rooms
        for (const auto& r : level.rooms()) {
            bool anyInside = false;
            for (const auto& pt : r->boundary) {
                if (rect.contains(pt)) {
                    anyInside = true;
                    break;
                }
            }
            if (anyInside || rect.contains(r->labelPosition())) {
                if (m_selected.insert(r->id).second) {
                    ++added;
                }
            }
        }

        // Check dimensions
        for (const auto& dim : level.dimensions()) {
            if (rect.contains(dim->point1) || rect.contains(dim->point2)) {
                if (m_selected.insert(dim->id).second) {
                    ++added;
                }
            }
        }

        // Check notes
        for (const auto& note : level.notes()) {
            if (rect.contains(note->position)) {
                if (m_selected.insert(note->id).second) {
                    ++added;
                }
            }
        }

        // Check library instances (Step 11)
        for (const auto& inst : level.libraryInstances()) {
            if (rect.contains(inst->position) || rect.intersects(inst->boundingBox())) {
                if (m_selected.insert(inst->id).second) {
                    ++added;
                }
            }
        }

        return added;
    }

    /// Compute semantic summary and combined bounding box of current selection on level.
    [[nodiscard]] SelectionSummary summarize(const kalara::architecture::Level& level) const {
        SelectionSummary summary;
        summary.totalCount = m_selected.size();

        std::vector<kalara::core::geometry::Point2D> pts;

        for (const auto& id : m_selected) {
            if (auto* w = level.findWall(id)) {
                ++summary.wallCount;
                pts.push_back(w->start);
                pts.push_back(w->end);
            } else if (auto* r = level.findRoom(id)) {
                ++summary.roomCount;
                for (const auto& p : r->boundary) pts.push_back(p);
            } else if (auto* inst = level.findLibraryInstance(id)) {
                ++summary.libraryInstanceCount;
                for (const auto& p : inst->worldBoundary()) pts.push_back(p);
            } else {
                for (const auto& d : level.doors()) {
                    if (d->id == id) {
                        ++summary.doorCount;
                        if (auto* h = level.findWall(d->hostWallId)) {
                            pts.push_back(d->centerPoint(*h));
                        }
                        break;
                    }
                }
                for (const auto& win : level.windows()) {
                    if (win->id == id) {
                        ++summary.windowCount;
                        if (auto* h = level.findWall(win->hostWallId)) {
                            pts.push_back(win->centerPoint(*h));
                        }
                        break;
                    }
                }
                for (const auto& dim : level.dimensions()) {
                    if (dim->id == id) {
                        ++summary.dimensionCount;
                        pts.push_back(dim->point1);
                        pts.push_back(dim->point2);
                        break;
                    }
                }
                for (const auto& note : level.notes()) {
                    if (note->id == id) {
                        ++summary.noteCount;
                        pts.push_back(note->position);
                        break;
                    }
                }
            }
        }

        if (!pts.empty()) {
            summary.boundingBox = kalara::core::geometry::GeometricOps::boundingBox(pts);
        }

        return summary;
    }

private:
    std::unordered_set<kalara::architecture::EntityId> m_selected;
};

} // namespace kalara::runtime
