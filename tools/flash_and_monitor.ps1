# Mochi Bot V1 - Flash & Serial Monitor Helper Script
$ErrorActionPreference = "Stop"

$workspaceRoot = Split-Path -Parent $PSScriptRoot
$pioExe = Join-Path $workspaceRoot ".venv\Scripts\platformio.exe"

if (!(Test-Path $pioExe)) {
    Write-Error "PlatformIO executable not found in .venv. Ensure virtualenv is set up."
    exit 1
}

Write-Host "==================================================" -ForegroundColor Cyan
Write-Host "  MOCHI BOT V1 - FIRMWARE FLASH & MONITOR UTILITY" -ForegroundColor Cyan
Write-Host "==================================================" -ForegroundColor Cyan
Write-Host "SAFETY CHECK:" -ForegroundColor Yellow
Write-Host "1. Ensure bot is UNDOCKED from magnetic cradle." -ForegroundColor Yellow
Write-Host "2. Ensure LiPo battery is DISCONNECTED from board." -ForegroundColor Yellow
Write-Host "3. Connect ESP32-S3 via USB-C data cable." -ForegroundColor Yellow
Write-Host ""
$confirm = Read-Host "Proceed with upload? (y/N)"

if ($confirm -ne 'y' -and $confirm -ne 'Y') {
    Write-Host "Upload canceled." -ForegroundColor Red
    exit 0
}

Write-Host "`n[1/2] Uploading firmware to ESP32-S3 SuperMini..." -ForegroundColor Green
& $pioExe run -d $workspaceRoot --target upload

if ($LASTEXITCODE -eq 0) {
    Write-Host "`n[2/2] Launching Serial Monitor (115200 baud)..." -ForegroundColor Green
    & $pioExe device monitor -d $workspaceRoot -b 115200
} else {
    Write-Error "Firmware upload failed. Check USB connection and driver."
}
