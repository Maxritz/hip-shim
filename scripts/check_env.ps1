Write-Host "========================================" -ForegroundColor Cyan
Write-Host "HIP-Shim Environment Check" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$allGood = $true

# Check HIP_PATH
Write-Host "`n[1] Checking HIP_PATH..." -ForegroundColor Yellow
if ($env:HIP_PATH) {
    Write-Host "  ✓ HIP_PATH = $env:HIP_PATH" -ForegroundColor Green
    if (Test-Path $env:HIP_PATH) {
        Write-Host "  ✓ Path exists" -ForegroundColor Green
    }
    else {
        Write-Host "  ✗ Path does not exist!" -ForegroundColor Red
        $allGood = $false
    }
}
else {
    Write-Host "  ✗ HIP_PATH not set" -ForegroundColor Red
    Write-Host "    Set with: `$env:HIP_PATH = 'C:\Program Files\AMD\ROCm\7.1'" -ForegroundColor Gray
    $allGood = $false
}

# Check for amdhip64.dll
Write-Host "`n[2] Checking for real HIP DLL..." -ForegroundColor Yellow
$hipDll = Join-Path $env:HIP_PATH "bin\amdhip64.dll"
if (Test-Path $hipDll) {
    Write-Host "  ✓ Found: $hipDll" -ForegroundColor Green
    $fileInfo = Get-Item $hipDll
    Write-Host "    Size: $([math]::Round($fileInfo.Length / 1MB, 2)) MB" -ForegroundColor Gray
}
else {
    Write-Host "  ✗ Not found: $hipDll" -ForegroundColor Red
    $allGood = $false
}

# Check for amdhip64_real.dll (for after installation)
$hipRealDll = Join-Path $env:HIP_PATH "bin\amdhip64_real.dll"
if (Test-Path $hipRealDll) {
    Write-Host "  ✓ Found renamed real DLL: amdhip64_real.dll" -ForegroundColor Green
    Write-Host "    (Ready for shim installation)" -ForegroundColor Gray
}
else {
    Write-Host "  ℹ Real DLL not yet renamed to amdhip64_real.dll" -ForegroundColor Yellow
    Write-Host "    (This is fine if you haven't installed the shim yet)" -ForegroundColor Gray
}

# Check for built shim DLLs
Write-Host "`n[3] Checking built shim DLLs..." -ForegroundColor Yellow
$projectRoot = Split-Path -Parent $PSScriptRoot
$shimDll = Join-Path $projectRoot "build\bin\Release\amdhip64.dll"
$rocblasDll = Join-Path $projectRoot "build\bin\Release\rocblas.dll"

if (Test-Path $shimDll) {
    $fileInfo = Get-Item $shimDll
    Write-Host "  ✓ amdhip64.dll ($([math]::Round($fileInfo.Length / 1KB, 1)) KB)" -ForegroundColor Green
}
else {
    Write-Host "  ✗ amdhip64.dll not built" -ForegroundColor Red
    Write-Host "    Run: .\scripts\build.ps1" -ForegroundColor Gray
    $allGood = $false
}

if (Test-Path $rocblasDll) {
    $fileInfo = Get-Item $rocblasDll
    Write-Host "  ✓ rocblas.dll ($([math]::Round($fileInfo.Length / 1KB, 1)) KB)" -ForegroundColor Green
}
else {
    Write-Host "  ✗ rocblas.dll not built" -ForegroundColor Red
    $allGood = $false
}

# Check for Visual Studio
Write-Host "`n[4] Checking build tools..." -ForegroundColor Yellow
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($vsPath) {
        Write-Host "  ✓ Visual Studio 2022 found" -ForegroundColor Green
    }
    else {
        Write-Host "  ✗ Visual Studio C++ tools not found" -ForegroundColor Red
        $allGood = $false
    }
}
else {
    Write-Host "  ? Could not verify Visual Studio installation" -ForegroundColor Yellow
}

# Check CMake
Write-Host "`n[5] Checking CMake..." -ForegroundColor Yellow
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if ($cmake) {
    $version = & cmake --version | Select-Object -First 1
    Write-Host "  ✓ $version" -ForegroundColor Green
}
else {
    Write-Host "  ✗ CMake not found in PATH" -ForegroundColor Red
    $allGood = $false
}

# Check GPU
Write-Host "`n[6] Checking for AMD GPU..." -ForegroundColor Yellow
try {
    $gpu = Get-WmiObject Win32_VideoController | Where-Object { $_.Name -like "*AMD*" -or $_.Name -like "*Radeon*" }
    if ($gpu) {
        Write-Host "  ✓ Found: $($gpu.Name)" -ForegroundColor Green
        $vramGB = [math]::Round($gpu.AdapterRAM / 1GB, 2)
        if ($vramGB -gt 0) {
            Write-Host "    VRAM: $vramGB GB" -ForegroundColor Gray
        }
    }
    else {
        Write-Host "  ? No AMD GPU detected" -ForegroundColor Yellow
    }
}
catch {
    Write-Host "  ? Could not query GPU information" -ForegroundColor Yellow
}

# Summary
Write-Host "`n========================================" -ForegroundColor Cyan
if ($allGood) {
    Write-Host "Status: Ready to deploy! ✓" -ForegroundColor Green
    Write-Host "`nNext steps:" -ForegroundColor Cyan
    Write-Host "1. Rename real DLL (if not done): Copy-Item '$env:HIP_PATH\bin\amdhip64.dll' '$env:HIP_PATH\bin\amdhip64_real.dll'" -ForegroundColor Gray
    Write-Host "2. Install shim: .\scripts\install.ps1 -TargetPath 'path\to\app\bin' -Backup" -ForegroundColor Gray
    Write-Host "3. Test your application" -ForegroundColor Gray
}
else {
    Write-Host "Status: Issues detected ✗" -ForegroundColor Red
    Write-Host "`nResolve the issues above before deploying." -ForegroundColor Yellow
}
Write-Host "========================================" -ForegroundColor Cyan
