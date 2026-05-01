#include "apex/jit/circuit_library.hpp"

namespace apex {
namespace jit {

void CircuitLibrary::emit_gt_64(
    asmjit::a64::Assembler& a,
    const asmjit::a64::Gp& a_bit,
    const asmjit::a64::Gp& b_bit,
    asmjit::a64::Gp& gt_mask,
    asmjit::a64::Gp& eq_mask) noexcept {
    using namespace asmjit::a64;

    // Branchless bit-sliced GT comparison:
    // GT = GT | (EQ & A_bit & ~B_bit)
    // EQ = EQ & ~(A_bit ^ B_bit)

    // Use x8 and x9 as scratch registers (ARM64 ABI: not caller-saved)
    Gp temp = x8;
    Gp eq_and_temp = x9;

    // temp = A_bit & ~B_bit (using BIC: Bit Clear)
    a.bic(temp, a_bit, b_bit);

    // eq_and_temp = EQ & temp
    a.and_(eq_and_temp, eq_mask, temp);

    // GT_new = GT | (EQ & temp)
    a.orr(gt_mask, gt_mask, eq_and_temp);

    // temp = A_bit ^ B_bit (reuse temp register)
    a.eor(temp, a_bit, b_bit);

    // EQ_new = EQ & ~(A_bit ^ B_bit)
    a.bic(eq_mask, eq_mask, temp);
}

} // namespace jit
} // namespace apex
