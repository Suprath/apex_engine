# Unit tests for Apex Engine
# Test infrastructure and placeholder for test targets

# Registry tests
add_executable(test_registry test_registry.cpp)
target_include_directories(test_registry PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(test_registry PRIVATE apex)
target_compile_options(test_registry PRIVATE ${APEX_COMPILE_FLAGS})

# Add generated FlatBuffers headers directory if available
if(flatbuffers_FOUND)
    target_include_directories(test_registry PRIVATE "${FLATBUFFERS_GENERATED_DIR}")
endif()

add_test(NAME RegistryTests COMMAND test_registry)

# Memory fabric tests (requires iceoryx)
if(iceoryx_FOUND)
    add_executable(test_memory_fabric test_memory_fabric.cpp)
    target_include_directories(test_memory_fabric PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
    target_link_libraries(test_memory_fabric PRIVATE apex iceoryx_posh iceoryx_hoofs iceoryx_posh_roudi iceoryx_posh_config)
    target_compile_options(test_memory_fabric PRIVATE ${APEX_COMPILE_FLAGS})
    target_compile_definitions(test_memory_fabric PRIVATE APEX_HAS_ICEORYX)

    # Add generated FlatBuffers headers directory if available
    if(flatbuffers_FOUND)
        target_include_directories(test_memory_fabric PRIVATE "${FLATBUFFERS_GENERATED_DIR}")
    endif()

    add_test(NAME MemoryFabricTests COMMAND test_memory_fabric)
    message(STATUS "Tests configured: test_registry, test_memory_fabric")
else()
    message(STATUS "Tests configured: test_registry (test_memory_fabric requires iceoryx)")
endif()
