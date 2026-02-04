#include "memory_manager.hpp"
#include "function_table.hpp"
#include <malloc.h>
#include <algorithm>
#include <chrono>

namespace hip_shim
{

    extern HipFunctionTable g_realHip;

    bool MemoryManager::initialize(size_t vram_size)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (initialized_)
            return true;

        vram_total_ = vram_size;
        size_t headroom = (vram_size * config::VRAM_HEADROOM_PERCENT) / 100;
        vram_total_ = (vram_size > headroom) ? (vram_size - headroom) : vram_size;

        LOG_INFO("MemoryManager initialized: VRAM=%zu MB, Spillover=%s",
                 vram_total_ / (1024 * 1024),
                 config::ENABLE_MEMORY_SPILLOVER ? "enabled" : "disabled");

        initialized_ = true;
        return true;
    }

    void MemoryManager::shutdown()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_)
            return;

        LOG_INFO("MemoryManager shutdown: %zu allocations remaining", allocations_.size());

        for (auto &[ptr, rec] : allocations_)
        {
            if (rec.location == MemoryLocation::VRAM && g_realHip.hipFree)
            {
                g_realHip.hipFree(rec.device_ptr);
            }
            else if (rec.location == MemoryLocation::SystemRAM)
            {
                if (g_realHip.hipHostUnregister)
                    g_realHip.hipHostUnregister(rec.user_ptr);
                _aligned_free(rec.user_ptr);
            }
        }
        allocations_.clear();
        initialized_ = false;
    }

    hipError_t MemoryManager::allocate(void **ptr, size_t size)
    {
        if (!ptr || size == 0)
            return hipErrorInvalidValue;
        if (!initialized_)
            return hipErrorNotInitialized;

        std::lock_guard<std::mutex> lock(mutex_);

        size_t aligned_size = (size + config::ALLOCATION_ALIGNMENT - 1) & ~(config::ALLOCATION_ALIGNMENT - 1);

        // Try VRAM first
        if (vram_used_ + aligned_size <= vram_total_)
        {
            void *vram_ptr = nullptr;
            if (try_allocate_vram(&vram_ptr, aligned_size))
            {
                vram_used_ += aligned_size;
                *ptr = vram_ptr;

                AllocationRecord rec;
                rec.user_ptr = vram_ptr;
                rec.device_ptr = vram_ptr;
                rec.size = aligned_size;
                rec.alignment = config::ALLOCATION_ALIGNMENT;
                rec.location = MemoryLocation::VRAM;
                allocations_[vram_ptr] = rec;

                size_t total_used = vram_used_ + system_ram_used_;
                if (total_used > peak_allocated_)
                    peak_allocated_ = total_used;

                LOG_DEBUG("Allocated %zu bytes in VRAM (total: %zu/%zu MB)",
                          aligned_size, vram_used_ / (1024 * 1024), vram_total_ / (1024 * 1024));
                return hipSuccess;
            }
        }

        // Fallback to system RAM if spillover enabled
        if (config::ENABLE_MEMORY_SPILLOVER)
        {
            void *host_ptr = nullptr;
            void *device_ptr = nullptr;
            if (try_allocate_system_ram(&host_ptr, &device_ptr, aligned_size))
            {
                system_ram_used_ += aligned_size;
                num_spillovers_++;
                *ptr = device_ptr;

                AllocationRecord rec;
                rec.user_ptr = host_ptr;
                rec.device_ptr = device_ptr;
                rec.size = aligned_size;
                rec.alignment = config::ALLOCATION_ALIGNMENT;
                rec.location = MemoryLocation::SystemRAM;
                allocations_[device_ptr] = rec;

                size_t total_used = vram_used_ + system_ram_used_;
                if (total_used > peak_allocated_)
                    peak_allocated_ = total_used;

                LOG_WARNING("Spilled %zu bytes to system RAM (total spillover: %zu MB)",
                            aligned_size, system_ram_used_ / (1024 * 1024));
                return hipSuccess;
            }
        }

        LOG_ERROR("Out of memory: requested %zu bytes, VRAM used %zu/%zu MB, SysRAM used %zu MB",
                  aligned_size, vram_used_ / (1024 * 1024), vram_total_ / (1024 * 1024),
                  system_ram_used_ / (1024 * 1024));
        return hipErrorOutOfMemory;
    }

    hipError_t MemoryManager::free(void *ptr)
    {
        if (!ptr)
            return hipSuccess;

        std::lock_guard<std::mutex> lock(mutex_);

        auto it = allocations_.find(ptr);
        if (it == allocations_.end())
        {
            LOG_WARNING("Attempted to free untracked pointer %p", ptr);
            return g_realHip.hipFree ? g_realHip.hipFree(ptr) : hipErrorInvalidValue;
        }

        AllocationRecord &rec = it->second;
        if (rec.location == MemoryLocation::VRAM)
        {
            vram_used_ -= rec.size;
            if (g_realHip.hipFree)
                g_realHip.hipFree(rec.device_ptr);
        }
        else if (rec.location == MemoryLocation::SystemRAM)
        {
            system_ram_used_ -= rec.size;
            if (g_realHip.hipHostUnregister)
                g_realHip.hipHostUnregister(rec.user_ptr);
            _aligned_free(rec.user_ptr);
        }

        allocations_.erase(it);
        LOG_DEBUG("Freed %zu bytes from %s", rec.size,
                  rec.location == MemoryLocation::VRAM ? "VRAM" : "SystemRAM");
        return hipSuccess;
    }

    hipError_t MemoryManager::memcpy(void *dst, const void *src, size_t size, hipMemcpyKind kind)
    {
        if (g_realHip.hipMemcpy)
            return g_realHip.hipMemcpy(dst, src, size, kind);
        return hipErrorNotSupported;
    }

    hipError_t MemoryManager::memcpy_async(void *dst, const void *src, size_t size, hipMemcpyKind kind, hipStream_t stream)
    {
        if (g_realHip.hipMemcpyAsync)
            return g_realHip.hipMemcpyAsync(dst, src, size, kind, stream);
        return hipErrorNotSupported;
    }

    hipError_t MemoryManager::memset(void *ptr, int value, size_t size)
    {
        if (g_realHip.hipMemset)
            return g_realHip.hipMemset(ptr, value, size);
        return hipErrorNotSupported;
    }

    hipError_t MemoryManager::memset_async(void *ptr, int value, size_t size, hipStream_t stream)
    {
        if (g_realHip.hipMemsetAsync)
            return g_realHip.hipMemsetAsync(ptr, value, size, stream);
        return hipErrorNotSupported;
    }

    hipError_t MemoryManager::get_mem_info(size_t *free, size_t *total)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (free)
            *free = vram_total_ - vram_used_;
        if (total)
            *total = vram_total_;
        return hipSuccess;
    }

    bool MemoryManager::is_managed_pointer(void *ptr)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return allocations_.find(ptr) != allocations_.end();
    }

    MemoryStats MemoryManager::get_stats() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return {vram_total_, vram_used_, system_ram_used_, peak_allocated_, num_spillovers_};
    }

    bool MemoryManager::try_allocate_vram(void **ptr, size_t size)
    {
        if (!g_realHip.hipMalloc)
            return false;
        *ptr = nullptr;
        return g_realHip.hipMalloc(ptr, size) == hipSuccess;
    }

    bool MemoryManager::try_allocate_system_ram(void **ptr, void **device_ptr, size_t size)
    {
        size_t avail = get_available_system_ram();
        size_t reserve = config::MIN_SYSTEM_RAM_RESERVE_MB * 1024 * 1024;

        if (avail < size + reserve)
        {
            LOG_WARNING("Insufficient system RAM: need %zu MB, available %zu MB",
                        size / (1024 * 1024), avail / (1024 * 1024));
            return false;
        }

        *ptr = _aligned_malloc(size, config::ALLOCATION_ALIGNMENT);
        if (!*ptr)
        {
            LOG_ERROR("System RAM allocation failed: %zu bytes", size);
            return false;
        }

        if (!g_realHip.hipHostRegister || !g_realHip.hipHostGetDevicePointer)
        {
            _aligned_free(*ptr);
            *ptr = nullptr;
            return false;
        }

        unsigned int flags = hipHostRegisterMapped | hipHostRegisterPortable;
        if (g_realHip.hipHostRegister(*ptr, size, flags) != hipSuccess)
        {
            _aligned_free(*ptr);
            *ptr = nullptr;
            return false;
        }

        if (g_realHip.hipHostGetDevicePointer(device_ptr, *ptr, 0) != hipSuccess)
        {
            g_realHip.hipHostUnregister(*ptr);
            _aligned_free(*ptr);
            *ptr = nullptr;
            return false;
        }

        return true;
    }

} // namespace hip_shim
