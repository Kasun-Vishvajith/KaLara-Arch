#include "kalara/architecture/entity_id.hpp"
#include <atomic>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace kalara::architecture {

static std::atomic<uint64_t> s_counter{1};

EntityId EntityId::generate(std::string_view prefix) {
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    uint64_t count = s_counter.fetch_add(1, std::memory_order_relaxed);

    std::ostringstream ss;
    ss << prefix << "_" << std::hex << (now & 0xFFFFFFFF) << std::setw(4) << std::setfill('0') << (count & 0xFFFF);
    return EntityId(ss.str());
}

} // namespace kalara::architecture
