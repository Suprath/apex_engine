#include <iostream>
#include <cstring>
#include <cstdint>
#include <vector>
#include <bitset>
#include "apex/core/registry.hpp"
#include "apex/core/types.hpp"
#include "apex/compute/bit_slicer.hpp"
#include "apex/compute/column_buffer.hpp"
#include "apex/jit/compiler.hpp"

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

    std::cout << "\n=== Module 3 & 4 Verification: Bit-Slicer + JIT Compiler ===\n\n";

    // Create a buffer of 64 test prices
    apex::compute::ColumnBuffer prices;
    prices.data[0] = 10000;
    prices.data[1] = 20000;
    prices.data[2] = 25000;
    prices.data[3] = 30000;
    prices.data[4] = 50000;
    for (int i = 5; i < 64; i++) {
        prices.data[i] = 15000 + (i * 1000);
    }

    std::cout << "✓ Created buffer of 64 prices\n";
    std::cout << "  Sample: [0]=10000, [1]=20000, [2]=25000, [3]=30000, [4]=50000, ...\n";

    // Step 1: Transpose via BitSlicer to get bit-planes
    apex::compute::BitSlicer slicer;
    apex::compute::ColumnBuffer bit_planes;
    slicer.slice(prices, bit_planes);
    std::cout << "✓ BitSlicer generated 64 bit-planes from prices\n";

    // VERTICAL AUDIT: Check Row 63 Transpose Correctness
    std::cout << "\n=== Vertical Audit: Row 63 (All-Bits Set) ===\n";
    apex::compute::ColumnBuffer prices_audit;
    for (int i = 0; i < 64; i++) {
        prices_audit.data[i] = 0;
    }
    prices_audit.data[63] = 0xFFFFFFFFFFFFFFFF;  // Set all bits in row 63
    slicer.slice(prices_audit, bit_planes);

    std::cout << "Set prices[63] = 0xFFFFFFFFFFFFFFFF (all bits set)\n";
    std::cout << "Verifying: Every bit_plane[0..63] should have bit 63 = 1\n\n";

    int audit_errors = 0;
    for (int bit = 0; bit < 64; bit++) {
        uint64_t bit63_value = (bit_planes.data[bit] >> 63) & 1;
        if (bit63_value != 1) {
            std::cout << "  ERROR: bit_planes[" << bit << "] bit 63 = " << bit63_value << " (expected 1)\n";
            audit_errors++;
        }
    }
    if (audit_errors == 0) {
        std::cout << "  ✓ Row 63 transpose: ALL BIT-PLANES CORRECT (64/64)\n\n";
    } else {
        std::cout << "  ✗ Row 63 transpose FAILED: " << audit_errors << " bit-planes missing row 63\n\n";
    }

    // VERTICAL AUDIT: Check Row 53 Transpose Correctness
    std::cout << "=== Vertical Audit: Row 53 (All-Bits Set) ===\n";
    for (int i = 0; i < 64; i++) {
        prices_audit.data[i] = 0;
    }
    prices_audit.data[53] = 0xFFFFFFFFFFFFFFFF;  // Set all bits in row 53
    slicer.slice(prices_audit, bit_planes);

    std::cout << "Set prices[53] = 0xFFFFFFFFFFFFFFFF (all bits set)\n";
    std::cout << "Verifying: Every bit_plane[0..63] should have bit 53 = 1\n\n";

    audit_errors = 0;
    for (int bit = 0; bit < 64; bit++) {
        uint64_t bit53_value = (bit_planes.data[bit] >> 53) & 1;
        if (bit53_value != 1) {
            std::cout << "  ERROR: bit_planes[" << bit << "] bit 53 = " << bit53_value << " (expected 1)\n";
            audit_errors++;
        }
    }
    if (audit_errors == 0) {
        std::cout << "  ✓ Row 53 transpose: ALL BIT-PLANES CORRECT (64/64)\n\n";
    } else {
        std::cout << "  ✗ Row 53 transpose FAILED: " << audit_errors << " bit-planes missing row 53\n\n";
    }

    // Restore original prices for further testing
    prices.data[0] = 10000;
    prices.data[1] = 20000;
    prices.data[2] = 25000;
    prices.data[3] = 30000;
    prices.data[4] = 50000;
    for (int i = 5; i < 64; i++) {
        prices.data[i] = 15000 + (i * 1000);
    }
    slicer.slice(prices, bit_planes);

    // DIAGNOSTIC: Verify bit-plane correctness (especially rows 53-63)
    std::cout << "\n=== Bit-Plane Diagnostic ===\n";
    std::cout << "Checking bit-planes for correctness:\n";
    int errors = 0;
    for (int row = 0; row < 64; row++) {
        if (row % 16 != 0 && row < 48) continue;  // Skip most middle rows, check 53-63
        if (row >= 48 && row < 53) continue;       // Skip 48-52 for brevity

        uint64_t price = prices.data[row];
        bool row_ok = true;
        for (int bit = 0; bit < 64; bit++) {
            uint64_t expected_bit = (price >> bit) & 1;
            uint64_t bit_plane_bit = (bit_planes.data[bit] >> row) & 1;
            if (expected_bit != bit_plane_bit) {
                if (row_ok) {
                    std::cout << "Row " << row << " (price=" << price << "): ";
                    row_ok = false;
                }
                std::cout << "bit" << bit << "=" << bit_plane_bit << "(expected " << expected_bit << ") ";
                errors++;
            }
        }
        if (row_ok) {
            std::cout << "Row " << row << " (price=" << price << "): ✓\n";
        } else {
            std::cout << "\n";
        }
    }
    if (errors == 0) std::cout << "All checked rows: ✓\n";
    else std::cout << "ERROR: " << errors << " bit mismatches detected!\n";
    std::cout << "\n";

    // Step 2: Compile a comparison kernel for threshold 25000
    apex::jit::JitCompiler compiler;
    auto gt_kernel = compiler.compile_comparison(25000);
    if (!gt_kernel) {
        std::cerr << "✗ Failed to compile JIT kernel\n";
        apex::shutdown_runtime();
        return 1;
    }
    std::cout << "✓ JIT compiled comparison kernel (threshold=25000)\n";

    // Step 3: Execute the kernel on the bit-planes
    uint64_t result_mask = gt_kernel(bit_planes.data);
    std::cout << "✓ Executed kernel, result mask (binary):\n";

    // Print in binary for verification
    std::cout << "  ";
    for (int i = 0; i < 64; i++) {
        if (i > 0 && i % 16 == 0) std::cout << "\n  ";
        std::cout << ((result_mask >> (63 - i)) & 1);
    }
    std::cout << "\n";

    // Verify results: rows with price > 25000 should have bit set
    // CORRECTED: Bit i of result_mask corresponds to row i (not bit 63-i)
    std::cout << "\n[VERIFICATION] Rows with price > 25000:\n";
    int count = 0;
    for (int i = 0; i < 64; i++) {
        if (prices.data[i] > 25000) {
            bool bit_set = (result_mask >> i) & 1;  // FIXED: Check bit i for row i
            std::cout << "  Row " << i << " (price=" << prices.data[i]
                      << ") -> bit_set=" << bit_set;
            if (bit_set) {
                std::cout << " ✓\n";
                count++;
            } else {
                std::cout << " ✗ MISMATCH\n";
            }
        }
    }
    std::cout << "✓ Total matches: " << count << "/55 rows with price > 25000\n";

    // COMPREHENSIVE VERIFICATION: Check all 64 rows
    std::cout << "\n=== COMPREHENSIVE VERIFICATION: All 64 Rows ===\n";
    int total_correct = 0;
    for (int i = 0; i < 64; i++) {
        bool bit_set = (result_mask >> i) & 1;
        bool expected = (prices.data[i] > 25000);
        if (bit_set == expected) {
            total_correct++;
        } else {
            std::cout << "  MISMATCH: Row " << i << " (price=" << prices.data[i]
                      << ") -> bit_set=" << bit_set << " (expected " << expected << ")\n";
        }
    }
    std::cout << "✓ FINAL RESULT: " << total_correct << "/64 rows correct\n";

    if (total_correct == 64) {
        std::cout << "\n🎉 MODULE 4 COMPLETE: JIT Comparison Kernel Verified (64/64)!\n";
    }

    apex::shutdown_runtime();
    return 0;
}
