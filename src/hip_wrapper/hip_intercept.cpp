#include "hip_shim_common.hpp"
#include "function_table.hpp"
#include "memory_manager.hpp"
#include "device_spoof.hpp"

using namespace hip_shim;

// Initialization check helper
static inline bool check_initialized()
{
    if (!g_realHip.hModule)
    {
        LOG_ERROR("HIP-Shim not initialized");
        return false;
    }
    return true;
}

// Device Management
extern "C" __declspec(dllexport) hipError_t hipGetDeviceCount(int *count)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipGetDeviceCount ? g_realHip.hipGetDeviceCount(count) : hipErrorNotSupported;
}

extern "C" __declspec(dllexport) hipError_t hipGetDevice(int *deviceId)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipGetDevice ? g_realHip.hipGetDevice(deviceId) : hipErrorNotSupported;
}

extern "C" __declspec(dllexport) hipError_t hipSetDevice(int deviceId)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipSetDevice ? g_realHip.hipSetDevice(deviceId) : hipErrorNotSupported;
}

extern "C" __declspec(dllexport) hipError_t hipGetDeviceProperties(hipDeviceProp_t *props, int deviceId)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return DeviceSpoof::instance().get_spoofed_properties(props, deviceId);
}

// Versioned API - some apps call this instead
extern "C" __declspec(dllexport) hipError_t hipGetDevicePropertiesR0600(hipDeviceProp_t *props, int deviceId)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return DeviceSpoof::instance().get_spoofed_properties(props, deviceId);
}

extern "C" __declspec(dllexport) hipError_t hipDeviceGetAttribute(int *value, hipDeviceAttribute_t attr, int deviceId)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return DeviceSpoof::instance().get_spoofed_attribute(value, attr, deviceId);
}

extern "C" __declspec(dllexport) hipError_t hipDeviceSynchronize(void)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipDeviceSynchronize ? g_realHip.hipDeviceSynchronize() : hipErrorNotSupported;
}

extern "C" __declspec(dllexport) hipError_t hipDeviceReset(void)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipDeviceReset ? g_realHip.hipDeviceReset() : hipErrorNotSupported;
}

// Memory Management
extern "C" __declspec(dllexport) hipError_t hipMalloc(void **ptr, size_t size)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return MemoryManager::instance().allocate(ptr, size);
}

extern "C" __declspec(dllexport) hipError_t hipFree(void *ptr)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return MemoryManager::instance().free(ptr);
}

extern "C" __declspec(dllexport) hipError_t hipMemcpy(void *dst, const void *src, size_t sizeBytes, hipMemcpyKind kind)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return MemoryManager::instance().memcpy(dst, src, sizeBytes, kind);
}

extern "C" __declspec(dllexport) hipError_t hipMemcpyAsync(void *dst, const void *src, size_t sizeBytes, hipMemcpyKind kind, hipStream_t stream)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return MemoryManager::instance().memcpy_async(dst, src, sizeBytes, kind, stream);
}

extern "C" __declspec(dllexport) hipError_t hipMemset(void *dst, int value, size_t sizeBytes)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return MemoryManager::instance().memset(dst, value, sizeBytes);
}

extern "C" __declspec(dllexport) hipError_t hipMemsetAsync(void *dst, int value, size_t sizeBytes, hipStream_t stream)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return MemoryManager::instance().memset_async(dst, value, sizeBytes, stream);
}

extern "C" __declspec(dllexport) hipError_t hipMemGetInfo(size_t *free, size_t *total)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return DeviceSpoof::instance().get_spoofed_mem_info(free, total);
}

extern "C" __declspec(dllexport) hipError_t hipHostMalloc(void **ptr, size_t size, unsigned int flags)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipHostMalloc ? g_realHip.hipHostMalloc(ptr, size, flags) : hipErrorNotSupported;
}

extern "C" __declspec(dllexport) hipError_t hipHostFree(void *ptr)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipHostFree ? g_realHip.hipHostFree(ptr) : hipErrorNotSupported;
}

