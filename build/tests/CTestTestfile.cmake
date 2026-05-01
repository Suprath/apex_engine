# CMake generated Testfile for 
# Source directory: /workspace/tests
# Build directory: /workspace/build/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(RegistryTests "/workspace/build/tests/test_registry")
set_tests_properties(RegistryTests PROPERTIES  _BACKTRACE_TRIPLES "/workspace/tests/CMakeLists.txt;15;add_test;/workspace/tests/CMakeLists.txt;0;")
add_test(BitSlicerTransposeTests "/workspace/build/tests/test_bit_slicer_transpose")
set_tests_properties(BitSlicerTransposeTests PROPERTIES  _BACKTRACE_TRIPLES "/workspace/tests/CMakeLists.txt;27;add_test;/workspace/tests/CMakeLists.txt;0;")
add_test(SDKIoTTests "/workspace/build/tests/test_sdk_iot")
set_tests_properties(SDKIoTTests PROPERTIES  _BACKTRACE_TRIPLES "/workspace/tests/CMakeLists.txt;46;add_test;/workspace/tests/CMakeLists.txt;0;")
add_test(MemoryFabricTests "/workspace/build/tests/test_memory_fabric")
set_tests_properties(MemoryFabricTests PROPERTIES  _BACKTRACE_TRIPLES "/workspace/tests/CMakeLists.txt;61;add_test;/workspace/tests/CMakeLists.txt;0;")
