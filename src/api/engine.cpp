#include "apex/engine.hpp"
#include "apex/compute/parallel_runner.hpp"
#include <algorithm>
#include <cstring>
#include <functional>
#include <iostream>

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

void ApexEngine::set_expression(std::string_view schema_name, ir::Node* expr_root, ExecutionMode mode) noexcept {
    if (!expr_root) return;

    jit::ExprKernelFunc kernel = nullptr;
    if (mode == ExecutionMode::BIT_SLICED) {
        kernel = compiler_.compile_expression(expr_root, registry_, schema_name);
    } else {
        kernel = compiler_.compile_scalar_expression(expr_root, registry_, schema_name);
    }
    if (!kernel) return;

    // Collect referenced fields from the expression tree
    std::vector<const core::FieldDescriptor*> fields;
    std::unordered_map<int, bool> field_indices_seen;

    std::function<void(ir::Node*)> collect_fields = [&](ir::Node* node) {
        if (!node) return;
        if (node->kind == ir::NodeKind::LOAD) {
            int idx = node->field_idx;
            if (idx >= 0 && field_indices_seen.find(idx) == field_indices_seen.end()) {
                const auto* field = registry_.get_field(schema_name, std::string_view(node->field_name));
                if (field) {
                    fields.push_back(field);
                    field_indices_seen[idx] = true;
                }
            }
        }
        collect_fields(node->left);
        collect_fields(node->right);
        if (node->kind == ir::NodeKind::SELECT) {
            collect_fields(node->cond);
        }
    };

    collect_fields(expr_root);

    // Sort fields by their assigned indices
    std::sort(fields.begin(), fields.end(),
        []([[maybe_unused]] const core::FieldDescriptor* a,
           [[maybe_unused]] const core::FieldDescriptor* b) {
        // We rely on the field assignment order from compile_expression
        return false; // Keep insertion order for now
    });

    std::string expr_key = std::string(schema_name);
    expr_logic_[expr_key] = {kernel, fields, mode};
}

void ApexEngine::gather_field(const void* data_ptr,
                              const core::FieldDescriptor* field,
                              size_t row_stride,
                              size_t row_count,
                              compute::ColumnBuffer& out) const noexcept {
    const uint8_t* base = static_cast<const uint8_t*>(data_ptr);
    size_t rows_to_gather = std::min(row_count, size_t(64));
    const size_t offset = field->offset;

    // 4x unrolled gather: exploit M3 dual load units
    size_t i = 0;
    for (; i + 4 <= rows_to_gather; i += 4) {
        // Prefetch 2 blocks (128 rows) ahead
        __builtin_prefetch(base + (i + 128) * row_stride + offset, 0, 3);
        __builtin_prefetch(base + (i + 130) * row_stride + offset, 0, 3);

        // 4x scalar loads from strided AoS memory (NEON can't gather from non-contiguous)
        uint64_t v0, v1, v2, v3;
        std::memcpy(&v0, base + (i + 0) * row_stride + offset, 8);
        std::memcpy(&v1, base + (i + 1) * row_stride + offset, 8);
        std::memcpy(&v2, base + (i + 2) * row_stride + offset, 8);
        std::memcpy(&v3, base + (i + 3) * row_stride + offset, 8);

        // Store as contiguous block — compiler will vectorize to stp
        out.data[i + 0] = v0;
        out.data[i + 1] = v1;
        out.data[i + 2] = v2;
        out.data[i + 3] = v3;
    }

    // Handle remaining rows (0-3)
    for (; i < rows_to_gather; i++) {
        std::memcpy(&out.data[i], base + i * row_stride + offset, 8);
    }

    // Zero-pad remaining slots
    for (; i < 64; i++) {
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

uint64_t ApexEngine::process_chunk_expr(
    const void* data_ptr,
    size_t row_stride,
    size_t row_count,
    const ExprCompiledLogic& expr_logic) noexcept {

    // Gather all referenced fields - use aligned array
    alignas(64) const uint64_t* field_planes_array[8] = {};

    for (size_t i = 0; i < expr_logic.fields.size() && i < 8; ++i) {
        gather_field(data_ptr, expr_logic.fields[i], row_stride, row_count, field_buffers_[i]);
        if (expr_logic.mode == ExecutionMode::BIT_SLICED) {
            slicer_.slice(field_buffers_[i], field_buffers_[i]);
        }
        field_planes_array[i] = field_buffers_[i].data;
    }

    // Thread-local scratchpad (4KB)
    struct ScratchpadBuffer {
        alignas(64) uint64_t data[8 * 64];
    };
    static thread_local ScratchpadBuffer scratchpad_buffer;

    // Call the JIT kernel
    uint64_t result_mask = expr_logic.kernel(field_planes_array, scratchpad_buffer.data);

    return result_mask;
}

uint64_t ApexEngine::execute(const void* data_ptr, size_t row_count) noexcept {
    // Check for expression-based logic first
    if (!expr_logic_.empty()) {
        auto meta_it = expr_logic_.begin();
        if (meta_it != expr_logic_.end()) {
            const std::string& schema_name = meta_it->first;
            const ExprCompiledLogic& expr_logic = meta_it->second;

            auto schema_it = schema_metadata_.find(schema_name);
            if (schema_it == schema_metadata_.end()) return 0;

            size_t row_stride = schema_it->second.row_stride;
            uint64_t total_matches = 0;
            const uint8_t* base = static_cast<const uint8_t*>(data_ptr);

            // Process in 64-row chunks
            for (size_t chunk = 0; chunk * 64 < row_count; chunk++) {
                const void* chunk_ptr = base + chunk * 64 * row_stride;
                size_t rows_in_chunk = std::min(size_t(64), row_count - chunk * 64);

                uint64_t chunk_mask = process_chunk_expr(chunk_ptr, row_stride, rows_in_chunk, expr_logic);
                total_matches += static_cast<uint64_t>(__builtin_popcountll(chunk_mask));
            }

            return total_matches;
        }
    }

    // Fall back to legacy single-field logic
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

uint64_t ApexEngine::execute_parallel(const void* data_ptr, size_t row_count, int num_threads) noexcept {
    if (expr_logic_.empty()) {
        return execute(data_ptr, row_count);
    }

    auto meta_it = expr_logic_.begin();
    if (meta_it != expr_logic_.end()) {
        const std::string& schema_name = meta_it->first;
        const ExprCompiledLogic& expr_logic = meta_it->second;

        auto schema_it = schema_metadata_.find(schema_name);
        if (schema_it == schema_metadata_.end()) return 0;

        compute::ParallelRunner::TaskConfig config;
        config.kernel = expr_logic.kernel;
        config.fields = expr_logic.fields;
        config.row_stride = schema_it->second.row_stride;
        config.mode = expr_logic.mode;

        return compute::ParallelRunner::run(data_ptr, row_count, config, num_threads);
    }

    return 0;
}

} // namespace apex
