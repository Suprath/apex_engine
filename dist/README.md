# Project Apex v1.0.0 Distribution

This directory contains the final release distribution of Project Apex Universal Logic Engine.

## Contents

### /include
Public headers for C and C++ bindings:
- `apex_c_api.h` - C FFI interface for integration with C code
- `Apex.hpp` - C++ API for native integration

Use these headers to embed Apex into your applications.

### /lib
Compiled shared libraries (platform-specific):
- `libapex.so` (Linux)
- `libapex.dylib` (macOS)
- `libapex.dll` (Windows)

Link against your target platform's library during compilation.

### /bindings
Language-specific SDKs and bindings:
- `python/` - Python 3.8+ module (via pybind11)
- `java/` - Java SDK (via JNI)

### Root-Level Files
- `LICENSE` - Business Source License 1.1 with universal use grant
- `NOTICE` - Attribution for third-party components (AsmJit, Highway, iceoryx, FlatBuffers)

## Installation

### Linux / macOS
```bash
# Copy headers
cp include/*.h include/*.hpp /usr/local/include/apex/

# Copy library
cp lib/libapex.* /usr/local/lib/

# Update library cache
ldconfig  # Linux only
```

### CMake Integration
```cmake
find_package(Apex REQUIRED)
target_link_libraries(my_app PRIVATE Apex::apex)
```

### Python
```bash
pip install ./bindings/python
import apex
```

### Java
```bash
cp bindings/java/*.jar /path/to/classpath/
import com.suprath.apex.*;
```

## Licensing

**Business Source License 1.1** until 2029-05-03, then **Apache License 2.0**.

Commercial use (production data processing, paid analytics, or integration into commercial products) requires a separate commercial license agreement.

Non-production use (research, evaluation, open-source development) is permitted under the Additional Use Grant.

See `LICENSE` for full details.

## Performance Guarantees

- Throughput: 1B+ records/sec per core
- Latency (p99): < 1 microsecond
- Memory: Fixed, predictable footprint
- Determinism: Platform-independent results

## Support

For issues, feature requests, or commercial inquiries, contact:
📧 suprathps@gmail.com

---
Project Apex v1.0.0
Copyright (c) 2024-2026 Suprath PS. All rights reserved.
