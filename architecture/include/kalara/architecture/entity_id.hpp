#pragma once
#include <string>
#include <string_view>
#include <cstdint>
#include <functional>

namespace kalara::architecture {

/// Strongly-typed stable entity ID (Rule 1 & Rule 2).
/// Format: <prefix>_<unique_hex>, e.g., "proj_1a2b3c", "site_9f8e7d", "bld_4a5b6c", "lvl_7e8f9a"
class EntityId {
public:
    EntityId() = default;
    explicit EntityId(std::string id) : m_id(std::move(id)) {}

    static EntityId generate(std::string_view prefix = "ent");

    [[nodiscard]] const std::string& string() const noexcept { return m_id; }
    [[nodiscard]] bool isValid() const noexcept { return !m_id.empty(); }

    bool operator==(const EntityId& other) const noexcept { return m_id == other.m_id; }
    bool operator!=(const EntityId& other) const noexcept { return m_id != other.m_id; }
    bool operator<(const EntityId& other) const noexcept { return m_id < other.m_id; }

private:
    std::string m_id;
};

} // namespace kalara::architecture

namespace std {
template <>
struct hash<kalara::architecture::EntityId> {
    std::size_t operator()(const kalara::architecture::EntityId& id) const noexcept {
        return std::hash<std::string>{}(id.string());
    }
};
} // namespace std
