#ifndef DEVICE_SPOOF_HPP
#define DEVICE_SPOOF_HPP

#include "hip_shim_common.hpp"
#include "hip_types_fwd.hpp"

namespace hip_shim
{

    class DeviceSpoof
    {
    public:
        static DeviceSpoof &instance()
        {
            static DeviceSpoof inst;
            return inst;
        }

        bool initialize(int device_id = 0);
        int get_spoofed_properties(void *props, int device_id);  // Returns int (hipError_t typedef)
        int get_spoofed_attribute(int *value, hipDeviceAttribute_t attr, int device_id);
        int get_spoofed_mem_info(size_t *free, size_t *total);

        size_t get_real_vram() const { return real_vram_; }

    private:
        DeviceSpoof() : real_vram_(0), extra_memory_(0), initialized_(false) {}

        size_t real_vram_;
        size_t extra_memory_;
        bool initialized_;
    };

} // namespace hip_shim
#endif
