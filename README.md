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

## Troubleshooting

Check `hip_shim.log` in the application directory for detailed logging.
