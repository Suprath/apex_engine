#pragma once

#include <asmjit/a64.h>
#include <cstdint>
#include <memory>

namespace apex {
namespace jit {

using KernelFunc = uint64_t (*)(const uint64_t* bit_planes);

class JitCompiler {
public:
    JitCompiler() noexcept;
    ~JitCompiler() noexcept;

    KernelFunc compile_comparison(uint64_t threshold) noexcept;

private:
    void dump_bytecode(const asmjit::CodeHolder& code, const char* label) const noexcept;

    std::unique_ptr<asmjit::JitRuntime> runtime_;
};

} // namespace jit
} // namespace apex
