#pragma once

#include "apex/core/registry.hpp"
#include "apex/core/types.hpp"
#include "apex/compute/bit_slicer.hpp"
#include "apex/compute/column_buffer.hpp"
#include "apex/jit/compiler.hpp"
#include "apex/jit/ir.hpp"
#include <string>
#include <string_view>
#include <memory>
#include <cstring>
#include <unordered_map>
#include <array>
#include <vector>

namespace apex {

class ApexEngine {
public:
    ApexEngine() noexcept;
    ~ApexEngine() noexcept = default;

    void register_schema(std::string_view schema_name,
                        const std::vector<core::FieldDescriptor>& fields,
                        size_t total_row_stride) noexcept;

    void set_logic(std::string_view schema_name,
                   std::string_view field_name,
                   uint64_t threshold) noexcept;

    // Expression-based API
    void set_expression(std::string_view schema_name, ir::Node* expr_root) noexcept;

    uint64_t execute(const void* data_ptr, size_t row_count) noexcept;
    uint64_t execute_parallel(const void* data_ptr, size_t row_count, int num_threads = 4) noexcept;

    // Public access for benchmarking and testing
    void gather_field(const void* data_ptr,
                     const core::FieldDescriptor* field,
                     size_t row_stride,
                     size_t row_count,
                     compute::ColumnBuffer& out) const noexcept;

    jit::JitCompiler& get_compiler() noexcept { return compiler_; }
    const core::SchemaRegistry& get_registry() const noexcept { return registry_; }

private:
    struct SchemaMetadata {
        size_t row_stride;
        std::vector<core::FieldDescriptor> fields;
    };

    struct CompiledLogic {
        const core::FieldDescriptor* field;
        jit::KernelFunc kernel;
    };

    struct ExprCompiledLogic {
        jit::ExprKernelFunc kernel;
        std::vector<const core::FieldDescriptor*> fields;  // indexed by field_idx
    };

    core::SchemaRegistry registry_;
    jit::JitCompiler compiler_;
    compute::BitSlicer slicer_;
    std::unordered_map<std::string, SchemaMetadata> schema_metadata_;
    std::unordered_map<std::string, CompiledLogic> compiled_logic_;
    std::unordered_map<std::string, ExprCompiledLogic> expr_logic_;

    // Pre-allocated column buffers for multi-field gather (up to 8 fields)
    std::array<compute::ColumnBuffer, 8> field_buffers_;

    uint64_t process_chunk(const uint64_t* gathered_values,
                          const CompiledLogic& logic) noexcept;

    uint64_t process_chunk_expr(
        const void* data_ptr,
        size_t row_stride,
        size_t row_count,
        const ExprCompiledLogic& expr_logic) noexcept;
};

} // namespace apex
