#include "kalara/core/json.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <cmath>
#include <charconv>

namespace kalara::core {

namespace {

void writeIndent(std::string& out, int spaces) {
    out.append(spaces, ' ');
}

void escapeString(const std::string& str, std::string& out) {
    out.push_back('"');
    for (char c : str) {
        switch (c) {
            case '"':  out.append("\\\""); break;
            case '\\': out.append("\\\\"); break;
            case '\b': out.append("\\b"); break;
            case '\f': out.append("\\f"); break;
            case '\n': out.append("\\n"); break;
            case '\r': out.append("\\r"); break;
            case '\t': out.append("\\t"); break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned int>(static_cast<unsigned char>(c)));
                    out.append(buf);
                } else {
                    out.push_back(c);
                }
                break;
        }
    }
    out.push_back('"');
}

class JsonParser {
public:
    explicit JsonParser(std::string_view src) : m_src(src), m_pos(0) {}

    std::optional<JsonValue> parse(std::string* errorOut) {
        skipWhitespace();
        if (m_pos >= m_src.size()) {
            if (errorOut) *errorOut = "Empty input string";
            return std::nullopt;
        }

        auto val = parseValue();
        if (!val.has_value()) {
            if (errorOut) *errorOut = m_error;
            return std::nullopt;
        }

        skipWhitespace();
        if (m_pos < m_src.size()) {
            if (errorOut) *errorOut = "Unexpected trailing characters at position " + std::to_string(m_pos);
            return std::nullopt;
        }

        return val;
    }

private:
    std::string_view m_src;
    size_t m_pos = 0;
    std::string m_error;

    void skipWhitespace() {
        while (m_pos < m_src.size()) {
            char c = m_src[m_pos];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                ++m_pos;
            } else {
                break;
            }
        }
    }

    char peek() const {
        return (m_pos < m_src.size()) ? m_src[m_pos] : '\0';
    }

    char get() {
        return (m_pos < m_src.size()) ? m_src[m_pos++] : '\0';
    }

    std::optional<JsonValue> parseValue() {
        skipWhitespace();
        if (m_pos >= m_src.size()) {
            m_error = "Unexpected end of input while expecting value";
            return std::nullopt;
        }

        char c = peek();
        if (c == 'n') return parseNull();
        if (c == 't' || c == 'f') return parseBool();
        if (c == '"') return parseString();
        if (c == '[') return parseArray();
        if (c == '{') return parseObject();
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parseNumber();

        m_error = std::string("Unexpected character '") + c + "' at position " + std::to_string(m_pos);
        return std::nullopt;
    }

    std::optional<JsonValue> parseNull() {
        if (m_src.substr(m_pos, 4) == "null") {
            m_pos += 4;
            return JsonValue(nullptr);
        }
        m_error = "Expected 'null'";
        return std::nullopt;
    }

    std::optional<JsonValue> parseBool() {
        if (m_src.substr(m_pos, 4) == "true") {
            m_pos += 4;
            return JsonValue(true);
        }
        if (m_src.substr(m_pos, 5) == "false") {
            m_pos += 5;
            return JsonValue(false);
        }
        m_error = "Expected 'true' or 'false'";
        return std::nullopt;
    }

    std::optional<JsonValue> parseString() {
        get(); // consume opening quote '"'
        std::string s;
        while (m_pos < m_src.size()) {
            char c = get();
            if (c == '"') {
                return JsonValue(std::move(s));
            }
            if (c == '\\') {
                if (m_pos >= m_src.size()) {
                    m_error = "Unexpected end of string escape";
                    return std::nullopt;
                }
                char esc = get();
                switch (esc) {
                    case '"':  s.push_back('"'); break;
                    case '\\': s.push_back('\\'); break;
                    case '/':  s.push_back('/'); break;
                    case 'b':  s.push_back('\b'); break;
                    case 'f':  s.push_back('\f'); break;
                    case 'n':  s.push_back('\n'); break;
                    case 'r':  s.push_back('\r'); break;
                    case 't':  s.push_back('\t'); break;
                    case 'u': {
                        if (m_pos + 4 > m_src.size()) {
                            m_error = "Incomplete unicode escape";
                            return std::nullopt;
                        }
                        // Simple 4-hex digit handling
                        unsigned int code = 0;
                        for (int i = 0; i < 4; ++i) {
                            char h = get();
                            code <<= 4;
                            if (h >= '0' && h <= '9') code |= (h - '0');
                            else if (h >= 'a' && h <= 'f') code |= (h - 'a' + 10);
                            else if (h >= 'A' && h <= 'F') code |= (h - 'A' + 10);
                            else {
                                m_error = "Invalid hex in unicode escape";
                                return std::nullopt;
                            }
                        }
                        if (code < 0x80) {
                            s.push_back(static_cast<char>(code));
                        } else if (code < 0x800) {
                            s.push_back(static_cast<char>(0xC0 | (code >> 6)));
                            s.push_back(static_cast<char>(0x80 | (code & 0x3F)));
                        } else {
                            s.push_back(static_cast<char>(0xE0 | (code >> 12)));
                            s.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
                            s.push_back(static_cast<char>(0x80 | (code & 0x3F)));
                        }
                        break;
                    }
                    default:
                        s.push_back(esc);
                        break;
                }
            } else {
                s.push_back(c);
            }
        }
        m_error = "Unterminated string literal";
        return std::nullopt;
    }

    std::optional<JsonValue> parseNumber() {
        size_t start = m_pos;
        bool isFloat = false;
        if (peek() == '-') ++m_pos;

        while (m_pos < m_src.size()) {
            char c = peek();
            if (std::isdigit(static_cast<unsigned char>(c))) {
                ++m_pos;
            } else if (c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-') {
                if (c == '.' || c == 'e' || c == 'E') isFloat = true;
                ++m_pos;
            } else {
                break;
            }
        }

        std::string_view numStr = m_src.substr(start, m_pos - start);
        std::string numCopy(numStr);
        if (isFloat) {
            try {
                double d = std::stod(numCopy);
                return JsonValue(d);
            } catch (...) {
                m_error = "Invalid floating point number: " + numCopy;
                return std::nullopt;
            }
        } else {
            try {
                int64_t i = std::stoll(numCopy);
                return JsonValue(i);
            } catch (...) {
                m_error = "Invalid integer: " + numCopy;
                return std::nullopt;
            }
        }
    }

    std::optional<JsonValue> parseArray() {
        get(); // consume '['
        JsonArray arr;
        skipWhitespace();

        if (peek() == ']') {
            get();
            return JsonValue(std::move(arr));
        }

        while (m_pos < m_src.size()) {
            auto elem = parseValue();
            if (!elem.has_value()) return std::nullopt;
            arr.push_back(std::move(*elem));

            skipWhitespace();
            char c = peek();
            if (c == ']') {
                get();
                return JsonValue(std::move(arr));
            } else if (c == ',') {
                get();
                skipWhitespace();
            } else {
                m_error = "Expected ',' or ']' inside array at pos " + std::to_string(m_pos);
                return std::nullopt;
            }
        }

        m_error = "Unterminated array literal";
        return std::nullopt;
    }

    std::optional<JsonValue> parseObject() {
        get(); // consume '{'
        JsonObject obj;
        skipWhitespace();

        if (peek() == '}') {
            get();
            return JsonValue(std::move(obj));
        }

        while (m_pos < m_src.size()) {
            skipWhitespace();
            if (peek() != '"') {
                m_error = "Expected string key in object at pos " + std::to_string(m_pos);
                return std::nullopt;
            }

            auto keyVal = parseString();
            if (!keyVal.has_value()) return std::nullopt;
            std::string key = keyVal->asString();

            skipWhitespace();
            if (peek() != ':') {
                m_error = "Expected ':' after key in object at pos " + std::to_string(m_pos);
                return std::nullopt;
            }
            get(); // consume ':'

            auto val = parseValue();
            if (!val.has_value()) return std::nullopt;
            obj[std::move(key)] = std::move(*val);

            skipWhitespace();
            char c = peek();
            if (c == '}') {
                get();
                return JsonValue(std::move(obj));
            } else if (c == ',') {
                get();
                skipWhitespace();
            } else {
                m_error = "Expected ',' or '}' inside object at pos " + std::to_string(m_pos);
                return std::nullopt;
            }
        }

        m_error = "Unterminated object literal";
        return std::nullopt;
    }
};

} // namespace

