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

message(STATUS "Tests configured: test_registry")
