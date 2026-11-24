@echo off
title Mochi Bot - Launch Desktop Companion
cd /d "%~dp0"

echo ==================================================
echo   MOCHI BOT V1 - DESKTOP COMPANION LAUNCHER
echo ==================================================
echo.

:: 1. Check if standalone compiled EXE exists (zero-python needed)
if exist "MochiCompanion.exe" (
    echo [+] Launching standalone MochiCompanion.exe...
    start "" "MochiCompanion.exe"
    goto success
)

:: 2. Check for virtualenv pythonw first (silent window)
if exist ".venv\Scripts\pythonw.exe" (
    echo [+] Launching via virtual environment PythonW...
    start "" ".venv\Scripts\pythonw.exe" "tools\windows_companion_tray.py"
    goto success
)

:: 3. Check for virtualenv python
if exist ".venv\Scripts\python.exe" (
    echo [+] Launching via virtual environment Python...
    start "" ".venv\Scripts\python.exe" "tools\windows_companion_tray.py"
    goto success
)

:: 4. Fallback to system PythonW
where pythonw >nul 2>&1
if %errorlevel% equ 0 (
    echo [+] Launching via system PythonW...
    start "" pythonw "tools\windows_companion_tray.py"
    goto success
)

:: 5. Fallback to system Python
where python >nul 2>&1
if %errorlevel% equ 0 (
    echo [+] Launching via system Python...
    start "" python "tools\windows_companion_tray.py"
    goto success
)

echo [ERROR] No standalone executable or Python installation was found!
echo Please run Build_Standalone_Exe.bat or install Python 3.
pause
exit /b 1

:success
echo [+] Mochi Companion is running in your Windows System Tray (near clock).
timeout /t 2 >nul
exit
