# Python Bindings (PyBind11)
find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)

if(Python3_FOUND)
    if(NOT TARGET pybind11::module)
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../external/pybind11 pybind11_build)
    endif()

    pybind11_add_module(apex_python apex_pybind.cpp)
    target_include_directories(apex_python PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
    target_link_libraries(apex_python PRIVATE apex)

    # Install
    install(TARGETS apex_python DESTINATION python)
    message(STATUS "Python bindings configured")
else()
    message(WARNING "Python headers not found; skipping apex_python bindings")
endif()
