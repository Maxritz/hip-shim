#ifndef MEMORY_MANAGER_HPP
#define MEMORY_MANAGER_HPP

#include "hip_shim_common.hpp"
#include "hip_types_fwd.hpp"
#include <unordered_map>
#include <mutex>

namespace hip_shim
{

    enum class MemoryLocation
    {
        VRAM,
        SystemRAM,
        Unknown
    };

    struct AllocationRecord
    {
        void *user_ptr;
        void *device_ptr;
        size_t size;
        size_t alignment;
        MemoryLocation location;
    };

    struct MemoryStats
    {
        size_t vram_total;
        size_t vram_used;
        size_t system_ram_used;
        size_t peak_allocated;
        size_t num_spillovers;
    };

    class MemoryManager
    {
    public:
        static MemoryManager &instance()
        {
            static MemoryManager inst;
            return inst;
        }

        bool initialize(size_t vram_size);
        void shutdown();

        hipError_t allocate(void **ptr, size_t size);
        hipError_t free(void *ptr);
        hipError_t memcpy(void *dst, const void *src, size_t size, hipMemcpyKind kind);
        hipError_t memcpy_async(void *dst, const void *src, size_t size, hipMemcpyKind kind, hipStream_t stream);
        hipError_t memset(void *ptr, int value, size_t size);
        hipError_t memset_async(void *ptr, int value, size_t size, hipStream_t stream);
        hipError_t get_mem_info(size_t *free, size_t *total);

        bool is_managed_pointer(void *ptr);
        MemoryStats get_stats() const;

    private:
        MemoryManager() : vram_total_(0), vram_used_(0), system_ram_used_(0),
                          peak_allocated_(0), num_spillovers_(0), initialized_(false) {}
        ~MemoryManager() { shutdown(); }

        bool try_allocate_vram(void **ptr, size_t size);
        bool try_allocate_system_ram(void **ptr, void **device_ptr, size_t size);

        size_t vram_total_;
        size_t vram_used_;
        size_t system_ram_used_;
        size_t peak_allocated_;
        size_t num_spillovers_;
        std::unordered_map<void *, AllocationRecord> allocations_;
        mutable std::mutex mutex_;
        bool initialized_;
    };

} // namespace hip_shim
#endif
