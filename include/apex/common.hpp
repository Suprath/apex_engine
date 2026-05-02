#pragma once

#include <cstdint>

namespace apex {

enum class ExecutionMode : uint8_t {
    BIT_SLICED = 0,
    SCALAR = 1
};

} // namespace apex
