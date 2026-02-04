#ifndef FUNCTION_TABLE_HPP
#define FUNCTION_TABLE_HPP

#include "hip_types_fwd.hpp"
#include <Windows.h>
#include "hip_shim_common.hpp"

namespace hip_shim
{

    // Function pointer types
    typedef hipError_t (*pfn_hipGetDeviceCount)(int *);
    typedef hipError_t (*pfn_hipGetDevice)(int *);
    typedef hipError_t (*pfn_hipSetDevice)(int);
    typedef hipError_t (*pfn_hipGetDeviceProperties)(hipDeviceProp_t *, int);
    typedef hipError_t (*pfn_hipDeviceGetAttribute)(int *, hipDeviceAttribute_t, int);
    typedef hipError_t (*pfn_hipDeviceSynchronize)(void);
    typedef hipError_t (*pfn_hipDeviceReset)(void);
    typedef hipError_t (*pfn_hipMalloc)(void **, size_t);
    typedef hipError_t (*pfn_hipFree)(void *);
    typedef hipError_t (*pfn_hipMemcpy)(void *, const void *, size_t, hipMemcpyKind);
    typedef hipError_t (*pfn_hipMemcpyAsync)(void *, const void *, size_t, hipMemcpyKind, hipStream_t);
    typedef hipError_t (*pfn_hipMemset)(void *, int, size_t);
    typedef hipError_t (*pfn_hipMemsetAsync)(void *, int, size_t, hipStream_t);
    typedef hipError_t (*pfn_hipMemGetInfo)(size_t *, size_t *);
    typedef hipError_t (*pfn_hipHostMalloc)(void **, size_t, unsigned int);
    typedef hipError_t (*pfn_hipHostFree)(void *);
    typedef hipError_t (*pfn_hipHostRegister)(void *, size_t, unsigned int);
    typedef hipError_t (*pfn_hipHostUnregister)(void *);
    typedef hipError_t (*pfn_hipHostGetDevicePointer)(void **, void *, unsigned int);
    typedef hipError_t (*pfn_hipMallocManaged)(void **, size_t, unsigned int);
    typedef hipError_t (*pfn_hipStreamCreate)(hipStream_t *);
    typedef hipError_t (*pfn_hipStreamDestroy)(hipStream_t);
    typedef hipError_t (*pfn_hipStreamSynchronize)(hipStream_t);
    typedef hipError_t (*pfn_hipEventCreate)(hipEvent_t *);
    typedef hipError_t (*pfn_hipEventDestroy)(hipEvent_t);
    typedef hipError_t (*pfn_hipEventRecord)(hipEvent_t, hipStream_t);
    typedef hipError_t (*pfn_hipEventSynchronize)(hipEvent_t);
    typedef hipError_t (*pfn_hipGetLastError)(void);
    typedef const char *(*pfn_hipGetErrorString)(hipError_t);
    typedef hipError_t (*pfn_hipModuleLoad)(hipModule_t *, const char *);
    typedef hipError_t (*pfn_hipModuleUnload)(hipModule_t);
    typedef hipError_t (*pfn_hipLaunchKernel)(const void *, dim3, dim3, void **, size_t, hipStream_t);
    typedef hipError_t (*pfn_hipPointerGetAttributes)(hipPointerAttribute_t *, const void *);

