#include "kalara/core/logging.hpp"
#include <iostream>

namespace kalara::core {

void Logger::log(LogLevel level, std::string_view message) {
    const char* prefix = "[INFO]";
    switch (level) {
        case LogLevel::Debug:   prefix = "[DEBUG]"; break;
        case LogLevel::Info:    prefix = "[INFO]"; break;
        case LogLevel::Warning: prefix = "[WARN]"; break;
        case LogLevel::Error:   prefix = "[ERROR]"; break;
    }
    std::cout << prefix << " " << message << std::endl;
}

void Logger::info(std::string_view message) {
    log(LogLevel::Info, message);
}

void Logger::warning(std::string_view message) {
    log(LogLevel::Warning, message);
}

void Logger::error(std::string_view message) {
    log(LogLevel::Error, message);
}

} // namespace kalara::core
