#include "kalara/core/config.hpp"
#include <sstream>

namespace kalara::core {

std::string Version::toString() const {
    std::ostringstream ss;
    ss << major << "." << minor << "." << patch;
    return ss.str();
}

} // namespace kalara::core