    struct HipFunctionTable
    {
        pfn_hipGetDeviceCount hipGetDeviceCount = nullptr;
        pfn_hipGetDevice hipGetDevice = nullptr;
        pfn_hipSetDevice hipSetDevice = nullptr;
        pfn_hipGetDeviceProperties hipGetDeviceProperties = nullptr;
        pfn_hipDeviceGetAttribute hipDeviceGetAttribute = nullptr;
        pfn_hipDeviceSynchronize hipDeviceSynchronize = nullptr;
        pfn_hipDeviceReset hipDeviceReset = nullptr;
        pfn_hipMalloc hipMalloc = nullptr;
        pfn_hipFree hipFree = nullptr;
        pfn_hipMemcpy hipMemcpy = nullptr;
        pfn_hipMemcpyAsync hipMemcpyAsync = nullptr;
        pfn_hipMemset hipMemset = nullptr;
        pfn_hipMemsetAsync hipMemsetAsync = nullptr;
        pfn_hipMemGetInfo hipMemGetInfo = nullptr;
        pfn_hipHostMalloc hipHostMalloc = nullptr;
        pfn_hipHostFree hipHostFree = nullptr;
        pfn_hipHostRegister hipHostRegister = nullptr;
        pfn_hipHostUnregister hipHostUnregister = nullptr;
        pfn_hipHostGetDevicePointer hipHostGetDevicePointer = nullptr;
        pfn_hipMallocManaged hipMallocManaged = nullptr;
        pfn_hipStreamCreate hipStreamCreate = nullptr;
        pfn_hipStreamDestroy hipStreamDestroy = nullptr;
        pfn_hipStreamSynchronize hipStreamSynchronize = nullptr;
        pfn_hipEventCreate hipEventCreate = nullptr;
        pfn_hipEventDestroy hipEventDestroy = nullptr;
        pfn_hipEventRecord hipEventRecord = nullptr;
        pfn_hipEventSynchronize hipEventSynchronize = nullptr;
        pfn_hipGetLastError hipGetLastError = nullptr;
        pfn_hipGetErrorString hipGetErrorString = nullptr;
        pfn_hipModuleLoad hipModuleLoad = nullptr;
        pfn_hipModuleUnload hipModuleUnload = nullptr;
        pfn_hipLaunchKernel hipLaunchKernel = nullptr;
        pfn_hipPointerGetAttributes hipPointerGetAttributes = nullptr;

        HMODULE hModule = nullptr;

        bool load(const char *dll_name)
        {
            hModule = LoadLibraryA(dll_name);
            if (!hModule)
            {
                LOG_ERROR("Failed to load %s: %lu", dll_name, GetLastError());
                return false;
            }

#define LOAD_FUNC(name)                                                  \
    name = reinterpret_cast<pfn_##name>(GetProcAddress(hModule, #name)); \
    if (!name)                                                           \
        LOG_WARNING("Function " #name " not found in %s", dll_name);

            LOAD_FUNC(hipGetDeviceCount);
            LOAD_FUNC(hipGetDevice);
            LOAD_FUNC(hipSetDevice);
            LOAD_FUNC(hipGetDeviceProperties);
            LOAD_FUNC(hipDeviceGetAttribute);
            LOAD_FUNC(hipDeviceSynchronize);
            LOAD_FUNC(hipDeviceReset);
            LOAD_FUNC(hipMalloc);
            LOAD_FUNC(hipFree);
            LOAD_FUNC(hipMemcpy);
            LOAD_FUNC(hipMemcpyAsync);
            LOAD_FUNC(hipMemset);
            LOAD_FUNC(hipMemsetAsync);
            LOAD_FUNC(hipMemGetInfo);
            LOAD_FUNC(hipHostMalloc);
            LOAD_FUNC(hipHostFree);
            LOAD_FUNC(hipHostRegister);
            LOAD_FUNC(hipHostUnregister);
            LOAD_FUNC(hipHostGetDevicePointer);
            LOAD_FUNC(hipMallocManaged);
            LOAD_FUNC(hipStreamCreate);
            LOAD_FUNC(hipStreamDestroy);
            LOAD_FUNC(hipStreamSynchronize);
            LOAD_FUNC(hipEventCreate);
            LOAD_FUNC(hipEventDestroy);
            LOAD_FUNC(hipEventRecord);
            LOAD_FUNC(hipEventSynchronize);
            LOAD_FUNC(hipGetLastError);
            LOAD_FUNC(hipGetErrorString);
            LOAD_FUNC(hipModuleLoad);
            LOAD_FUNC(hipModuleUnload);
            LOAD_FUNC(hipLaunchKernel);
            LOAD_FUNC(hipPointerGetAttributes);

#undef LOAD_FUNC

            LOG_INFO("Successfully loaded %s", dll_name);
            return true;
        }

        void unload()
        {
            if (hModule)
            {
                FreeLibrary(hModule);
                hModule = nullptr;
            }
        }
    };

    extern HipFunctionTable g_realHip;

} // namespace hip_shim
#endif
