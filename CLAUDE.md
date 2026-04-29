# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Project Apex** is a 10B+ TPS Universal JIT Logic Engine designed for extreme performance in sub-microsecond latencies. The engine combines runtime code generation (AsmJit), portable SIMD (Google Highway), zero-copy IPC (iceoryx), and schema-driven serialization (FlatBuffers) into a unified, high-performance execution platform.

**Key Goals**:
- Throughput: 10B+ transactions per second
- Latency: Sub-microsecond p99 (< 1000 ns)
- Memory: Fixed-size, predictable footprint
- Determinism: Platform-independent results via fixed-point arithmetic

## Development Philosophy: ULL (Ultra-Low Latency)

All code in hot paths must follow the **4 ULL Directives** (documented in DEVELOPMENT_GUIDE.md):

1. **Zero Heap**: No dynamic allocation in the hot path.
2. **Branchless**: Use bitwise logic and cmov instead of branches.
3. **Cache-First**: Align hot structs to 64 bytes (L1D cache line).
4. **Fixed-Point**: Use int64_t for all numeric values; avoid floating-point.

These are non-negotiable for latency-critical code. Mark functions with comments indicating if they are "hot-path safe" (zero-alloc, branchless, etc.).

## Build System

- **Build tool**: CMake 3.18+
- **C++ standard**: C++20 (strict)
- **Compiler flags**: `-O3 -march=native -ffast-math -fno-finite-math-only`
- **Primary artifacts**:
  - `libapex.so` — Shared library (APEX runtime)
  - `apex_engine` — Production executable

### Building Locally

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja

# Build
cmake --build build

# Run
./build/apex_engine
```

Debug builds enable sanitizers (ASan, UBSan):

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -G Ninja
cmake --build build
```

### Building with Docker

Use the provided `build.sh` script for simplified builds:

```bash
# Local build
./build.sh --release
./build.sh --debug

# Docker build (recommended)
./build.sh --docker --release
./build.sh --docker --debug

# Clean rebuild
./build.sh --clean --docker

# Build and run tests/benchmarks
./build.sh --docker --test
./build.sh --docker --benchmark
```

Or use Docker directly:

```bash
# One-shot Docker build
docker build -f .docker/build.Dockerfile -t apex-engine:latest .
docker run --rm -v $(pwd):/workspace apex-engine:latest

# Interactive Docker container
docker-compose -f .docker/docker-compose.yml up -d
docker-compose -f .docker/docker-compose.yml exec apex-build bash
```

### CMake Architecture

- **OBJECT library** (`apex_obj`): Compiles all src/ files once, shared across targets.
- **SHARED library** (`apex`): Runtime library; links to apex_obj, integrates external dependencies.
- **EXECUTABLE** (`apex_engine`): Production runner; links to apex shared library.

External dependencies are optional (find_package with graceful fallback):
- **AsmJit**: Runtime JIT code generation (x86/x64, ARM)
- **Google Highway**: Portable SIMD library
- **iceoryx**: Zero-copy IPC and pub-sub transport
- **FlatBuffers**: Schema-driven, zero-copy serialization

## Code Organization

### src/ Structure

- **src/core/**: Fundamental primitives, memory pools, synchronization primitives, initialization.
- **src/memory/**: Memory management, arena allocators, fixed-size pools (no heap in hot paths).
- **src/jit/**: JIT compiler using AsmJit; code generation and optimization.
- **src/compute/**: Logic execution engine, bytecode interpreter, hot-path scheduler.
- **src/api/**: Public API and FFI boundaries; wraps hot-path code.
- **src/telemetry/**: Metrics, tracing, observability (must not touch hot paths).

### Supporting Directories

- **include/apex/**: Public headers for the library.
- **fbs/**: FlatBuffers schema files (.fbs).
- **tests/**: Unit and integration tests (add tests as features are built).
- **benchmarks/**: Performance microbenchmarks to validate latency targets.
- **.docker/**: Docker configuration files (Dockerfile, docker-compose.yml).

## Testing & Validation

- **Unit tests**: `tests/` — Use standard C++ testing (e.g., Catch2, Google Test).
- **Benchmarks**: `benchmarks/` — Measure throughput and latency; validate p99 < 1000 ns.
- **Profiling**: Use `perf`, `cachegrind`, or CPU sampling to identify bottlenecks.
- **Sanitizers**: Debug builds include ASan/UBSan/TSan for correctness.

## Docker Setup

Docker support is **fully configured**. See `.docker/README.md` for detailed instructions.

### Quick Docker Start

```bash
# Build Docker image
docker build -f .docker/build.Dockerfile -t apex-engine:latest .

# Run interactive container
docker run -it --rm -v $(pwd):/workspace apex-engine:latest bash

# Or use docker-compose
docker-compose -f .docker/docker-compose.yml up -d apex-build
docker-compose -f .docker/docker-compose.yml exec apex-build bash
```

### CLion Docker Toolchain Integration

1. **Settings** → **Tools** → **Docker**
   - Ensure Docker is connected
   
2. **Settings** → **Tools** → **CMake** → **Toolchain** → **+**
   - Name: `apex-docker`
   - Toolchain: Docker
   - Docker image: `apex-engine:latest`
   - CMake: `/usr/bin/cmake`
   - Make: `/usr/bin/ninja`
   - C Compiler: `/usr/bin/clang`
   - C++ Compiler: `/usr/bin/clang++`

3. **Settings** → **CMake** → **Profiles**
   - Toolchain: `apex-docker`
   - Generator: `Ninja`
   - Build type: `Release` or `Debug`

Now CLion will build, debug, and run inside Docker automatically.

## Dependencies

All external libraries are optional; code gracefully degrades if not found:

| Library | Purpose | Flag |
|---------|---------|------|
| AsmJit | Runtime code generation | `APEX_HAS_ASMJIT` |
| Google Highway | Portable SIMD | `APEX_HAS_HIGHWAY` |
| iceoryx | Zero-copy IPC | `APEX_HAS_ICEORYX` |
| FlatBuffers | Serialization | `APEX_HAS_FLATBUFFERS` |

Enable features conditionally via compile definitions.

## Compiler Flags & Optimization

**Production (Release)**: `-O3 -march=native -ffast-math -fno-finite-math-only`
- `-O3`: Maximum optimization
- `-march=native`: CPU-specific instructions (SIMD, AVX, etc.)
- `-ffast-math`: Unsafe floating-point optimizations (if needed)
- `-fno-finite-math-only`: Allow inf/nan for safety

**Debug**: `-O0 -g -fsanitize=address,undefined -fno-omit-frame-pointer`
- Sanitizers catch undefined behavior and memory issues
- Symbols for debugging and profiling

## Performance Targets

- **Throughput**: 10B+ transactions per second
- **Latency**: p99 < 1000 ns (1 microsecond)
- **Memory**: Fixed-size, predictable; no surprises under load
- **Determinism**: Platform-independent via fixed-point arithmetic and branchless logic
