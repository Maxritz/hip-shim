#include "dtype_converter.hpp"

namespace hip_shim
{

    bool DTypeConverter::convert_fp8_to_int8(const void *src, void *dst, size_t count)
    {
        // TODO: Implement FP8 to INT8 conversion when needed
        LOG_WARNING("FP8 conversion not yet implemented");
        return false;
    }

} // namespace hip_shim
