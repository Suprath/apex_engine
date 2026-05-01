#include "apex/engine.hpp"
#include "apex/jit/ir.hpp"
#include <iostream>
#include <iomanip>
#include <cstdint>

// Test market data structure
struct MarketTick {
    uint64_t ask;
    uint64_t bid;
    uint64_t spread;
    uint64_t prev_spread;
    uint64_t volume;
};

// Scalar reference implementation
bool arbitrage_signal_reference(const MarketTick& tick) {
    int64_t diff = static_cast<int64_t>(tick.ask) - static_cast<int64_t>(tick.bid);
    return (diff < 5) && (tick.spread > tick.prev_spread) && (tick.volume > 1000);
}

int main() {
    using namespace apex;

    // Initialize engine
    ApexEngine engine;

    // Register schema for MarketTick
    std::vector<core::FieldDescriptor> fields = {
        {"ask", offsetof(MarketTick, ask), 64, core::DataType::UINT64},
        {"bid", offsetof(MarketTick, bid), 64, core::DataType::UINT64},
        {"spread", offsetof(MarketTick, spread), 64, core::DataType::UINT64},
        {"prev_spread", offsetof(MarketTick, prev_spread), 64, core::DataType::UINT64},
        {"volume", offsetof(MarketTick, volume), 64, core::DataType::UINT64},
    };

    engine.register_schema("market", fields, sizeof(MarketTick));

    // Build expression: (Ask - Bid) < 5 AND Spread > Prev_Spread AND Volume > 1000
    auto* expr_root = builder::And(
        builder::And(
            builder::LT(
                builder::Sub(
                    builder::Load("ask"),
                    builder::Load("bid")
                ),
                builder::Const(5)
            ),
            builder::GT(
                builder::Load("spread"),
                builder::Load("prev_spread")
            )
        ),
        builder::GT(
            builder::Load("volume"),
            builder::Const(1000)
        )
    );

    engine.set_expression("market", expr_root);

    // Generate 64 test rows (one chunk)
    MarketTick test_rows[64];
    int expected_matches = 0;

    for (int i = 0; i < 64; ++i) {
        test_rows[i].ask = 10000 + i * 10;
        test_rows[i].bid = 9990 + i * 10;
        test_rows[i].spread = 100 + i;
        test_rows[i].prev_spread = 90 + i;
        test_rows[i].volume = 1500 + i * 100;

        if (arbitrage_signal_reference(test_rows[i])) {
            expected_matches++;
        }
    }

    // Execute via JIT
    uint64_t jit_matches = engine.execute(test_rows, 64);

    std::cout << "\n=== Universal Expression Engine Test (Arbitrage Signal) ===\n";
    std::cout << "Expected matches: " << expected_matches << "\n";
    std::cout << "JIT matches:      " << jit_matches << "\n";

    // Verify
    if (jit_matches == expected_matches) {
        std::cout << "✓ TEST PASSED: JIT and scalar reference match!\n";
        return 0;
    } else {
        std::cout << "✗ TEST FAILED: Mismatch!\n";
        std::cout << "\nDetailed comparison:\n";
        for (int i = 0; i < 64; ++i) {
            bool expected = arbitrage_signal_reference(test_rows[i]);
            std::cout << "Row " << std::setw(2) << i << ": ";
            std::cout << "ask=" << std::setw(5) << test_rows[i].ask << " ";
            std::cout << "bid=" << std::setw(5) << test_rows[i].bid << " ";
            std::cout << "diff=" << std::setw(4) << (int64_t(test_rows[i].ask) - int64_t(test_rows[i].bid)) << " ";
            std::cout << "spread=" << std::setw(3) << test_rows[i].spread << " ";
            std::cout << "prev_spread=" << std::setw(3) << test_rows[i].prev_spread << " ";
            std::cout << "volume=" << std::setw(4) << test_rows[i].volume << " ";
            std::cout << "expected=" << (expected ? "Y" : "N") << "\n";
        }
        return 1;
    }
}
