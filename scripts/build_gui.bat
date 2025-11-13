@echo off
echo ========================================
echo Twitch Stream Monitor v2.4 (GUI Build)
echo ========================================
echo.

cd ..

REM Проверка наличия Qt6
where /q qmake
if errorlevel 1 (
    echo ERROR: Qt6 not found in PATH
    echo Please install Qt6 and add to PATH
    echo Example: C:\Qt\6.5.0\msvc2019_64\bin
    echo.
    echo Or try: set PATH=C:\Qt\6.5.0\msvc2019_64\bin;%%PATH%%
    pause
    exit /b 1
)

echo [INFO] Qt6 found in PATH
qmake --version
echo.

REM Проверка CMake
where /q cmake
if errorlevel 1 (
    echo ERROR: CMake not found in PATH
    echo Please install CMake 3.15+
    pause
    exit /b 1
)

echo [INFO] CMake found
cmake --version
echo.

REM Очистка старой сборки
if exist build (
    echo [CLEAN] Removing old build directory...
    rmdir /s /q build
)

REM Создание build директории
mkdir build
cd build

echo.
echo [1/4] Configuring with CMake...
echo.

REM Попытка автоматически найти Qt
for %%d in (C:\Qt\6.* C:\Qt6\6.* %LOCALAPPDATA%\Qt\6.*) do (
    if exist "%%d\msvc*_64\lib\cmake\Qt6" (
        set "QT_PATH=%%d\msvc2019_64"
        goto :found_qt
    )
)

:not_found_qt
echo WARNING: Could not auto-detect Qt6 path
echo Please specify Qt6 path manually
echo Example: cmake .. -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\msvc2019_64"
echo.
cmake ..
goto :after_cmake

:found_qt
echo Found Qt6 at: %QT_PATH%
cmake .. -DCMAKE_PREFIX_PATH="%QT_PATH%"

:after_cmake
if errorlevel 1 (
    echo.
    echo ERROR: CMake configuration failed
    echo.
    echo Possible fixes:
    echo 1. Make sure Qt6 is installed
    echo 2. Set Qt6 path: cmake .. -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\msvc2019_64"
    echo 3. Install missing dependencies
    pause
    exit /b 1
)

echo.
echo [2/4] Building project...
echo.
cmake --build . --config Release -j 4

if errorlevel 1 (
    echo.
    echo ERROR: Build failed
    echo Check errors above
    pause
    exit /b 1
)

echo.
echo [3/4] Deploying Qt dependencies...
echo.

REM Копирование Qt DLL (если windeployqt доступен)
where /q windeployqt
if not errorlevel 1 (
    windeployqt Release\stream_monitor_gui.exe --release --no-translations
) else (
    echo WARNING: windeployqt not found, skipping Qt DLL deployment
    echo You may need to copy Qt DLLs manually
)

REM Копирование конфигов (уже делает CMake, но на всякий случай)
if exist ..\config\config.ini (
    xcopy /Y ..\config\config.ini . >nul 2>&1
)
if exist ..\config\streamers.txt (
    xcopy /Y ..\config\streamers.txt . >nul 2>&1
)

REM Создание папок
if not exist logs mkdir logs
if not exist stats mkdir stats

echo.
echo [4/4] Build complete!
echo ========================================
echo.
echo Executables:
echo   GUI:  build\Release\stream_monitor_gui.exe
echo   CLI:  build\Release\stream_monitor_cli.exe
echo.
echo To run GUI version:
echo   cd build\Release
echo   stream_monitor_gui.exe
echo.
echo To run CLI version:
echo   cd build\Release
echo   stream_monitor_cli.exe lydiaviolet
echo.

pause