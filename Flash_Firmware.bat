@echo off
title Mochi Bot V1 - 1-Click Firmware Flasher
color 0b
cd /d "%~dp0"

echo ======================================================================
echo                MOCHI BOT V1 - FIRMWARE FLASHER & MONITOR
echo ======================================================================
echo.
echo  [CRITICAL HARDWARE SAFETY PRE-CHECK]:
echo  1. DISCONNECT the 3.7V LiPo battery (or turn power switch OFF).
echo  2. UNDOCK Mochi Bot from the 2-pin magnetic charging cradle.
echo  3. Plug the ESP32-S3 SuperMini DIRECTLY into your PC using a USB-C
echo     DATA cable (avoid charging-only cables).
echo.
echo ======================================================================
echo.
echo Press any key when your ESP32-S3 is connected via USB-C to continue...
pause >nul

echo.
echo [*] Locating PlatformIO compiler toolchain...

set "PIO_BIN="
if exist ".venv\Scripts\platformio.exe" (
    set "PIO_BIN=.venv\Scripts\platformio.exe"
    goto found_pio
)

where platformio >nul 2>&1
if %errorlevel% equ 0 (
    set "PIO_BIN=platformio"
    goto found_pio
)

echo [!] Error: PlatformIO was not found in .venv or system PATH.
echo     Please ensure Python and PlatformIO are installed.
echo.
pause
exit /b 1

:found_pio
echo [+] Found toolchain: %PIO_BIN%
echo.
echo [*] Compiling firmware and flashing ESP32-S3...
echo     (If flashing hangs or fails, hold the BOOT button, press RESET, then release BOOT)
echo.

"%PIO_BIN%" run --target upload

if %errorlevel% neq 0 (
    echo.
    echo ======================================================================
    echo  [!] UPLOAD FAILED!
    echo ======================================================================
    echo  Troubleshooting steps:
    echo   1. Verify your USB cable carries data (some cheap cables are power-only).
    echo   2. Put ESP32-S3 into ROM Bootloader:
    echo      - Hold BOOT button (GPIO0).
    echo      - Tap RESET button (RST).
    echo      - Release BOOT button.
    echo   3. Ensure no other application (e.g. Arduino IDE, PuTTY) has the COM port open.
    echo.
    pause
    exit /b %errorlevel%
)

echo.
echo ======================================================================
echo  [+] FLASH COMPLETED SUCCESSFULLY!
echo ======================================================================
echo.
echo [*] Starting Serial Monitor at 115200 baud...
echo     (Press Ctrl+C to exit monitor at any time)
echo.

"%PIO_BIN%" device monitor -b 115200

pause
