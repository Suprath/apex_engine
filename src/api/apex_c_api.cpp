#include "apex/apex_c_api.h"
#include "apex/engine.hpp"
#include <iostream>

using namespace apex;

extern "C" {

apex_engine_h apex_create(void) {
    try {
        return new ApexEngine();
    } catch (...) {
        return nullptr;
    }
}

void apex_destroy(apex_engine_h handle) {
    if (handle) {
        delete static_cast<ApexEngine*>(handle);
    }
}

int apex_register_schema(
    apex_engine_h handle, 
    const char* schema_name, 
    const apex_field_descriptor_t* fields, 
    size_t num_fields, 
    size_t stride) {
    
    if (!handle || !schema_name || !fields) return -1;

    try {
        ApexEngine* engine = static_cast<ApexEngine*>(handle);
        std::vector<core::FieldDescriptor> cpp_fields;
        cpp_fields.reserve(num_fields);

        for (size_t i = 0; i < num_fields; ++i) {
            cpp_fields.push_back({
                std::string(fields[i].name),
                static_cast<uint32_t>(fields[i].offset),
                static_cast<uint32_t>(fields[i].bit_width),
                static_cast<core::DataType>(fields[i].data_type)
            });
        }

        engine->register_schema(schema_name, cpp_fields, stride);
        return 0;
    } catch (...) {
        return -1;
    }
}

int apex_set_logic(apex_engine_h handle, const char* schema_name, void* ir_root_ptr, int mode) {
    if (!handle || !schema_name || !ir_root_ptr) return -1;

    try {
        ApexEngine* engine = static_cast<ApexEngine*>(handle);
        ir::Node* root = static_cast<ir::Node*>(ir_root_ptr);
        ExecutionMode exec_mode = (mode == APEX_EXEC_MODE_SCALAR) ? ExecutionMode::SCALAR : ExecutionMode::BIT_SLICED;
        
        engine->set_expression(schema_name, root, exec_mode);
        return 0;
    } catch (...) {
        return -1;
    }
}

uint64_t apex_execute(apex_engine_h handle, const void* data_ptr, size_t count) {
    if (!handle || !data_ptr) return (uint64_t)-1;

    try {
        ApexEngine* engine = static_cast<ApexEngine*>(handle);
        return engine->execute_parallel(data_ptr, count, 4); // Default 4 threads
    } catch (...) {
        return (uint64_t)-1;
    }
}

void* apex_create_universal_test_logic(void) {
    auto f0 = apex::builder::Load("Field0");
    auto f1 = apex::builder::Load("Field1");
    auto sum = apex::builder::Add(f0, f1);
    auto f2 = apex::builder::Load("Field2");
    auto root = apex::builder::GT(sum, f2);
    return root;
}

} // extern "C"
