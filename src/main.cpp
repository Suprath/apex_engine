#include <iostream>
#include <cstring>
#include <cstdint>
#include <vector>
#include "apex/core/registry.hpp"
#include "apex/core/types.hpp"

#ifdef APEX_HAS_ICEORYX
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "apex/memory/shm_fabric.hpp"
#include "apex/memory/data_viewer.hpp"
#include "apex/memory/market_tick.hpp"
#endif

namespace apex {

bool initialize_runtime() {
#ifdef APEX_HAS_ICEORYX
    iox::runtime::PoshRuntime::initRuntime("APEX_ENGINE");
    return true;
#else
    return true;
#endif
}

void shutdown_runtime() {
#ifdef APEX_HAS_ICEORYX
    // PoshRuntime cleanup happens on process exit
#endif
}

} // namespace apex

int main() {
    if (!apex::initialize_runtime()) {
        std::cerr << "Failed to initialize Apex runtime\n";
        return 1;
    }

    std::cout << "Apex Engine Initialized [M3 Optimized]\n";
    std::cout << "\n=== Module 1 Verification: Universal Metadata Registry ===\n\n";

    // Initialize SchemaRegistry
    apex::core::SchemaRegistry registry;

    // Register NSE_TICK schema with proper 8-byte alignment for 64-bit fields
    std::vector<apex::core::FieldDescriptor> nse_tick_fields{
        {"timestamp", 0, 64, apex::core::DataType::UINT64},
        {"symbol_id", 8, 32, apex::core::DataType::UINT32},
        {"padding", 12, 32, apex::core::DataType::UINT32},
        {"bid", 16, 64, apex::core::DataType::UINT64},
        {"ask", 24, 64, apex::core::DataType::UINT64},
        {"volume", 32, 64, apex::core::DataType::UINT64},
    };

    registry.register_schema("NSE_TICK", nse_tick_fields);
    std::cout << "✓ Registered schema: NSE_TICK (6 fields, aligned)\n";

    // Diagnostic loop: print field offsets
    std::cout << "\nField Layout (ARM64 Aligned):\n";
    const char* field_names[] = {"timestamp", "symbol_id", "padding", "bid", "ask", "volume"};
    for (const auto* field_name : field_names) {
        const auto* field = registry.get_field("NSE_TICK", field_name);
        if (field) {
            std::cout << "  " << field->name << " -> offset: " << field->offset
                      << ", bit_width: " << field->bit_width << "\n";
        }
    }

    // ULL Test: Live buffer write/read
    std::cout << "\n=== ULL Test: Buffer Write/Read ===\n";
    uint8_t tick_buffer[64] = {0};  // 64-byte buffer for a tick

    // Get bid field and write value
    const auto* bid_field = registry.get_field("NSE_TICK", "bid");
    if (bid_field) {
        uint64_t bid_value = 25000;
        std::memcpy(&tick_buffer[bid_field->offset], &bid_value, sizeof(uint64_t));
        std::cout << "✓ Wrote bid value: " << bid_value << " at offset " << bid_field->offset << "\n";

        // Read value back
        uint64_t bid_read = 0;
        std::memcpy(&bid_read, &tick_buffer[bid_field->offset], sizeof(uint64_t));
        std::cout << "[VERIFICATION] bid offset: " << bid_field->offset
                  << " | bid value read: " << bid_read << "\n";
    }

    apex::shutdown_runtime();
    return 0;
}
