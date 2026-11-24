@echo off
title Mochi Bot - Build Standalone Windows Executable
cd /d "%~dp0"

echo ======================================================================
echo           MOCHI BOT V1 - BUILD STANDALONE EXECUTABLE (.EXE)
echo ======================================================================
echo.
echo Compiling tools\windows_companion_tray.py into a single, standalone
echo MochiCompanion.exe with PyInstaller...
echo.

set "PY_EXE="
if exist ".venv\Scripts\python.exe" (
    set "PY_EXE=.venv\Scripts\python.exe"
) else (
    set "PY_EXE=python"
)

"%PY_EXE%" -m PyInstaller --noconsole --onefile --name "MochiCompanion" "tools\windows_companion_tray.py" --distpath "dist" --workpath "build" --specpath "build"

if %errorlevel% neq 0 (
    echo [!] PyInstaller build failed.
    pause
    exit /b %errorlevel%
)

copy /y "dist\MochiCompanion.exe" "MochiCompanion.exe"

echo.
echo ======================================================================
echo [+] SUCCESS! Created MochiCompanion.exe in project root.
echo ======================================================================
pause
