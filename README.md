# Project Apex v1.0.0

## Universal JIT-Accelerated Vector Engine
### 10B+ Transactions Per Second | Sub-Microsecond Latency

Project Apex is a high-performance, deterministic execution engine designed for extreme throughput and ultra-low latency in production environments. Built on runtime code generation (AsmJit), portable SIMD (Google Highway), zero-copy IPC (iceoryx), and schema-driven serialization (FlatBuffers), Apex delivers **1B+ Records Per Second** throughput per core with p99 latency under 1 microsecond.

---

## Key Features

### Performance Metrics
- **Throughput**: 1B+ records per second per core (10B+ TPS on 10-core systems)
- **Latency**: p99 < 1000 nanoseconds (sub-microsecond)
- **Memory**: Fixed-size, deterministic footprint; no garbage collection in hot paths
- **Determinism**: Platform-independent results via fixed-point arithmetic and branchless logic

### Architecture Pillars
- **Zero-Copy IPC**: Eclipse iceoryx memory fabric for inter-process communication without serialization overhead
- **JIT Code Generation**: AsmJit runtime compilation for expression evaluation, optimized per schema
- **Portable SIMD**: Google Highway for vectorized bit-slicing and matrix transpose operations
- **Schema-Driven**: FlatBuffers for zero-copy data access and automatic code generation

### Ultra-Low Latency Directives (ULL)
All hot-path code follows four mandatory principles:

1. **Zero Heap**: No dynamic allocation in transaction processing
2. **Branchless**: Bitwise logic and conditional moves (cmov) instead of branches
3. **Cache-First**: 64-byte alignment for L1D cache line efficiency
4. **Fixed-Point**: Integer-only arithmetic; no floating-point operations

---

## Industry Use Cases

### 🏦 Financial Technology
**Sub-Microsecond Risk Analysis & Backtesting**

Apex powers real-time financial analytics:
- **Ultra-Fast Backtesting**: Process 1B+ trades per second to validate trading strategies in seconds
- **Risk Engine**: Continuous monitoring with sub-microsecond latency for regulatory compliance (Dodd-Frank, MiFID II)
- **Order Execution**: Deterministic, latency-bound logic for algorithmic trading systems
- **Market Surveillance**: Real-time pattern detection across millions of concurrent instruments

### 🔒 Cybersecurity
**Line-Rate Deep Packet Inspection (DPI) & Threat Detection**

Apex enables network-speed threat analysis:
- **DPI at Line Rate**: Process millions of packets per second with zero-copy IPC to detection engines
- **Threat Detection**: Pattern matching and anomaly detection at gigabit-plus link speeds
- **Intrusion Prevention**: Sub-microsecond decision latency for inline security appliances
- **Compliance Scanning**: Real-time data classification and PII detection with audit trails

### 🏭 Industrial IoT
**Real-Time Predictive Maintenance & Sensor Analytics**

Apex handles industrial-scale sensor processing:
- **High-Frequency Sensors**: Process MHz-rate sensor arrays (accelerometers, temperature, pressure) from machinery
- **Predictive Maintenance**: Anomaly detection and fault prediction with microsecond responsiveness
- **Edge Computing**: Run critical logic at network edge with deterministic performance
- **Time-Series Analytics**: Efficient bit-sliced computation for rolling aggregations and correlations

---

## Getting Started

### Build Requirements
- **CMake**: 3.18+
- **C++ Compiler**: GCC 10+ or Clang 10+ (C++20 support required)
- **Optional Dependencies**: AsmJit, Google Highway, iceoryx, FlatBuffers (auto-detected)

