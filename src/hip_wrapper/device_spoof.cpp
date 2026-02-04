// Include REAL HIP headers FIRST to prevent forward declarations
#include <hip/hip_runtime_api.h>

#include "device_spoof.hpp"
#include "function_table.hpp"
#include <algorithm>
#include <cstring>

namespace hip_shim
{

    extern HipFunctionTable g_realHip;

    bool DeviceSpoof::initialize(int device_id)
    {
        if (initialized_)
            return true;

        if (!g_realHip.hipMemGetInfo)
            return false;

        size_t free_mem, total_mem;
        if (g_realHip.hipMemGetInfo(&free_mem, &total_mem) == hipSuccess)
        {
            real_vram_ = total_mem;
            extra_memory_ = get_total_system_ram() - (config::MIN_SYSTEM_RAM_RESERVE_MB * 1024 * 1024);
            LOG_INFO("DeviceSpoof initialized: Real VRAM=%zu MB, Extra RAM=%zu MB",
                     real_vram_ / (1024 * 1024), extra_memory_ / (1024 * 1024));
            initialized_ = true;
            return true;
        }
        return false;
    }

    int DeviceSpoof::get_spoofed_properties(void *props, int device_id)
    {
        // Cast to real HIP struct type (hipDeviceProp_tR0600)
        auto *real_props = reinterpret_cast<hipDeviceProp_tR0600 *>(props);
        
        if (!real_props)
            return static_cast<int>(hipErrorInvalidValue);
        if (!initialized_ && !initialize(device_id))
            return hipErrorNotInitialized;

        LOG_DEBUG("get_spoofed_properties called for device %d", device_id);

        // Zero out the structure first
        memset(real_props, 0, sizeof(hipDeviceProp_tR0600));

        // Try to get real properties (may fail or return zeros for gfx1031)
        hipError_t err = hipSuccess;
        if (g_realHip.hipGetDeviceProperties)
        {
            err = g_realHip.hipGetDeviceProperties(real_props, device_id);
            LOG_DEBUG("Real hipGetDeviceProperties returned: %d, mem=%zu, CUs=%d",
                      err, real_props->totalGlobalMem, real_props->multiProcessorCount);
        }

        // Always populate all fields ourselves for gfx1031 compatibility
        // Device name - keep real if valid, otherwise set to known model
        if (real_props->name[0] == '\0' || strstr(real_props->name, "9070") != nullptr)
        {
            strncpy(real_props->name, "AMD Radeon RX 9070 XT", sizeof(real_props->name) - 1);
            real_props->name[sizeof(real_props->name) - 1] = '\0';
        }

        // Spoof architecture to gfx1100 (RDNA3)
        strncpy(real_props->gcnArchName, "gfx1100", sizeof(real_props->gcnArchName) - 1);
        real_props->gcnArchName[sizeof(real_props->gcnArchName) - 1] = '\0';

        // Inflate VRAM from 16GB to 24GB, plus system RAM
        size_t inflated_vram = (size_t)(24 * 1024) * 1024 * 1024; // 24GB base
        real_props->totalGlobalMem = inflated_vram + extra_memory_;

        // Core compute properties
        real_props->multiProcessorCount = 60;   // Reasonable for RDNA3
        real_props->maxThreadsPerBlock = 1024;  // Standard AMD limit
        real_props->warpSize = 32;              // AMD wavefront size (can be 32 or 64, use 32 for compatibility)
        real_props->clockRate = 2500000;        // 2.5 GHz (KHz)
        real_props->memoryClockRate = 16000000; // 16 GHz GDDR6 (KHz)
        real_props->memoryBusWidth = 192;       // 192-bit bus width

        // Thread and grid dimensions
        real_props->maxThreadsDim[0] = 1024;
        real_props->maxThreadsDim[1] = 1024;
        real_props->maxThreadsDim[2] = 1024;
        real_props->maxGridSize[0] = 2147483647;
        real_props->maxGridSize[1] = 65536;
        real_props->maxGridSize[2] = 65536;

        // Memory and cache properties
        real_props->regsPerBlock = 65536;       // Register file size
        real_props->sharedMemPerBlock = 65536;  // 64KB LDS
        real_props->totalConstMem = 2147483647; // 2GB theoretical
        real_props->l2CacheSize = 4194304;      // 4MB L2 cache
        real_props->maxThreadsPerMultiProcessor = 2048;

        // Compute capability (RDNA3 ~ 10.3)
        real_props->major = 10;
        real_props->minor = 3;

        // Feature flags
        real_props->concurrentKernels = 1;
        real_props->canMapHostMemory = 1;
        real_props->cooperativeLaunch = 1;
        real_props->integrated = 0; // Discrete GPU
        real_props->computeMode = hipComputeModeDefault;
        real_props->pciDeviceID = 0x744C; // Example PCI ID
        real_props->pciBusID = 1;
        real_props->pciDomainID = 0;

        // Debug: print actual byte offsets and values
        LOG_DEBUG("Struct size: %zu bytes", sizeof(hipDeviceProp_tR0600));
        LOG_DEBUG("After spoofing - name[0]='%c', gcnArchName[0]='%c'", real_props->name[0], real_props->gcnArchName[0]);
        LOG_DEBUG("After spoofing - totalGlobalMem=%zu, multiProcessorCount=%d",
                  real_props->totalGlobalMem, real_props->multiProcessorCount);

        LOG_INFO("Spoofed properties: %s (gfx1100), %.2f GB VRAM, %d CUs, %d MHz, warp=%d",
                 real_props->name,
                 real_props->totalGlobalMem / (1024.0 * 1024.0 * 1024.0),
                 real_props->multiProcessorCount,
                 real_props->clockRate / 1000,
                 real_props->warpSize);

        return static_cast<int>(hipSuccess); // Always return success since we populated everything
    }

    int DeviceSpoof::get_spoofed_attribute(int *value, hipDeviceAttribute_t attr, int device_id)
    {
        if (!value)
            return static_cast<int>(hipErrorInvalidValue);
        if (!initialized_ && !initialize(device_id))
            return hipErrorNotInitialized;

        if (attr == hipDeviceAttributeMemoryPoolsSupported)
        {
            *value = 1;
            return static_cast<int>(hipSuccess);
        }

        return g_realHip.hipDeviceGetAttribute ? static_cast<int>(g_realHip.hipDeviceGetAttribute(value, attr, device_id)) : static_cast<int>(hipErrorNotSupported);
    }

    int DeviceSpoof::get_spoofed_mem_info(size_t *free, size_t *total)
    {
        if (!initialized_ && !initialize(0))
            return static_cast<int>(hipErrorNotInitialized);

        size_t real_free = 0, real_total = 0;
        hipError_t err = g_realHip.hipMemGetInfo ? g_realHip.hipMemGetInfo(&real_free, &real_total) : hipErrorNotSupported;

        if (err == hipSuccess)
        {
            size_t available_extra = std::min(extra_memory_, get_available_system_ram());
            size_t inflated_vram = (size_t)(24 * 1024) * 1024 * 1024; // 24GB base

            // Report inflated free and total
            if (free)
                *free = real_free + (inflated_vram - real_vram_) + available_extra;
            if (total)
                *total = inflated_vram + extra_memory_;

            LOG_DEBUG("Spoofed memory info: free=%zu MB, total=%zu MB (24GB base + system RAM)",
                      (*free) / (1024 * 1024), (*total) / (1024 * 1024));
        }

        return static_cast<int>(err);
    }

} // namespace hip_shim
