#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <optional>

namespace kalara::architecture {

using MetadataValue = std::variant<std::string, double, int64_t, bool>;

/// Extensible structured metadata container for architectural entities.
class Metadata {
public:
    void set(std::string key, MetadataValue value) {
        m_properties[std::move(key)] = std::move(value);
    }

    [[nodiscard]] bool has(const std::string& key) const noexcept {
        return m_properties.find(key) != m_properties.end();
    }

    [[nodiscard]] std::optional<MetadataValue> get(const std::string& key) const {
        auto it = m_properties.find(key);
        if (it != m_properties.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    template <typename T>
    [[nodiscard]] std::optional<T> getAs(const std::string& key) const {
        auto it = m_properties.find(key);
        if (it != m_properties.end() && std::holds_alternative<T>(it->second)) {
            return std::get<T>(it->second);
        }
        return std::nullopt;
    }

    void remove(const std::string& key) {
        m_properties.erase(key);
    }

    void clear() noexcept {
        m_properties.clear();
    }

    [[nodiscard]] const std::unordered_map<std::string, MetadataValue>& all() const noexcept {
        return m_properties;
    }

private:
    std::unordered_map<std::string, MetadataValue> m_properties;
};

} // namespace kalara::architecture
