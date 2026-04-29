cmake_minimum_required(VERSION 3.18)
project(apex_engine VERSION 1.0.0 LANGUAGES CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} -std=c++20")

# Build type defaults
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Extreme optimization flags for production
set(APEX_COMPILE_FLAGS -O3 -march=native -ffast-math -fno-finite-math-only -Wall -Wextra)

# Debug flags with sanitizers
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer")

# Release flags (production)
set(CMAKE_CXX_FLAGS_RELEASE "${APEX_COMPILE_FLAGS}")

# Include directories
include_directories(include)

# Source files for OBJECT library
set(APEX_SOURCES
    src/core/core.cpp
    src/memory/memory.cpp
    src/jit/jit.cpp
    src/compute/compute.cpp
    src/api/api.cpp
    src/telemetry/telemetry.cpp
)

# OBJECT library: shared compilation unit for linking into both SHARED and EXECUTABLE
add_library(apex_obj OBJECT ${APEX_SOURCES})
target_compile_options(apex_obj PRIVATE ${APEX_COMPILE_FLAGS})
target_include_directories(apex_obj PUBLIC include)

# External Dependencies: Try submodules first, then system packages

# AsmJit - JIT code generation
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/asmjit/CMakeLists.txt")
    message(STATUS "Building AsmJit from submodule")
    add_subdirectory(external/asmjit)
    set(asmjit_FOUND TRUE)
else()
    find_package(asmjit QUIET)
    if(NOT asmjit_FOUND)
        message(STATUS "AsmJit not found; JIT features will be disabled")
    endif()
endif()

# Google Highway - Portable SIMD
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/highway/CMakeLists.txt")
    message(STATUS "Building Google Highway from submodule")
    set(HWY_ENABLE_EXAMPLES OFF CACHE BOOL "Disable Highway examples")
    set(HWY_ENABLE_TESTS OFF CACHE BOOL "Disable Highway tests")
    add_subdirectory(external/highway)
    set(highway_FOUND TRUE)
else()
    find_package(highway QUIET)
    if(NOT highway_FOUND)
        message(STATUS "Google Highway not found; SIMD features will be disabled")
    endif()
endif()

# iceoryx - Zero-copy IPC
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/iceoryx/CMakeLists.txt")
    message(STATUS "Building iceoryx from submodule")
    set(IOX_BUILD_TESTS OFF CACHE BOOL "Disable iceoryx tests")
    set(IOX_BUILD_EXAMPLES OFF CACHE BOOL "Disable iceoryx examples")
    add_subdirectory(external/iceoryx)
    set(iceoryx_binding_c_FOUND TRUE)
else()
    find_package(iceoryx_binding_c QUIET)
    if(NOT iceoryx_binding_c_FOUND)
        message(STATUS "iceoryx not found; IPC features will be disabled")
    endif()
endif()

# FlatBuffers - Serialization
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/flatbuffers/CMakeLists.txt")
    message(STATUS "Building FlatBuffers from submodule")
    set(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "Disable FlatBuffers tests")
    set(FLATBUFFERS_BUILD_FLATLIB OFF CACHE BOOL "Disable FlatLib")
    add_subdirectory(external/flatbuffers)
    set(flatbuffers_FOUND TRUE)
else()
    find_package(flatbuffers QUIET)
    if(NOT flatbuffers_FOUND)
        message(STATUS "FlatBuffers not found; serialization features will be disabled")
    endif()
endif()

# SHARED library: apex runtime library
add_library(apex SHARED
    $<TARGET_OBJECTS:apex_obj>
)
target_compile_options(apex PRIVATE ${APEX_COMPILE_FLAGS})
target_include_directories(apex PUBLIC include)

if(asmjit_FOUND)
    target_link_libraries(apex PRIVATE asmjit::asmjit)
    target_compile_definitions(apex PRIVATE APEX_HAS_ASMJIT)
endif()

if(highway_FOUND)
    target_link_libraries(apex PRIVATE hwy)
    target_compile_definitions(apex PRIVATE APEX_HAS_HIGHWAY)
endif()

if(iceoryx_binding_c_FOUND)
    target_link_libraries(apex PRIVATE iceoryx_binding_c::iceoryx_binding_c)
    target_compile_definitions(apex PRIVATE APEX_HAS_ICEORYX)
endif()

if(flatbuffers_FOUND)
    target_link_libraries(apex PRIVATE flatbuffers)
    target_compile_definitions(apex PRIVATE APEX_HAS_FLATBUFFERS)
endif()

# EXECUTABLE: production runner (apex_engine)
add_executable(apex_engine src/main.cpp)
target_compile_options(apex_engine PRIVATE ${APEX_COMPILE_FLAGS})
target_include_directories(apex_engine PRIVATE include)
target_link_libraries(apex_engine PRIVATE apex)

# Testing
enable_testing()
add_subdirectory(tests)

# Benchmarking
add_subdirectory(benchmarks)
