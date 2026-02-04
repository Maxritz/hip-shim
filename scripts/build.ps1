param(
    [string]$BuildType = "Release",
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "HIP-Shim Build Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Check HIP_PATH
if (-not $env:HIP_PATH) {
    Write-Host "ERROR: HIP_PATH environment variable not set" -ForegroundColor Red
    Write-Host "Set it with: `$env:HIP_PATH = 'C:\Program Files\AMD\ROCm\7.1'" -ForegroundColor Yellow
    exit 1
}

Write-Host "HIP_PATH: $env:HIP_PATH" -ForegroundColor Green

# Get project root
$projectRoot = Split-Path -Parent $PSScriptRoot
Write-Host "Project root: $projectRoot" -ForegroundColor Gray
Set-Location $projectRoot

# Clean if requested
if ($Clean -and (Test-Path "build")) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force "build"
}

# Create build directory
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

Set-Location "build"

# Configure
Write-Host "`nConfiguring CMake..." -ForegroundColor Cyan
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=$BuildType ..

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: CMake configuration failed" -ForegroundColor Red
    exit $LASTEXITCODE
}

# Build
Write-Host "`nBuilding..." -ForegroundColor Cyan
cmake --build . --config $BuildType --parallel

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Build failed" -ForegroundColor Red
    exit $LASTEXITCODE
}

# Show results
Write-Host "`n========================================" -ForegroundColor Green
Write-Host "Build complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host "Output directory: $projectRoot\build\bin\$BuildType" -ForegroundColor Gray

$outputDir = Join-Path $projectRoot "build\bin\$BuildType"
if (Test-Path $outputDir) {
    Write-Host "`nBuilt files:" -ForegroundColor Cyan
    Get-ChildItem $outputDir -Filter "*.dll" | ForEach-Object {
        $size = [math]::Round($_.Length / 1KB, 2)
        Write-Host "  $($_.Name) ($size KB)" -ForegroundColor Gray
    }
}

Set-Location $projectRoot
