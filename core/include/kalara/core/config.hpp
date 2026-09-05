#pragma once
#include <string>

namespace kalara::core {

struct Version {
    int major = 0;
    int minor = 1;
    int patch = 0;
    
    std::string toString() const;
};

struct Config {
    std::string appName = "KaLara Arch";
    Version version;
    std::string defaultUnits = "mm";
};

} // namespace kalara::core
