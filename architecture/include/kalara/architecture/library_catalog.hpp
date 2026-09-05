#pragma once
#include "kalara/architecture/library_item.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <optional>

namespace kalara::architecture {

/// 2D Architectural Object Catalog (Step 11).
/// Provides built-in architectural templates and manages custom user-created objects.
class LibraryCatalog {
public:
    LibraryCatalog();

    /// Retrieve catalog item by ID.
    [[nodiscard]] const LibraryItem* findItem(const std::string& itemId) const noexcept;

    /// Retrieve all items belonging to a category.
    [[nodiscard]] std::vector<const LibraryItem*> getItemsByCategory(LibraryCategory category) const;

    /// Retrieve all items matching a query string (case-insensitive search in name and ID).
    [[nodiscard]] std::vector<const LibraryItem*> searchItems(const std::string& query) const;

    /// Register a user-created library object (Step 11).
    bool registerUserItem(LibraryItem item);

    /// Total count of available library items.
    [[nodiscard]] size_t itemCount() const noexcept { return m_items.size(); }

    /// Access all items.
    [[nodiscard]] const std::unordered_map<std::string, LibraryItem>& allItems() const noexcept {
        return m_items;
    }

private:
    void populateBuiltinLibrary();

    std::unordered_map<std::string, LibraryItem> m_items;
};

} // namespace kalara::architecture
