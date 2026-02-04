#ifndef DTYPE_CONVERTER_HPP
#define DTYPE_CONVERTER_HPP

#include "hip_shim_common.hpp"
#include <cstdint>

namespace hip_shim
{

    // Future: FP8 to INT8 conversion utilities
    class DTypeConverter
    {
    public:
        static DTypeConverter &instance()
        {
            static DTypeConverter inst;
            return inst;
        }

        // Placeholder for future FP8 support
        bool convert_fp8_to_int8(const void *src, void *dst, size_t count);

    private:
        DTypeConverter() {}
    };

} // namespace hip_shim
#endif
