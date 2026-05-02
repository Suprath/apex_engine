#include "apex/Apex.hpp"
#include "apex/jit/ir.hpp"
#include <iostream>
#include <vector>
#include <chrono>

using namespace apex;

struct alignas(64) GenericRecord {
    uint64_t field0;
    uint64_t field1;
    uint64_t field2;
    uint64_t padding[5];
};

int main() {
    std::cout << "=== Universal C++ SDK Verification Test ===" << std::endl;
    
    Apex engine;

    std::vector<apex_field_descriptor_t> fields = {
        {"Field0", offsetof(GenericRecord, field0), 64, 3}, // UINT64
        {"Field1", offsetof(GenericRecord, field1), 64, 3},
        {"Field2", offsetof(GenericRecord, field2), 64, 3}
    };
    engine.register_schema("GenericSchema", fields, sizeof(GenericRecord));

    // Logic: (Field0 + Field1) > Field2
    auto f0 = ir::make_load("Field0");
    auto f1 = ir::make_load("Field1");
    auto sum = ir::make_add(f0, f1);
    auto f2 = ir::make_load("Field2");
    auto root = ir::make_gt(sum, f2);

    engine.set_logic("GenericSchema", root, APEX_EXEC_MODE_SCALAR);

    const size_t num_rows = 1000000;
    std::vector<GenericRecord> dataset(num_rows);
    
    // Generate data
    for (size_t i = 0; i < num_rows; ++i) {
        dataset[i].field0 = i;
        dataset[i].field1 = 1000;
        dataset[i].field2 = (i % 2 == 0) ? (i + 500) : (i + 2000);
    }
    // Condition: i + 1000 > field2. 
    // If even: field2 = i + 500. i + 1000 > i + 500 (True)
    // If odd: field2 = i + 2000. i + 1000 > i + 2000 (False)
    // Expected matches: 500,000

    auto start = std::chrono::high_resolution_clock::now();
    uint64_t matches = engine.execute(dataset.data(), num_rows);
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Matches: " << matches << " (Expected: 500000)" << std::endl;
    std::cout << "Time: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms" << std::endl;

    if (matches == 500000) {
        std::cout << "[PASS] Universal C++ SDK Test" << std::endl;
        return 0;
    } else {
        std::cout << "[FAIL] Incorrect matches" << std::endl;
        return 1;
    }
}
