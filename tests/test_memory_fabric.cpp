#include <iostream>
#include <cassert>

#ifdef APEX_HAS_ICEORYX
#include "apex/memory/market_tick.hpp"
#include "apex/memory/data_viewer.hpp"
#include "apex/core/registry.hpp"
#include "apex/core/types.hpp"

using namespace apex::memory;
using namespace apex::core;
#endif

int main() {
    std::cout << "=== Module 2 Test: Memory Fabric ===\n\n";

#ifdef APEX_HAS_ICEORYX
    // Setup SchemaRegistry
    SchemaRegistry registry;
    std::vector<FieldDescriptor> market_tick_fields{
        {"timestamp", 0, 64, DataType::UINT64},
        {"symbol_id", 8, 32, DataType::UINT32},
        {"padding", 12, 32, DataType::UINT32},
        {"bid", 16, 64, DataType::UINT64},
        {"ask", 24, 64, DataType::UINT64},
        {"volume", 32, 64, DataType::UINT64},
    };
    registry.register_schema("MarketTick", market_tick_fields);
    std::cout << "✓ MarketTick schema registered\n";

    // Create a MarketTick struct (stack-allocated for fallback testing)
    // This simulates what the shared memory would contain
    alignas(64) MarketTick tick;
    tick.timestamp = 1000000;
    tick.symbol_id = 42;
    tick.padding = 0;
    tick.bid = 25000;
    tick.ask = 25100;
    tick.volume = 5000;

    const void* data_addr = &tick;
    std::cout << "✓ Created MarketTick struct at addr " << data_addr << "\n";

    // Test DataViewer: extract fields from buffer using schema registry
    DataViewer viewer(data_addr, registry);

    uint64_t bid = viewer.get_value<uint64_t>("MarketTick", "bid");
    assert(bid == 25000 && "Bid value mismatch");
    std::cout << "✓ DataViewer extracted bid: " << bid << "\n";

    uint64_t ask = viewer.get_value<uint64_t>("MarketTick", "ask");
    assert(ask == 25100 && "Ask value mismatch");
    std::cout << "✓ DataViewer extracted ask: " << ask << "\n";

    uint64_t timestamp = viewer.get_value<uint64_t>("MarketTick", "timestamp");
    assert(timestamp == 1000000 && "Timestamp mismatch");
    std::cout << "✓ DataViewer extracted timestamp: " << timestamp << "\n";

    uint32_t symbol_id = viewer.get_value<uint32_t>("MarketTick", "symbol_id");
    assert(symbol_id == 42 && "Symbol ID mismatch");
    std::cout << "✓ DataViewer extracted symbol_id: " << symbol_id << "\n";

    std::cout << "\n✓ Memory fabric core functionality verified!\n";
    std::cout << "[Note: Full zero-copy IPC test with ShmFabric and Publisher/Subscriber\n";
    std::cout << " requires POSIX ACL support - see DEVELOPMENT_GUIDE.md for native testing]\n";
    return 0;

#else
    std::cout << "APEX_HAS_ICEORYX not defined; memory fabric tests skipped\n";
    return 1;
#endif
}
