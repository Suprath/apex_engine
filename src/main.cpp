#include <iostream>
#include <cstring>

#ifdef APEX_HAS_ICEORYX
#include <iceoryx_binding_c/runtime.h>
#endif

namespace apex {

bool initialize_runtime() {
#ifdef APEX_HAS_ICEORYX
    iox_runtime_init("apex_engine");
    return true;
#else
    return true;
#endif
}

void shutdown_runtime() {
#ifdef APEX_HAS_ICEORYX
    iox_runtime_shutdown();
#endif
}

} // namespace apex

int main() {
    if (!apex::initialize_runtime()) {
        std::cerr << "Failed to initialize Apex runtime\n";
        return 1;
    }

    std::cout << "Apex Engine Initialized [M3 Optimized]\n";

    apex::shutdown_runtime();
    return 0;
}
