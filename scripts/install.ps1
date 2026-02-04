param(
    [Parameter(Mandatory = $true)]
    [string]$TargetPath,
    [switch]$Backup,
    [switch]$Restore
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$buildBin = Join-Path $projectRoot "build\bin\Release"

# Restore backups
if ($Restore) {
    Write-Host "Restoring original DLLs..." -ForegroundColor Yellow
    @("amdhip64.dll", "rocblas.dll") | ForEach-Object {
        $backup = Join-Path $TargetPath "$_.backup"
        $target = Join-Path $TargetPath $_
        if (Test-Path $backup) {
            Copy-Item $backup $target -Force
            Remove-Item $backup
            Write-Host "Restored: $_" -ForegroundColor Green
        }
        else {
            Write-Host "No backup found for: $_" -ForegroundColor Yellow
        }
    }
    exit 0
}

# Check build exists
if (-not (Test-Path $buildBin)) {
    Write-Host "ERROR: Build directory not found" -ForegroundColor Red
    Write-Host "Run build.ps1 first" -ForegroundColor Yellow
    exit 1
}

# Check target directory
if (-not (Test-Path $TargetPath)) {
    Write-Host "ERROR: Target path does not exist: $TargetPath" -ForegroundColor Red
    exit 1
}

Write-Host "Installing HIP-Shim to: $TargetPath" -ForegroundColor Cyan

# Install amdhip64.dll
@("amdhip64.dll") | ForEach-Object {
    $src = Join-Path $buildBin $_
    $dst = Join-Path $TargetPath $_
    $backup = Join-Path $TargetPath "$_.backup"

    if (-not (Test-Path $src)) {
        Write-Host "WARNING: $_ not found in build" -ForegroundColor Yellow
        return
    }

    # Backup original if requested and exists
    if ($Backup -and (Test-Path $dst) -and -not (Test-Path $backup)) {
        Copy-Item $dst $backup -Force
        Write-Host "Backed up: $_ -> $_.backup" -ForegroundColor Gray
    }

    # Copy shim DLL
    Copy-Item $src $dst -Force
    Write-Host "Installed: $_" -ForegroundColor Green
}

Write-Host "`nInstallation complete!" -ForegroundColor Green
Write-Host "Run your application and check for hip_shim.log" -ForegroundColor Gray
