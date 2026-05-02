#include "apex/compute/parallel_runner.hpp"
#include <thread>
#include <array>
#include <algorithm>
#include <cstring>

#ifdef __APPLE__
#include <pthread.h>
#endif

namespace apex::compute {

// Cache-line padded result slot to prevent false sharing between threads
struct alignas(64) PaddedResult {
    uint64_t count = 0;
    char pad[56];  // Fill to 64 bytes
};

// Per-thread worker function — completely self-contained, zero shared mutable state
static void worker_thread(
    const uint8_t* base,
    size_t start_row,
    size_t end_row,
    const ParallelRunner::TaskConfig& config,
    PaddedResult* result) noexcept {

#ifdef __APPLE__
    // Hint macOS scheduler to use Performance cores (not Efficiency cores)
    pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
#endif

    // Per-thread resources — zero heap, zero sharing
    BitSlicer slicer;
    std::array<ColumnBuffer, 8> field_buffers;

    struct alignas(64) ScratchpadBuffer {
        uint64_t data[8 * 64];
    };
    ScratchpadBuffer scratchpad;

    const size_t row_stride = config.row_stride;
    const size_t num_fields = config.fields.size();
    uint64_t total_matches = 0;

    // Process in 64-row blocks
    for (size_t row = start_row; row < end_row; row += 64) {
        const uint8_t* chunk_base = base + row * row_stride;
        size_t rows_in_chunk = std::min(size_t(64), end_row - row);

        // Prefetch 2 blocks (128 rows) ahead — fills L1 before we need it
        __builtin_prefetch(chunk_base + 128 * row_stride, 0, 3);
        __builtin_prefetch(chunk_base + 192 * row_stride, 0, 3);

        // Gather + slice all referenced fields
        alignas(64) const uint64_t* field_planes[8] = {};
        for (size_t f = 0; f < num_fields && f < 8; ++f) {
            const size_t offset = config.fields[f]->offset;

            // 4x unrolled gather
            size_t i = 0;
            for (; i + 4 <= rows_in_chunk; i += 4) {
                uint64_t v0, v1, v2, v3;
                std::memcpy(&v0, chunk_base + (i + 0) * row_stride + offset, 8);
                std::memcpy(&v1, chunk_base + (i + 1) * row_stride + offset, 8);
                std::memcpy(&v2, chunk_base + (i + 2) * row_stride + offset, 8);
                std::memcpy(&v3, chunk_base + (i + 3) * row_stride + offset, 8);
                field_buffers[f].data[i + 0] = v0;
                field_buffers[f].data[i + 1] = v1;
                field_buffers[f].data[i + 2] = v2;
                field_buffers[f].data[i + 3] = v3;
            }
            for (; i < rows_in_chunk; i++) {
                std::memcpy(&field_buffers[f].data[i], chunk_base + i * row_stride + offset, 8);
            }
            for (; i < 64; i++) {
                field_buffers[f].data[i] = 0;
            }

            if (config.mode == ExecutionMode::BIT_SLICED) {
                slicer.slice(field_buffers[f], field_buffers[f]);
            }
            field_planes[f] = field_buffers[f].data;
        }

        // Execute JIT kernel
        uint64_t mask = config.kernel(field_planes, scratchpad.data);
        total_matches += static_cast<uint64_t>(__builtin_popcountll(mask));
    }

    result->count = total_matches;
}

uint64_t ParallelRunner::run(
    const void* data_ptr,
    size_t total_rows,
    const TaskConfig& config,
    int num_threads) noexcept {

    if (num_threads <= 0) num_threads = 1;
    if (total_rows == 0) return 0;

    const uint8_t* base = static_cast<const uint8_t*>(data_ptr);

    // Align chunk boundaries to 64-row blocks
    size_t rows_per_thread = ((total_rows / num_threads) / 64) * 64;
    if (rows_per_thread == 0) rows_per_thread = 64;

    // Cache-line padded result slots — no false sharing
    std::vector<PaddedResult> results(num_threads);
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (int t = 0; t < num_threads; ++t) {
        size_t start = t * rows_per_thread;
        size_t end = (t == num_threads - 1) ? total_rows : start + rows_per_thread;

        if (start >= total_rows) break;

        threads.emplace_back(worker_thread,
                             base, start, end,
                             std::cref(config),
                             &results[t]);
    }

    // Wait for all workers
    for (auto& t : threads) {
        t.join();
    }

    // Aggregate results
    uint64_t total = 0;
    for (int t = 0; t < num_threads; ++t) {
        total += results[t].count;
    }
    return total;
}

} // namespace apex::compute
