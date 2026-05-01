#pragma once

#include "apex/compute/column_buffer.hpp"

namespace apex::compute {

class BitSlicer {
public:
    void slice(const ColumnBuffer& in, ColumnBuffer& out) noexcept;
};

} // namespace apex::compute
