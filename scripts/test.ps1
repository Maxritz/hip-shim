param(
    [switch]$Build,
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$testExe = Join-Path $projectRoot "build\bin\Release\test_shim.exe"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "HIP Shim Test Runner" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Build if requested
if ($Build) {
    Write-Host "`nBuilding test..." -ForegroundColor Yellow
    & "$PSScriptRoot\build.ps1"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed!" -ForegroundColor Red
        exit $LASTEXITCODE
    }
}

# Check if test executable exists
if (-not (Test-Path $testExe)) {
    Write-Host "`nTest executable not found: $testExe" -ForegroundColor Red
    Write-Host "Run with -Build flag to build first" -ForegroundColor Yellow
    exit 1
}

# Check HIP_PATH
if (-not $env:HIP_PATH) {
    Write-Host "`nWARNING: HIP_PATH not set" -ForegroundColor Yellow
    Write-Host "Set with: `$env:HIP_PATH = 'C:\Program Files\AMD\ROCm\7.1'" -ForegroundColor Gray
}

# Ensure the shim DLL is in the same directory as the test exe or in PATH
$shimDll = Join-Path (Split-Path $testExe) "amdhip64.dll"
if (-not (Test-Path $shimDll)) {
    Write-Host "`nWARNING: Shim DLL not found next to test executable" -ForegroundColor Yellow
    $builtShim = Join-Path $projectRoot "build\bin\Release\amdhip64.dll"
    if (Test-Path $builtShim) {
        Write-Host "Copying shim DLL to test directory..." -ForegroundColor Gray
        Copy-Item $builtShim $shimDll
    }
}

# Run the test
Write-Host "`nRunning test..." -ForegroundColor Green
Write-Host "Command: $testExe" -ForegroundColor Gray
Write-Host ""

Push-Location (Split-Path $testExe)
try {
    & $testExe
    $exitCode = $LASTEXITCODE
    
    Write-Host ""
    
    # Check for log file
    $logFile = Join-Path (Get-Location) "hip_shim.log"
    if (Test-Path $logFile) {
        Write-Host "Log file created: $logFile" -ForegroundColor Cyan
        if ($Verbose) {
            Write-Host "`n--- hip_shim.log contents ---" -ForegroundColor Cyan
            Get-Content $logFile
            Write-Host "--- end of log ---`n" -ForegroundColor Cyan
        }
        else {
            Write-Host "Use -Verbose flag to see log contents" -ForegroundColor Gray
        }
    }
    
    if ($exitCode -eq 0) {
        Write-Host "Test PASSED ✓" -ForegroundColor Green
    }
    else {
        Write-Host "Test FAILED with exit code $exitCode" -ForegroundColor Red
    }
    
    exit $exitCode
}
finally {
    Pop-Location
}
