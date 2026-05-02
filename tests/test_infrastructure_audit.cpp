#include "apex/engine.hpp"
#include "apex/compute/bit_slicer.hpp"
#include "apex/jit/ir.hpp"
#include "apex/core/types.hpp"
#include <chrono>
#include <random>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstring>

namespace {
    struct alignas(64) MarketData {
        uint64_t ask;
        uint64_t bid;
        uint64_t volume;
        uint64_t spread;
        uint64_t timestamp;
    };

    void print_header(const std::string& title) {
        std::cout << "\n";
        std::cout << "=" << std::string(title.length() + 2, '=') << "=\n";
        std::cout << "| " << title << " |\n";
        std::cout << "=" << std::string(title.length() + 2, '=') << "=\n";
    }

    void print_result(const std::string& metric, double value, const std::string& unit,
                     double threshold, bool pass) {
        std::cout << "  " << std::left << std::setw(30) << metric << ": ";
        std::cout << std::right << std::setw(12) << std::fixed << std::setprecision(2) << value << " " << unit;
        if (!unit.empty()) {
            std::cout << " [target: <" << threshold << "]";
        }
        std::cout << " " << (pass ? "✓ PASS" : "✗ FAIL") << "\n";
    }
}

int main() {
    int total_tests = 0, passed_tests = 0;

    std::cout << "\n╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║           APEX INFRASTRUCTURE AUDIT                             ║\n";
    std::cout << "║     System Readiness Verification Before JIT Implementation      ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n";

    // =====================================================================
    // TEST 1: GATHER-SPEED BENCHMARK
    // =====================================================================
    {
        print_header("Test 1: Gather-Speed Benchmark");
        total_tests++;

        apex::ApexEngine engine;

        std::vector<apex::core::FieldDescriptor> fields = {
            {"ask", static_cast<uint32_t>(offsetof(MarketData, ask)), 64, apex::core::DataType::UINT64},
            {"bid", static_cast<uint32_t>(offsetof(MarketData, bid)), 64, apex::core::DataType::UINT64},
            {"volume", static_cast<uint32_t>(offsetof(MarketData, volume)), 64, apex::core::DataType::UINT64},
            {"spread", static_cast<uint32_t>(offsetof(MarketData, spread)), 64, apex::core::DataType::UINT64},
            {"timestamp", static_cast<uint32_t>(offsetof(MarketData, timestamp)), 64, apex::core::DataType::UINT64},
        };

        engine.register_schema("market", fields, sizeof(MarketData));

        // Generate 64 rows of test data
        std::vector<MarketData> rows(64);
        for (int i = 0; i < 64; i++) {
            rows[i].ask = 10000 + i * 2;
            rows[i].bid = 9990 + i * 2;
            rows[i].volume = 100000 + i * 1000;
            rows[i].spread = 10 + i;
            rows[i].timestamp = i * 100;
        }

        std::cout << "  Test setup:\n";
        std::cout << "    - Gathering 5 fields from 64 rows\n";
        std::cout << "    - 1000 iterations per field\n";
        std::cout << "    - Total: 5000 gather operations\n\n";

        // Warm up
        apex::compute::ColumnBuffer buffer;
        for (int i = 0; i < 5; i++) {
            engine.gather_field(rows.data(), &fields[i], sizeof(MarketData), 64, buffer);
        }

        // Benchmark
        auto start = std::chrono::high_resolution_clock::now();
        for (int iter = 0; iter < 1000; iter++) {
            for (int field_idx = 0; field_idx < 5; field_idx++) {
                engine.gather_field(rows.data(), &fields[field_idx], sizeof(MarketData), 64, buffer);
            }
        }
        auto end = std::chrono::high_resolution_clock::now();

        double total_ns = std::chrono::duration<double, std::nano>(end - start).count();
        double per_gather_ns = total_ns / 5000.0;
        bool gather_pass = per_gather_ns < 100.0;

        std::cout << "  Results:\n";
        std::cout << "    Total time: " << std::fixed << std::setprecision(3) << total_ns << " ns\n";
        print_result("Per-gather latency", per_gather_ns, "ns", 100.0, gather_pass);

        if (gather_pass) {
            passed_tests++;
            std::cout << "\n  ✓ Gather performance is excellent (< 100ns)\n";
        } else {
            std::cout << "\n  ⚠ Gather may need prefetching optimization\n";
        }
    }

    // =====================================================================
    // TEST 2: SCRATCHPAD ALIGNMENT CHECK
    // =====================================================================
    {
        print_header("Test 2: Scratchpad Alignment Check");
        total_tests++;

        // Create thread-local scratchpad with 64-byte alignment
        struct alignas(64) ScratchpadBuffer {
            uint64_t data[8 * 64];  // 8 slots × 64 bit-planes
        };
        static thread_local ScratchpadBuffer scratchpad;

        uintptr_t addr = reinterpret_cast<uintptr_t>(&scratchpad);
        int alignment = addr % 64;
        bool aligned = (alignment == 0);

        std::cout << "  Scratchpad layout:\n";
        std::cout << "    Size: 8 slots × 64 bit-planes × 8 bytes = " << (8 * 64 * 8) << " bytes\n";
        std::cout << "    Alignment: alignas(64)\n\n";

        std::cout << "  Memory layout:\n";
        std::cout << std::hex << std::uppercase;
        print_result("Address", static_cast<double>(addr), "0x", 0.0, true);
        std::cout << std::dec << std::nouppercase;
        print_result("Alignment offset", static_cast<double>(alignment), "bytes", 0.0, aligned);

        if (aligned) {
            passed_tests++;
            std::cout << "\n  ✓ Scratchpad is properly 64-byte aligned\n";
        } else {
            std::cout << "\n  ✗ Scratchpad is misaligned - potential 2x performance penalty\n";
        }
    }

    // =====================================================================
    // TEST 3: ABI LOOPBACK TEST (Identity Kernel)
    // =====================================================================
    {
        print_header("Test 3: ABI Loopback Test");
        total_tests++;

        std::cout << "  Test setup:\n";
        std::cout << "    Creating identity kernel via compile_expression()\n";
        std::cout << "    Kernel behavior: load first pointer from field_planes\n";
        std::cout << "    Expected return: pointer value as uint64_t\n\n";

        apex::ApexEngine engine;

        std::vector<apex::core::FieldDescriptor> fields = {
            {"test_field", 0, 64, apex::core::DataType::UINT64},
        };
        engine.register_schema("test", fields, 8);

        // Create a simple test node (LOAD will be ignored in our stub, but needed for compilation)
        auto test_node = apex::builder::Const(0xDEADBEEF00000000ULL);

        std::cout << "  Compiling identity kernel...\n";
        auto kernel = engine.get_compiler().compile_expression(test_node, engine.get_registry(), "test");

        if (!kernel) {
            std::cout << "    ✗ FAIL: Kernel compilation failed\n";
        } else {
            std::cout << "    ✓ Kernel compiled successfully\n\n";

            // Create test field array and field_planes pointer array
            apex::compute::ColumnBuffer test_field;
            for (int i = 0; i < 64; i++) {
                test_field.data[i] = 0x123456789ABCDEF0ULL + i;
            }

            const uint64_t* field_planes[] = {test_field.data};
            uint64_t scratchpad[512] = {};

            std::cout << "  Executing kernel:\n";
            std::cout << "    field_planes[0] = " << std::hex << std::uppercase
                     << reinterpret_cast<uintptr_t>(test_field.data) << std::dec << "\n";

            uint64_t result = kernel(field_planes, scratchpad);

            std::cout << "    Kernel returned: " << std::hex << std::uppercase << result << std::dec << "\n\n";

            // Verify result
            bool abi_pass = (result == reinterpret_cast<uint64_t>(test_field.data));

            if (abi_pass) {
                passed_tests++;
                std::cout << "  ✓ ABI loopback successful - calling convention verified\n";
            } else {
                std::cout << "  ⚠ ABI loopback returned unexpected value\n";
                std::cout << "    Expected: " << std::hex << std::uppercase
                         << reinterpret_cast<uint64_t>(test_field.data) << std::dec << "\n";
                std::cout << "    Got:      " << std::hex << std::uppercase << result << std::dec << "\n";
            }
        }
    }

    // =====================================================================
    // TEST 4: BIT-SLICER STRESS TEST
    // =====================================================================
    {
        print_header("Test 4: Bit-Slicer Stress Test");
        total_tests++;

        std::cout << "  Test setup:\n";
        std::cout << "    - Processing 1,000,000 blocks of 64 rows each\n";
        std::cout << "    - Each block: random uint64_t values\n";
        std::cout << "    - Verification: slice → reconstruct → compare\n\n";

        apex::compute::BitSlicer slicer;
        std::mt19937_64 rng(42);  // Fixed seed for reproducibility
        std::uniform_int_distribution<uint64_t> dist;

        const int BLOCKS = 1000000;
        int failures = 0;
        int failure_rows = 0;
        uint64_t first_failure_input = 0;
        uint64_t first_failure_reconstructed = 0;

        auto start = std::chrono::high_resolution_clock::now();

        for (int block = 0; block < BLOCKS; block++) {
            apex::compute::ColumnBuffer col, planes;

            // Fill with random data
            for (int i = 0; i < 64; i++) {
                col.data[i] = dist(rng);
            }

            // Slice
            slicer.slice(col, planes);

            // Reconstruct
            apex::compute::ColumnBuffer reconstructed;
            for (int i = 0; i < 64; i++) {
                reconstructed.data[i] = 0;
                for (int bit = 0; bit < 64; bit++) {
                    uint64_t bit_value = (planes.data[bit] >> i) & 1;
                    reconstructed.data[i] |= (bit_value << bit);
                }
            }

            // Verify
            for (int i = 0; i < 64; i++) {
                if (col.data[i] != reconstructed.data[i]) {
                    if (failures == 0) {
                        first_failure_input = col.data[i];
                        first_failure_reconstructed = reconstructed.data[i];
                    }
                    failures++;
                    failure_rows++;
                }
            }

            // Progress indicator
            if ((block + 1) % 200000 == 0) {
                std::cout << "  Progress: " << std::setw(7) << (block + 1) << " / "
                         << BLOCKS << " blocks processed\n";
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        double elapsed_sec = std::chrono::duration<double>(end - start).count();
        double blocks_per_sec = BLOCKS / elapsed_sec;

        std::cout << "\n  Results:\n";
        std::cout << "    Total blocks: " << BLOCKS << "\n";
        std::cout << "    Total time: " << std::fixed << std::setprecision(3) << elapsed_sec << " seconds\n";
        std::cout << "    Throughput: " << std::fixed << std::setprecision(1) << blocks_per_sec << " blocks/sec\n";
        print_result("Failures", static_cast<double>(failures), "mismatches", 1.0, failures == 0);

        if (failures == 0) {
            passed_tests++;
            std::cout << "\n  ✓ 100% consistency verified across 64M sliced values\n";
        } else {
            std::cout << "\n  ✗ Data corruption detected!\n";
            std::cout << "    First failure:\n";
            std::cout << "      Input:         " << std::hex << std::uppercase << first_failure_input << std::dec << "\n";
            std::cout << "      Reconstructed: " << std::hex << std::uppercase << first_failure_reconstructed << std::dec << "\n";
        }
    }

    // =====================================================================
    // SUMMARY
    // =====================================================================
    {
        std::cout << "\n";
        std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║                      AUDIT SUMMARY                             ║\n";
        std::cout << "╚════════════════════════════════════════════════════════════════╝\n\n";

        std::cout << "  Tests passed: " << passed_tests << " / " << total_tests << "\n\n";

        if (passed_tests == total_tests) {
            std::cout << "  ✓ ALL TESTS PASSED - Infrastructure is ready for JIT implementation\n\n";
            return 0;
        } else {
            std::cout << "  ✗ SOME TESTS FAILED - Review failures above before proceeding\n\n";
            return 1;
        }
    }

    return 0;
}
