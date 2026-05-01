#pragma once

#include <cstdint>

namespace apex::memory {

struct alignas(64) MarketTick {
    uint64_t timestamp;
    uint32_t symbol_id;
    uint32_t padding;
    uint64_t bid;
    uint64_t ask;
    uint64_t volume;
};

} // namespace apex::memory
