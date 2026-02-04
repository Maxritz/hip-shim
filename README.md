# HIP-Shim: ROCm Compatibility Layer for Unsupported AMD GPUs

## Overview

Wrapper DLL that enables unsupported AMD GPUs (RX 6700 XT / gfx1031) to work with ROCm 7.x applications.

**Target Hardware**: AMD RX 6700 XT (gfx1031, 12GB VRAM, RDNA2)

**Features**:

- Device property spoofing (gfx1031 → gfx1100)
- VRAM + System RAM unified memory management
- FP8 → INT8 conversion (future)
- Transparent passthrough to real HIP runtime

## Quick Start

```powershell
# 1. Set HIP_PATH
$env:HIP_PATH = "C:\Program Files\AMD\ROCm\7.1"

# 2. Build
.\scripts\build.ps1

# 3. Install to LM Studio
.\scripts\install.ps1 -TargetPath "$env:LOCALAPPDATA\LM Studio\resources\app\bin" -Backup

# 4. Run LM Studio and check hip_shim.log
```

## Requirements

- Windows 10/11 x64
- Visual Studio 2022 (or Build Tools)
- AMD HIP SDK 7.1+ installed
- CMake 3.21+

## How It Works

1. **Device Spoofing**: Reports gfx1100 instead of gfx1031
2. **Memory Expansion**: Combines VRAM + System RAM for larger models
3. **Transparent Proxy**: All other HIP calls pass through to real runtime

## Build

```powershell
.\scripts\build.ps1 -BuildType Release
```

## Install

```powershell
# Backup original DLLs first
.\scripts\install.ps1 -TargetPath "path\to\app\bin" -Backup

# Restore originals
.\scripts\install.ps1 -TargetPath "path\to\app\bin" -Restore
```

## Test Results

**Test System:**
- CPU: AMD Ryzen 9 5900XT 16-Core
- GPU: AMD Radeon RX 9070 XT (gfx1031, 16GB VRAM)
- RAM: 96 GB
- OS: Windows 11
- ROCm: 7.1

**Spoofed Configuration:**
- Architecture: gfx1100 (RDNA3)
- Total Memory: **117.91 GB** (24GB VRAM + 96GB system RAM)
- Compute Units: 60
- Clock: 2500 MHz / 16000 MHz (core/memory)

**Test Results:** ✅ **ALL PASSED**

| Test Category | Status | Details |
|--------------|--------|---------|
| Device Detection | ✅ PASS | Successfully enumerated 1 device |
| Device Properties | ✅ PASS | Correctly spoofed to gfx1100 with 117.91 GB |
| Memory Allocation | ✅ PASS | Small (100MB) and large (1GB) allocations successful |
| Memory Operations | ✅ PASS | Host↔Device copies and verification working |
| Stream Operations | ✅ PASS | Create, sync, destroy working correctly |
| Event Operations | ✅ PASS | Create, record, sync, destroy working correctly |
| Memory Tracking | ✅ PASS | Proper allocation/deallocation tracking, 0 leaks |

See [TEST_RESULTS.md](TEST_RESULTS.md) for detailed logs and performance metrics.

## Configuration

The shim can be customized at runtime using environment variables (no rebuild needed):

```powershell
# DLL loading
$env:HIP_SHIM_REAL_DLL = "amdhip64_7_real.dll"      # Real HIP DLL name
$env:HIP_SHIM_ROCBLAS_DLL = "rocblas_real.dll"      # Real ROCBlas DLL name

# Logging
$env:HIP_SHIM_LOG_FILE = "hip_shim.log"              # Log file path
$env:HIP_SHIM_DEBUG = "1"                            # Enable debug logging (1/0)

# Memory
$env:HIP_SHIM_VRAM_HEADROOM = "10"                   # VRAM reserve percentage
```

See [CONFIGURATION.md](CONFIGURATION.md) for full documentation.

## Troubleshooting

Check `hip_shim.log` in the application directory for detailed logging.
