# HIP Shim Configuration Guide

The HIP shim can be customized at runtime using environment variables. No recompilation needed!

## Environment Variables

### DLL Loading

**`HIP_SHIM_REAL_DLL`**  
Name of the real HIP DLL to load.  
- **Default:** `amdhip64_7_real.dll`
- **Example:** `set HIP_SHIM_REAL_DLL=amdhip64_custom.dll`

**`HIP_SHIM_ROCBLAS_DLL`**  
Name of the real ROCBlas DLL to load.  
- **Default:** `rocblas_real.dll`
- **Example:** `set HIP_SHIM_ROCBLAS_DLL=rocblas_custom.dll`

### Logging

**`HIP_SHIM_LOG_FILE`**  
Path to the log file.  
- **Default:** `hip_shim.log`
- **Example:** `set HIP_SHIM_LOG_FILE=C:\logs\hip_debug.log`

**`HIP_SHIM_DEBUG`**  
Enable/disable debug logging.  
- **Default:** `1` (enabled)
- **Values:** `1`, `true` (enabled) or `0`, `false` (disabled)
- **Example:** `set HIP_SHIM_DEBUG=0`

### Memory Management

**`HIP_SHIM_VRAM_HEADROOM`**  
Percentage of VRAM to reserve as headroom.  
- **Default:** `10` (10%)
- **Example:** `set HIP_SHIM_VRAM_HEADROOM=5`

## Usage Examples

### Windows CMD
```cmd
set HIP_SHIM_REAL_DLL=amdhip64_7_real.dll
set HIP_SHIM_LOG_FILE=debug.log
set HIP_SHIM_DEBUG=1
your_app.exe
```

### PowerShell
```powershell
$env:HIP_SHIM_REAL_DLL="amdhip64_7_real.dll"
$env:HIP_SHIM_LOG_FILE="debug.log"
$env:HIP_SHIM_DEBUG="1"
.\your_app.exe
```

### Permanent Configuration (Windows)
```cmd
setx HIP_SHIM_DEBUG 0
setx HIP_SHIM_LOG_FILE "C:\logs\hip.log"
```

## DLL Loading Priority

The shim searches for the real HIP DLL in this order:

1. **Custom name** from `HIP_SHIM_REAL_DLL` in the same directory as the shim
2. **ROCm installation** at `%HIP_PATH%\bin\amdhip64_7.dll`
3. **Fallback** at `%HIP_PATH%\bin\amdhip64.dll`

## Compile-Time Defaults

If environment variables are not set, these defaults are used (from `hip_shim_common.hpp`):

```cpp
constexpr size_t VRAM_HEADROOM_PERCENT = 10;
constexpr size_t MIN_SYSTEM_RAM_RESERVE_MB = 2048;
constexpr size_t ALLOCATION_ALIGNMENT = 256;
constexpr bool ENABLE_MEMORY_SPILLOVER = true;
constexpr bool ENABLE_DEBUG_LOGGING = true;
constexpr const char *REAL_HIP_DLL_NAME = "amdhip64_7_real.dll";
constexpr const char *REAL_ROCBLAS_DLL_NAME = "rocblas_real.dll";
constexpr const char *LOG_FILE_PATH = "hip_shim.log";
```

## Troubleshooting

### DLL Not Found
If you see "Failed to load ... : 126", the real DLL couldn't be found:
1. Check `HIP_SHIM_REAL_DLL` is correct
2. Verify the DLL exists in the same directory
3. Ensure `HIP_PATH` environment variable is set
4. Check the log file for details

### Debug Logging
Enable verbose logging to diagnose issues:
```cmd
set HIP_SHIM_DEBUG=1
set HIP_SHIM_LOG_FILE=debug.log
```

Then check `debug.log` for detailed operation logs.