void JsonValue::dumpInternal(std::string& out, int indent, int currentIndent) const {
    if (isNull()) {
        out.append("null");
    } else if (isBool()) {
        out.append(asBool() ? "true" : "false");
    } else if (isInt()) {
        out.append(std::to_string(asInt()));
    } else if (isDouble()) {
        double d = asDouble();
        if (std::isnan(d) || std::isinf(d)) {
            out.append("0.0");
        } else {
            char buf[64];
            snprintf(buf, sizeof(buf), "%.6g", d);
            std::string s(buf);
            if (s.find('.') == std::string::npos && s.find('e') == std::string::npos) {
                s.append(".0");
            }
            out.append(s);
        }
    } else if (isString()) {
        escapeString(asString(), out);
    } else if (isArray()) {
        const auto& arr = asArray();
        if (arr.empty()) {
            out.append("[]");
            return;
        }

        out.push_back('[');
        if (indent > 0) out.push_back('\n');

        for (size_t i = 0; i < arr.size(); ++i) {
            if (indent > 0) writeIndent(out, currentIndent + indent);
            arr[i].dumpInternal(out, indent, currentIndent + indent);
            if (i + 1 < arr.size()) out.push_back(',');
            if (indent > 0) out.push_back('\n');
        }

        if (indent > 0) writeIndent(out, currentIndent);
        out.push_back(']');
    } else if (isObject()) {
        const auto& obj = asObject();
        if (obj.empty()) {
            out.append("{}");
            return;
        }

        out.push_back('{');
        if (indent > 0) out.push_back('\n');

        size_t count = 0;
        for (const auto& [k, v] : obj) {
            if (indent > 0) writeIndent(out, currentIndent + indent);
            escapeString(k, out);
            out.append((indent > 0) ? ": " : ":");
            v.dumpInternal(out, indent, currentIndent + indent);
            if (++count < obj.size()) out.push_back(',');
            if (indent > 0) out.push_back('\n');
        }

        if (indent > 0) writeIndent(out, currentIndent);
        out.push_back('}');
    }
}

std::string JsonValue::dump(int indent) const {
    std::string out;
    dumpInternal(out, indent, 0);
    return out;
}

std::optional<JsonValue> JsonValue::parse(std::string_view jsonStr, std::string* errorOut) {
    JsonParser parser(jsonStr);
    return parser.parse(errorOut);
}

bool JsonValue::saveToFile(const JsonValue& root, const std::filesystem::path& path, int indent) {
    std::ofstream file(path, std::ios::out | std::ios::trunc);
    if (!file.is_open()) return false;
    file << root.dump(indent);
    return file.good();
}

std::optional<JsonValue> JsonValue::loadFromFile(const std::filesystem::path& path, std::string* errorOut) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        if (errorOut) *errorOut = "Failed to open file: " + path.string();
        return std::nullopt;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();
    return parse(content, errorOut);
}

} // namespace kalara::core
