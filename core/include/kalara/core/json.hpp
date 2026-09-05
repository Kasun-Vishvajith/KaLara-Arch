#pragma once
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <string_view>
#include <optional>
#include <filesystem>
#include <cstdint>

namespace kalara::core {

class JsonValue;
using JsonObject = std::map<std::string, JsonValue>;
using JsonArray = std::vector<JsonValue>;

/// Robust, zero-dependency C++20 JSON value representation for core serialization (Rule 4).
class JsonValue {
public:
    using ValueType = std::variant<
        std::monostate, // Null
        bool,           // Bool
        int64_t,        // Int
        double,         // Double
        std::string,    // String
        JsonArray,      // Array
        JsonObject      // Object
    >;

    JsonValue() : m_value(std::monostate{}) {}
    JsonValue(std::nullptr_t) : m_value(std::monostate{}) {}
    JsonValue(bool b) : m_value(b) {}
    JsonValue(int i) : m_value(static_cast<int64_t>(i)) {}
    JsonValue(int64_t i) : m_value(i) {}
    JsonValue(size_t s) : m_value(static_cast<int64_t>(s)) {}
    JsonValue(double d) : m_value(d) {}
    JsonValue(const char* s) : m_value(std::string(s)) {}
    JsonValue(std::string s) : m_value(std::move(s)) {}
    JsonValue(std::string_view s) : m_value(std::string(s)) {}
    JsonValue(JsonArray arr) : m_value(std::move(arr)) {}
    JsonValue(JsonObject obj) : m_value(std::move(obj)) {}

    [[nodiscard]] bool isNull() const noexcept { return std::holds_alternative<std::monostate>(m_value); }
    [[nodiscard]] bool isBool() const noexcept { return std::holds_alternative<bool>(m_value); }
    [[nodiscard]] bool isInt() const noexcept { return std::holds_alternative<int64_t>(m_value); }
    [[nodiscard]] bool isDouble() const noexcept { return std::holds_alternative<double>(m_value); }
    [[nodiscard]] bool isNumber() const noexcept { return isInt() || isDouble(); }
    [[nodiscard]] bool isString() const noexcept { return std::holds_alternative<std::string>(m_value); }
    [[nodiscard]] bool isArray() const noexcept { return std::holds_alternative<JsonArray>(m_value); }
    [[nodiscard]] bool isObject() const noexcept { return std::holds_alternative<JsonObject>(m_value); }

    [[nodiscard]] bool asBool(bool defaultVal = false) const noexcept {
        return isBool() ? std::get<bool>(m_value) : defaultVal;
    }

    [[nodiscard]] int64_t asInt(int64_t defaultVal = 0) const noexcept {
        if (isInt()) return std::get<int64_t>(m_value);
        if (isDouble()) return static_cast<int64_t>(std::get<double>(m_value));
        return defaultVal;
    }

    [[nodiscard]] double asDouble(double defaultVal = 0.0) const noexcept {
        if (isDouble()) return std::get<double>(m_value);
        if (isInt()) return static_cast<double>(std::get<int64_t>(m_value));
        return defaultVal;
    }

    [[nodiscard]] const std::string& asString(const std::string& defaultVal = "") const noexcept {
        return isString() ? std::get<std::string>(m_value) : defaultVal;
    }

    [[nodiscard]] const JsonArray& asArray() const {
        static const JsonArray emptyArray;
        return isArray() ? std::get<JsonArray>(m_value) : emptyArray;
    }

    JsonArray& asArray() {
        if (!isArray()) m_value = JsonArray{};
        return std::get<JsonArray>(m_value);
    }

    [[nodiscard]] const JsonObject& asObject() const {
        static const JsonObject emptyObject;
        return isObject() ? std::get<JsonObject>(m_value) : emptyObject;
    }

    JsonObject& asObject() {
        if (!isObject()) m_value = JsonObject{};
        return std::get<JsonObject>(m_value);
    }

    [[nodiscard]] bool contains(const std::string& key) const noexcept {
        if (!isObject()) return false;
        return std::get<JsonObject>(m_value).find(key) != std::get<JsonObject>(m_value).end();
    }

    const JsonValue& operator[](const std::string& key) const {
        static const JsonValue nullVal;
        if (!isObject()) return nullVal;
        const auto& obj = std::get<JsonObject>(m_value);
        auto it = obj.find(key);
        return (it != obj.end()) ? it->second : nullVal;
    }

    JsonValue& operator[](const std::string& key) {
        return asObject()[key];
    }

    const JsonValue& operator[](size_t index) const {
        static const JsonValue nullVal;
        if (!isArray()) return nullVal;
        const auto& arr = std::get<JsonArray>(m_value);
        return (index < arr.size()) ? arr[index] : nullVal;
    }

    JsonValue& operator[](size_t index) {
        return asArray()[index];
    }

    const JsonValue& operator[](int index) const {
        return operator[](static_cast<size_t>(index));
    }

    JsonValue& operator[](int index) {
        return operator[](static_cast<size_t>(index));
    }

    void push_back(JsonValue val) {
        asArray().push_back(std::move(val));
    }

    /// Serialize to JSON string with specified indentation (0 for compact).
    [[nodiscard]] std::string dump(int indent = 2) const;

    /// Parse a JSON string into a JsonValue DOM.
    static std::optional<JsonValue> parse(std::string_view jsonStr, std::string* errorOut = nullptr);

    /// Helper to save directly to file on disk.
    static bool saveToFile(const JsonValue& root, const std::filesystem::path& path, int indent = 2);

    /// Helper to load and parse directly from file on disk.
    static std::optional<JsonValue> loadFromFile(const std::filesystem::path& path, std::string* errorOut = nullptr);

private:
    ValueType m_value;

    void dumpInternal(std::string& out, int indent, int currentIndent) const;
};

} // namespace kalara::core