extern "C" __declspec(dllexport) hipError_t hipMallocManaged(void **devPtr, size_t size, unsigned int flags)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipMallocManaged ? g_realHip.hipMallocManaged(devPtr, size, flags) : hipErrorNotSupported;
}

extern "C" __declspec(dllexport) hipError_t hipPointerGetAttributes(hipPointerAttribute_t *attributes, const void *ptr)
{
    if (!check_initialized())
        return hipErrorNotInitialized;

    // Check if we manage this pointer
    if (MemoryManager::instance().is_managed_pointer((void *)ptr))
    {
        // Fill in basic attributes for our managed pointer
        if (attributes)
        {
            memset(attributes, 0, sizeof(hipPointerAttribute_t));
            attributes->type = hipMemoryTypeDevice;
            attributes->device = 0;
            attributes->devicePointer = (void *)ptr;
            attributes->hostPointer = (void *)ptr;
        }
        return hipSuccess;
    }

    // Fall back to real implementation
    if (g_realHip.hipPointerGetAttributes)
    {
        return g_realHip.hipPointerGetAttributes(attributes, ptr);
    }
    return hipErrorNotSupported;
}

// Stream Management
extern "C" __declspec(dllexport) hipError_t hipStreamCreate(hipStream_t *stream)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipStreamCreate ? g_realHip.hipStreamCreate(stream) : hipErrorNotSupported;
}

extern "C" __declspec(dllexport) hipError_t hipStreamDestroy(hipStream_t stream)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipStreamDestroy ? g_realHip.hipStreamDestroy(stream) : hipErrorNotSupported;
}

extern "C" __declspec(dllexport) hipError_t hipStreamSynchronize(hipStream_t stream)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipStreamSynchronize ? g_realHip.hipStreamSynchronize(stream) : hipErrorNotSupported;
}

// Event Management
extern "C" __declspec(dllexport) hipError_t hipEventCreate(hipEvent_t *event)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipEventCreate ? g_realHip.hipEventCreate(event) : hipErrorNotSupported;
}

extern "C" __declspec(dllexport) hipError_t hipEventDestroy(hipEvent_t event)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipEventDestroy ? g_realHip.hipEventDestroy(event) : hipErrorNotSupported;
}

extern "C" __declspec(dllexport) hipError_t hipEventRecord(hipEvent_t event, hipStream_t stream)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipEventRecord ? g_realHip.hipEventRecord(event, stream) : hipErrorNotSupported;
}

extern "C" __declspec(dllexport) hipError_t hipEventSynchronize(hipEvent_t event)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipEventSynchronize ? g_realHip.hipEventSynchronize(event) : hipErrorNotSupported;
}

// Error Handling
extern "C" __declspec(dllexport) hipError_t hipGetLastError(void)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipGetLastError ? g_realHip.hipGetLastError() : hipSuccess;
}

extern "C" __declspec(dllexport) const char *hipGetErrorString(hipError_t hipError)
{
    return g_realHip.hipGetErrorString ? g_realHip.hipGetErrorString(hipError) : "Unknown";
}

// Kernel Launch
extern "C" __declspec(dllexport) hipError_t hipLaunchKernel(const void *function_address, dim3 numBlocks, dim3 dimBlocks, void **args, size_t sharedMemBytes, hipStream_t stream)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipLaunchKernel ? g_realHip.hipLaunchKernel(function_address, numBlocks, dimBlocks, args, sharedMemBytes, stream) : hipErrorNotSupported;
}

// Module Management
extern "C" __declspec(dllexport) hipError_t hipModuleLoad(hipModule_t *module, const char *fname)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipModuleLoad ? g_realHip.hipModuleLoad(module, fname) : hipErrorNotSupported;
}

extern "C" __declspec(dllexport) hipError_t hipModuleUnload(hipModule_t module)
{
    if (!check_initialized())
        return hipErrorNotInitialized;
    return g_realHip.hipModuleUnload ? g_realHip.hipModuleUnload(module) : hipErrorNotSupported;
}
