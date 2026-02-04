cmake_minimum_required(VERSION 3.21)
project(hip_runtime_test)

set(CMAKE_CXX_STANDARD 17)

# Find HIP SDK
if(NOT DEFINED ENV{HIP_PATH})
    message(FATAL_ERROR "HIP_PATH not set")
endif()

set(HIP_PATH "$ENV{HIP_PATH}")
file(TO_CMAKE_PATH "${HIP_PATH}" HIP_PATH)
set(HIP_INCLUDE_DIR "${HIP_PATH}/include")
set(HIP_LIB_DIR "${HIP_PATH}/lib")

# Create test executable
add_executable(hip_runtime_test hip_runtime_test.cpp)

target_include_directories(hip_runtime_test PRIVATE ${HIP_INCLUDE_DIR})

# Link against amdhip64.lib (import library)
target_link_libraries(hip_runtime_test PRIVATE "${HIP_LIB_DIR}/amdhip64.lib")

if(MSVC)
    target_compile_options(hip_runtime_test PRIVATE /W4 /EHsc)
    target_compile_definitions(hip_runtime_test PRIVATE 
        WIN32_LEAN_AND_MEAN 
        NOMINMAX 
        _CRT_SECURE_NO_WARNINGS
        __HIP_PLATFORM_AMD__
    )
endif()
