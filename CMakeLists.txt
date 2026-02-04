cmake_minimum_required(VERSION 3.21)
project(hip_shim VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Only 64-bit builds supported
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Only 64-bit builds are supported")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find HIP SDK
if(NOT DEFINED ENV{HIP_PATH})
    message(FATAL_ERROR "HIP_PATH not set. Install AMD HIP SDK first.")
endif()

set(HIP_PATH "$ENV{HIP_PATH}")
file(TO_CMAKE_PATH "${HIP_PATH}" HIP_PATH)
set(HIP_INCLUDE_DIR "${HIP_PATH}/include")
set(HIP_LIB_DIR "${HIP_PATH}/lib")

if(MSVC)
    add_compile_options(/W4 /EHsc /MP)
    add_compile_definitions(WIN32_LEAN_AND_MEAN NOMINMAX _CRT_SECURE_NO_WARNINGS __HIP_PLATFORM_AMD__)
endif()

# Common static library
add_library(hip_shim_common STATIC
    src/hip_wrapper/memory_manager.cpp
    src/hip_wrapper/device_spoof.cpp
    src/rocblas_wrapper/dtype_converter.cpp
)
target_include_directories(hip_shim_common PUBLIC include ${HIP_INCLUDE_DIR})

# amdhip64.dll wrapper
add_library(amdhip64 SHARED
    src/hip_wrapper/dllmain.cpp
    src/hip_wrapper/hip_intercept.cpp
    src/hip_wrapper/exports.def
)
target_include_directories(amdhip64 PRIVATE include ${HIP_INCLUDE_DIR})
target_link_libraries(amdhip64 PRIVATE hip_shim_common)
set_target_properties(amdhip64 PROPERTIES OUTPUT_NAME "amdhip64" PREFIX "")

# rocblas.dll wrapper
add_library(rocblas SHARED
    src/rocblas_wrapper/dllmain.cpp
    src/rocblas_wrapper/rocblas_intercept.cpp
    src/rocblas_wrapper/exports.def
)
target_include_directories(rocblas PRIVATE include ${HIP_INCLUDE_DIR})
target_link_libraries(rocblas PRIVATE hip_shim_common)
set_target_properties(rocblas PROPERTIES OUTPUT_NAME "rocblas" PREFIX "")

# Test executable
add_subdirectory(test)
