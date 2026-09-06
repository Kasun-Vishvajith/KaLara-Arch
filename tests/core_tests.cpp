#include "core/diagnostic.h"
#include <iostream>
int main() {
    std::string logged;
    kalara::Logger logger([&](kalara::Severity, const std::string& code) { logged = code; });
    logger.record({"invalid.geometry", kalara::Severity::error, "Private project label", {42}, "wall.length", "Enter a positive length"});
    if (logged != "invalid.geometry") { std::cerr << "Logger leaked message or lost code\n"; return 1; }
    return 0;
}
