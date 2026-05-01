#include "apex/engine.hpp"
#include <algorithm>
#include <cstring>

namespace apex {

ApexEngine::ApexEngine() noexcept = default;

void ApexEngine::register_schema(std::string_view schema_name,
                                 const std::vector<core::FieldDescriptor>& fields,
                                 size_t total_row_stride) noexcept {
    registry_.register_schema(schema_name, fields);
    schema_metadata_[std::string(schema_name)] = {total_row_stride, fields};
}

void ApexEngine::set_logic(std::string_view schema_name,
                           std::string_view field_name,
                           uint64_t threshold) noexcept {
    const auto* field = registry_.get_field(schema_name, field_name);
    if (!field) return;

    auto kernel = compiler_.compile_comparison(threshold);
    if (!kernel) return;

    std::string logic_key = std::string(schema_name) + ":" + std::string(field_name);
    compiled_logic_[logic_key] = {field, kernel};
}

void ApexEngine::gather_field(const void* data_ptr,
                              const core::FieldDescriptor* field,
                              size_t row_stride,
                              size_t row_count,
                              compute::ColumnBuffer& out) const noexcept {
    const uint8_t* base = static_cast<const uint8_t*>(data_ptr);
    size_t rows_to_gather = std::min(row_count, size_t(64));

    for (size_t i = 0; i < rows_to_gather; i++) {
        const uint8_t* row_ptr = base + i * row_stride + field->offset;
        std::memcpy(&out.data[i], row_ptr, sizeof(uint64_t));
    }

    // Zero-pad remaining slots
    for (size_t i = rows_to_gather; i < 64; i++) {
        out.data[i] = 0;
    }
}

uint64_t ApexEngine::process_chunk(const uint64_t* gathered_values,
                                    const CompiledLogic& logic) noexcept {
    compute::ColumnBuffer input;
    compute::ColumnBuffer bit_planes;

    std::memcpy(input.data, gathered_values, 64 * sizeof(uint64_t));

    slicer_.slice(input, bit_planes);

    uint64_t result_mask = logic.kernel(bit_planes.data);
    return result_mask;
}

uint64_t ApexEngine::execute(const void* data_ptr, size_t row_count) noexcept {
    // Find the active logic (assumes single logic per engine instance)
    if (compiled_logic_.empty()) return 0;

    const auto& [logic_key, logic] = *compiled_logic_.begin();

    // Parse schema name from logic key
    size_t colon_pos = logic_key.find(':');
    if (colon_pos == std::string::npos) return 0;

    std::string schema_name = logic_key.substr(0, colon_pos);
    auto meta_it = schema_metadata_.find(schema_name);
    if (meta_it == schema_metadata_.end()) return 0;

    size_t row_stride = meta_it->second.row_stride;
    uint64_t total_matches = 0;

    compute::ColumnBuffer gathered;
    const uint8_t* base = static_cast<const uint8_t*>(data_ptr);

    // Process in 64-row chunks
    for (size_t chunk = 0; chunk * 64 < row_count; chunk++) {
        const void* chunk_ptr = base + chunk * 64 * row_stride;
        size_t rows_in_chunk = std::min(size_t(64), row_count - chunk * 64);

        gather_field(chunk_ptr, logic.field, row_stride, rows_in_chunk, gathered);

        uint64_t chunk_mask = process_chunk(gathered.data, logic);

        // Count matches using popcnt
        total_matches += static_cast<uint64_t>(__builtin_popcountll(chunk_mask));
    }

    return total_matches;
}

} // namespace apex
