#include "core/diagnostic.h"
#include <utility>
namespace kalara {
Logger::Logger(Sink sink) : sink_(std::move(sink)) {}
void Logger::record(const Diagnostic& diagnostic) const {
    if (sink_) sink_(diagnostic.severity, diagnostic.code);
}
}
