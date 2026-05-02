# Performance microbenchmarks for Apex Engine
# Benchmark infrastructure and placeholder for benchmark targets

# Benchmark executables validate latency and throughput targets
# Add benchmarks as features are implemented

# Bit-Slicer benchmark (requires Highway)
if(highway_FOUND)
    add_executable(bench_bit_slicer bench_bit_slicer.cpp)
    target_include_directories(bench_bit_slicer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
    target_link_libraries(bench_bit_slicer PRIVATE apex)
    target_compile_options(bench_bit_slicer PRIVATE ${APEX_COMPILE_FLAGS})
    message(STATUS "Benchmarks configured: bench_bit_slicer")
else()
    message(STATUS "Benchmarks: bench_bit_slicer requires Highway (not found)")
endif()

# Final Apex benchmark with verification (Module 5 with scalar reference)
if(highway_FOUND AND asmjit_FOUND)
    add_executable(bench_apex_final bench_apex_final.cpp)
    target_include_directories(bench_apex_final PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
    target_include_directories(bench_apex_final PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../external/asmjit)
    target_link_libraries(bench_apex_final PRIVATE apex)
    target_compile_options(bench_apex_final PRIVATE ${APEX_COMPILE_FLAGS})

    add_executable(bench_10b_tps bench_10b_tps.cpp)
    target_include_directories(bench_10b_tps PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
    target_include_directories(bench_10b_tps PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../external/asmjit)
    target_link_libraries(bench_10b_tps PRIVATE apex)
    target_compile_options(bench_10b_tps PRIVATE ${APEX_COMPILE_FLAGS})

    message(STATUS "Benchmarks configured: bench_bit_slicer, bench_apex_final, bench_10b_tps")
else()
    message(STATUS "Benchmarks: bench_apex_final and bench_10b_tps require Highway + AsmJit (not found)")
endif()
