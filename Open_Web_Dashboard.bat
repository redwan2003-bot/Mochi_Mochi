@echo off
title Mochi Bot - Web Bluetooth Companion Dashboard
cd /d "%~dp0"

echo ======================================================================
echo          MOCHI BOT V1 - WEB BLUETOOTH COMPANION DASHBOARD
echo ======================================================================
echo.
echo Launching Web Bluetooth companion dashboard in your default browser...
echo.
echo Features available in your browser (Chrome or Edge recommended):
echo   - Direct BLE Connect / Pair with Mochi Bot
echo   - Virtual 128x64 OLED Live Mirror & Expression Switcher
echo   - Real-time PC Clock Synchronization
echo   - Driving Speedometer & GPS Telemetry Mode
echo   - Instant Custom Notification & Priority Alerts
echo   - Brightness, Sleep Timer, and Volume Controls
echo.

start "" "tools\web_ble_companion\index.html"

timeout /t 2 >nul
exit