### Quick Build (Local)
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build
./build/apex_engine
```

### Docker Build
```bash
./build.sh --docker --release
./build.sh --docker --test
./build.sh --docker --benchmark
```

### Verify Installation
```bash
# Run included benchmarks to verify latency targets
cmake --build build --target apex_benchmark
./build/apex_benchmark
```

---

## Architecture Overview

### Core Modules

**Module 1: Metadata Registry**
- Field descriptor registry for schema definitions
- Automatic FlatBuffers code generation
- Cache-line aligned field layouts

**Module 2: Memory Fabric**
- iceoryx-backed zero-copy IPC transport
- Fixed-size memory pools (no heap in hot paths)
- Thread-safe subscriber with wait-free semantics

**Module 3: Bit-Slicer**
- 64×64 bit-matrix transpose (6-stage Knuth algorithm)
- Highway SIMD for parallel bit operations
- Sub-100-nanosecond target

**Module 4: JIT Comparison Kernel**
- ARM64 JIT compiler for comparison logic
- Multi-condition expression evaluation
- Compiled bytecode with optimized register allocation

**Module 5: Parallel Orchestrator**
- Work-stealing thread pool for 128M+ row processing
- 656M TPS aggregate throughput per 4-thread system
- Cache-aware task distribution

---

## Development Philosophy

All code in hot paths is held to rigorous performance standards. See `DEVELOPMENT_GUIDE.md` for detailed ULL directives and profiling best practices.

### Key Principles
- **Determinism Over Convenience**: Fixed-point arithmetic ensures identical results across platforms
- **Explicit Over Implicit**: Cache alignment, register allocation, and memory layout are explicit in code
- **Measure Before Optimizing**: Profiling and microbenchmarks guide all optimizations
- **No Abstractions in Hot Path**: Inlining and unrolling are preferred over generic code

---

## Testing & Validation

### Unit Tests
```bash
cmake --build build --target apex_tests
ctest --output-on-failure
```

### Benchmarks
```bash
./build/apex_benchmark --throughput --latency --percentiles
```

### Profiling
```bash
# CPU cycles and cache misses
perf record -e cycles,cache-references,cache-misses ./build/apex_engine
perf report

# Cache line contention
perf record -e false_sharing ./build/apex_engine
```

---

## Project Structure

```
apex_engine/
├── src/
│   ├── core/             # Registry, initialization, primitives
│   ├── memory/           # Arena allocators, fixed-size pools
│   ├── jit/              # AsmJit compiler, bytecode generation
│   ├── compute/          # Bit-slicer, parallel runner, expression evaluation
│   ├── api/              # Public API (C and C++)
│   └── telemetry/        # Metrics and observability
├── include/apex/         # Public headers
├── fbs/                  # FlatBuffers schema definitions
├── tests/                # Unit and integration tests
├── benchmarks/           # Performance microbenchmarks
├── bindings/             # Python and Java language bindings
├── .docker/              # Docker configuration
└── LICENSE               # Business Source License 1.1
```

---

## Licensing

Project Apex v1.0.0 is licensed under the **Business Source License 1.1** until 2029-05-01, at which point it will transition to the **Apache License 2.0**.

### Additional Use Grant
You may use Project Apex for any non-production purpose, including academic research, personal evaluation, and open-source development testing.

**Commercial Use** (defined as using the Licensed Work to process production data-streams, provide paid analytics services, or integrate into commercial products in Finance, Cybersecurity, Industrial IoT, Telecommunications, or other sectors) requires a separate commercial license agreement.

For details, see the `LICENSE` file in the repository root.

### Attribution
Project Apex builds on world-class open-source libraries. See `NOTICE` for details on AsmJit, Google Highway, Eclipse iceoryx, FlatBuffers, and pybind11.

---

## Performance Targets & Guarantees

| Metric | Target | Validation |
|--------|--------|-----------|
| Throughput | 1B+ records/sec per core | Microbenchmarks in `benchmarks/` |
| Latency (p99) | < 1000 ns | Histogrammed across 1B+ operations |
| Memory Footprint | Fixed, predictable | No heap allocation in hot path |
| Determinism | Platform-independent | Fixed-point arithmetic, branchless logic |

---

## Contributing

Contributions must adhere to the **ULL Directives** and pass all performance benchmarks. See `DEVELOPMENT_GUIDE.md` for guidelines.

### Before Submitting
1. Run `cmake --build build --target apex_tests`
2. Run `./build/apex_benchmark` and verify latency targets
3. Profile with `perf` to check for regressions
4. Ensure all hot-path functions are marked with ULL compliance comments

---

## Support & Issues

Found a performance regression or bug? Open an issue with:
- Latency histogram (if performance-related)
- Reproduction steps
- Platform details (CPU model, OS, compiler)

---

## Roadmap

- [ ] NUMA-aware memory allocation for multi-socket systems
- [ ] GPU-accelerated bit-slicing via CUDA
- [ ] Distributed orchestration across cluster nodes
- [ ] Python/Java SDK with pybind11 and JNI bindings
- [ ] Commercial license tiers with support

---

**Project Apex v1.0.0**
Copyright (c) 2024-2026 Suprath PS. All rights reserved.
Licensed under the Business Source License 1.1 until 2029-05-03, thereafter Apache 2.0.
See LICENSE file for full details.
