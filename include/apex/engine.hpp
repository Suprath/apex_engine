#pragma once

#include "apex/core/registry.hpp"
#include "apex/core/types.hpp"
#include "apex/compute/bit_slicer.hpp"
#include "apex/compute/column_buffer.hpp"
#include "apex/jit/compiler.hpp"
#include <string>
#include <string_view>
#include <memory>
#include <cstring>
#include <unordered_map>

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

    uint64_t execute(const void* data_ptr, size_t row_count) noexcept;

private:
    struct SchemaMetadata {
        size_t row_stride;
        std::vector<core::FieldDescriptor> fields;
    };

    struct CompiledLogic {
        const core::FieldDescriptor* field;
        jit::KernelFunc kernel;
    };

    core::SchemaRegistry registry_;
    jit::JitCompiler compiler_;
    compute::BitSlicer slicer_;
    std::unordered_map<std::string, SchemaMetadata> schema_metadata_;
    std::unordered_map<std::string, CompiledLogic> compiled_logic_;

    void gather_field(const void* data_ptr,
                     const core::FieldDescriptor* field,
                     size_t row_stride,
                     size_t row_count,
                     compute::ColumnBuffer& out) const noexcept;

    uint64_t process_chunk(const uint64_t* gathered_values,
                          const CompiledLogic& logic) noexcept;
};

} // namespace apex
