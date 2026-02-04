#include "hip_shim_common.hpp"
#include <Windows.h>

// This is a stub implementation. ROCBlas has hundreds of functions.
// For now, we just provide a minimal passthrough structure.
// Real implementation would intercept specific BLAS functions and convert FP8 to INT8.

extern "C"
{

    // Placeholder exports - add specific functions as needed
    __declspec(dllexport) int rocblas_create_handle(void **handle)
    {
        LOG_DEBUG("rocblas_create_handle called (stub)");
        return 0; // Success
    }

    __declspec(dllexport) int rocblas_destroy_handle(void *handle)
    {
        LOG_DEBUG("rocblas_destroy_handle called (stub)");
        return 0; // Success
    }

} // extern "C"
