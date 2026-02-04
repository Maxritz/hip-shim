cmake_minimum_required(VERSION 3.21)

# Test executable
add_executable(test_shim test_shim.cpp)
target_include_directories(test_shim PRIVATE ${HIP_INCLUDE_DIR})
target_compile_definitions(test_shim PRIVATE __HIP_PLATFORM_AMD__)

# Link against the shim DLL
target_link_libraries(test_shim PRIVATE ${CMAKE_BINARY_DIR}/Release/amdhip64.lib)

# Make sure test_shim is built after the shim DLL
add_dependencies(test_shim amdhip64)

# Set output directory
set_target_properties(test_shim PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
