#include "apex/jit/compiler.hpp"
#include <iostream>
#include <iomanip>

namespace apex {
namespace jit {

JitCompiler::JitCompiler() noexcept
    : runtime_(std::make_unique<asmjit::JitRuntime>()) {}

JitCompiler::~JitCompiler() noexcept = default;

void JitCompiler::dump_bytecode(const asmjit::CodeHolder& code, const char* label) const noexcept {
    std::cout << "\n=== " << label << " ===\n";

    for (auto& section : code.sections()) {
        const uint8_t* data = section->buffer().data();
        size_t size = section->buffer().size();

        std::cout << "Section size: " << size << " bytes\n";
        std::cout << "Hex dump (first 2048 bytes):\n";

        size_t dump_size = (size > 2048) ? 2048 : size;
        for (size_t i = 0; i < dump_size; i++) {
            if (i % 16 == 0) {
                std::cout << "\n  " << std::hex << std::setw(4) << std::setfill('0') << i << ": ";
            }
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)data[i] << " ";
        }
        std::cout << std::dec << "\n";
    }
}

KernelFunc JitCompiler::compile_comparison(uint64_t threshold) noexcept {
    using namespace asmjit;
    using namespace asmjit::a64;

    CodeHolder code;
    code.init(runtime_->environment());

    Assembler a(&code);

    // ARM64 ABI register allocation
    // x0 = bit_planes_ptr (input, read-only)
    // x9 = GT_mask (accumulator)
    // x10 = EQ_mask (accumulator)
    // x11 = pointer_walker (post-index pointer)
    // x12 = a_bitplane (loaded value)
    // x13 = tmp (scratch)
    // x14, x15 = additional temps if needed

    Gp bit_planes_ptr = x0;
    Gp gt_mask = x9;
    Gp eq_mask = x10;
    Gp pointer_walker = x11;
    Gp a_bitplane = x12;
    Gp tmp = x13;

    // Initialize state: GT = 0, EQ = all 1s
    a.orr(gt_mask, xzr, xzr);  // GT = 0 (explicit XOR with zero register)
    a.mov(eq_mask, -1);         // EQ = all 1s (load immediate -1)

    // Setup pointer walker to bit_planes[63]
    a.add(pointer_walker, bit_planes_ptr, 504);  // 504 = 63 * 8 bytes

    // Generate 64 unrolled comparison blocks (bit 63 down to 0, MSB to LSB)
    std::cout << "DEBUG: threshold = " << threshold << " (0x" << std::hex << threshold << std::dec << ")\n";
    std::cout << "Threshold bits: ";
    for (int bit = 63; bit >= 0; --bit) {
        uint64_t threshold_bit = (threshold >> bit) & 1;
        if (bit % 8 == 0 && bit < 63) std::cout << " ";
        std::cout << threshold_bit;
    }
    std::cout << "\n\n";

    int branch0_count = 0, branch1_count = 0;

    for (int bit = 63; bit >= 0; --bit) {
        uint64_t threshold_bit = (threshold >> bit) & 1;

        // Load bit-plane[bit] using post-index addressing
        // a.ldr loads from [pointer_walker], then decrements pointer_walker by 8
        a.ldr(a_bitplane, Mem(pointer_walker).post(-8));

        if (threshold_bit == 0) {
            branch0_count++;
            // When threshold_bit is 0:
            // GT = GT | (EQ & A_bit & ~0) = GT | (EQ & A_bit)
            // EQ = EQ & ~(A_bit ^ 0) = EQ & ~A_bit

            // tmp = EQ & A_bit
            a.and_(tmp, eq_mask, a_bitplane);
            // GT |= tmp
            a.orr(gt_mask, gt_mask, tmp);
            // EQ &= ~A_bit (using BIC: Bit Clear)
            a.bic(eq_mask, eq_mask, a_bitplane);
        } else {
            branch1_count++;
            // When threshold_bit is 1:
            // GT = GT | (EQ & A_bit & ~1) = GT | 0 = GT (unchanged)
            // EQ = EQ & ~(A_bit ^ 1) = EQ & A_bit

            // EQ &= A_bit
            a.and_(eq_mask, eq_mask, a_bitplane);
        }
    }

    std::cout << "Branch 0 (threshold_bit==0) count: " << branch0_count << "\n";
    std::cout << "Branch 1 (threshold_bit==1) count: " << branch1_count << "\n\n";

    // Return GT_mask in x0
    a.mov(x0, gt_mask);
    a.ret(x30);

    // Dump bytecode for inspection
    dump_bytecode(code, "JIT Comparison Kernel Bytecode");

    // Add code to JIT runtime
    KernelFunc fn = nullptr;
    Error err = runtime_->add(&fn, &code);
    if (err != kErrorOk) {
        return nullptr;
    }

    return fn;
}

} // namespace jit
} // namespace apex
