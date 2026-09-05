#pragma once
#include <string>

namespace kalara::runtime {

struct RuntimeContext {
    bool isInitialized = false;
    std::string status = "Ready";
};

} // namespace kalara::runtime
