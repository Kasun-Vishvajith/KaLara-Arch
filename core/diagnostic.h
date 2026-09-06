#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace kalara {
enum class Severity { info, warning, error };
struct Diagnostic {
    std::string code;
    Severity severity = Severity::error;
    std::string message;
    std::vector<std::uint64_t> entityIds;
    std::string fieldPath;
    std::string suggestedAction;
};
// Logging intentionally accepts only a diagnostic code, never document content.
class Logger {
public:
    using Sink = std::function<void(Severity, const std::string&)>;
    explicit Logger(Sink sink);
    void record(const Diagnostic& diagnostic) const;
private:
    Sink sink_;
};
}
