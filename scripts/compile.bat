@echo off
echo ========================================
echo Twitch Stream Monitor CLI Build
echo ========================================
echo.

cd ..

REM Проверка CMake
where /q cmake
if errorlevel 1 (
    echo ERROR: CMake not found
    echo Install: https://cmake.org/download/
    pause
    exit /b 1
)

REM Очистка
if exist build_cli (
    echo [CLEAN] Removing old build...
    rmdir /s /q build_cli
)

mkdir build_cli
cd build_cli

echo [1/3] Configuring with CMake...
cmake .. -G "MinGW Makefiles"

if errorlevel 1 (
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)

echo.
echo [2/3] Building CLI only...
cmake --build . --target stream_monitor_cli --config Release

if errorlevel 1 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo.
echo [3/3] Build complete!
echo ========================================
echo.
echo Executable: build_cli\stream_monitor_cli.exe
echo.
echo Run: cd build_cli ^&^& stream_monitor_cli.exe lydiaviolet
echo.

pause