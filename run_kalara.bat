@echo off
setlocal

echo ===================================================
echo   KaLara Arch - Architectural Planning System
echo ===================================================

set "SCRIPT_DIR=%~dp0"
set "QT_BIN=C:\Qt\6.7.2\msvc2019_64\bin"
set "EXE_PATH=%SCRIPT_DIR%build\debug\editor\kalara_arch.exe"

if exist "%QT_BIN%" (
    set "PATH=%QT_BIN%;%PATH%"
)

if not exist "%EXE_PATH%" (
    echo [ERROR] KaLara Arch executable not found at:
    echo   %EXE_PATH%
    echo Please ensure the project has been built in build\debug.
    pause
    exit /b 1
)

echo Starting KaLara Arch...
start "" "%EXE_PATH%" %*

endlocal
