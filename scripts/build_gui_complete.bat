@echo off
echo ========================================
echo Twitch Stream Monitor GUI Build
echo ========================================
echo.

cd ..

REM Поиск Qt6
set QT_FOUND=0
set QT_PATH=

REM Проверка в стандартных местах
for %%d in (C:\Qt\6.* C:\Qt6\6.*) do (
    if exist "%%d\mingw_64\bin\qmake.exe" (
        set "QT_PATH=%%d\mingw_64"
        set QT_FOUND=1
        goto :qt_found
    )
)

REM Проверка в MSYS2
if exist "C:\msys64\mingw64\bin\qmake.exe" (
    set "QT_PATH=C:\msys64\mingw64"
    set QT_FOUND=1
    goto :qt_found
)

:qt_not_found
echo ========================================
echo ERROR: Qt6 not found!
echo ========================================
echo.
echo Please install Qt6:
echo.
echo Option 1: Official installer
echo   Download: https://www.qt.io/download-qt-installer-oss
echo   Install to: C:\Qt\6.x.x\mingw_64
echo.
echo Option 2: Via MSYS2
echo   pacman -S mingw-w64-x86_64-qt6-base
echo.
echo Then run this script again.
echo.
pause
exit /b 1

:qt_found
echo [OK] Qt6 found at: %QT_PATH%
echo.

REM Очистка build
if exist build (
    echo [CLEAN] Removing old build...
    rmdir /s /q build 2>nul
    if exist build (
        echo [WARNING] Could not remove build folder
        echo Please close any programs using it and try again
        pause
        exit /b 1
    )
)

mkdir build
cd build

echo [1/4] Configuring with CMake...
echo.
echo Paths:
echo   CURL Include: C:/curl/include
echo   CURL Library: C:/curl/lib/libcurl.dll.a
echo   Qt6 Path: %QT_PATH%
echo.

cmake .. ^
  -G "MinGW Makefiles" ^
  -DCURL_INCLUDE_DIR=C:/curl/include ^
  -DCURL_LIBRARY=C:/curl/lib/libcurl.dll.a ^
  -DCMAKE_PREFIX_PATH=%QT_PATH% ^
  -DBUILD_GUI=ON

if errorlevel 1 (
    echo.
    echo ========================================
    echo ERROR: CMake configuration failed
    echo ========================================
    pause
    exit /b 1
)

echo.
echo [2/4] Building project...
echo.

mingw32-make -j4

if errorlevel 1 (
    echo.
    echo ========================================
    echo ERROR: Build failed
    echo ========================================
    pause
    exit /b 1
)

echo.
echo [3/4] Copying DLLs...
echo.

REM Копируем libcurl.dll
if exist C:\curl\bin\libcurl.dll (
    copy /Y C:\curl\bin\libcurl.dll .
    echo   [OK] Copied libcurl.dll
)

REM Пробуем использовать windeployqt
where /q windeployqt
if not errorlevel 1 (
    echo   [OK] Using windeployqt to copy Qt DLLs...
    windeployqt stream_monitor_gui.exe --release --no-translations
) else (
    echo   [INFO] windeployqt not found, copying DLLs manually...
    
    REM Копируем Qt DLLs вручную
    if exist "%QT_PATH%\bin\Qt6Core.dll" (
        copy /Y "%QT_PATH%\bin\Qt6Core.dll" .
        copy /Y "%QT_PATH%\bin\Qt6Gui.dll" .
        copy /Y "%QT_PATH%\bin\Qt6Widgets.dll" .
        copy /Y "%QT_PATH%\bin\Qt6Network.dll" .
        echo   [OK] Copied Qt6 DLLs
    )
    
    REM Копируем MinGW runtime DLLs
    if exist "%QT_PATH%\bin\libgcc_s_seh-1.dll" (
        copy /Y "%QT_PATH%\bin\libgcc_s_seh-1.dll" .
        copy /Y "%QT_PATH%\bin\libstdc++-6.dll" .
        copy /Y "%QT_PATH%\bin\libwinpthread-1.dll" .
        echo   [OK] Copied MinGW runtime DLLs
    )
    
    REM Создаем папку platforms
    if not exist platforms mkdir platforms
    if exist "%QT_PATH%\plugins\platforms\qwindows.dll" (
        copy /Y "%QT_PATH%\plugins\platforms\qwindows.dll" platforms\
        echo   [OK] Copied platform plugin
    )
)

REM Копирование конфигов
if exist ..\config\config.ini (
    copy /Y ..\config\config.ini . >nul 2>&1
)
if exist ..\config\streamers.txt (
    copy /Y ..\config\streamers.txt . >nul 2>&1
)

REM Создание папок
if not exist logs mkdir logs
if not exist stats mkdir stats

echo.
echo [4/4] Build complete!
echo.
echo ========================================
echo SUCCESS!
echo ========================================
echo.
echo Executables:
echo   CLI: build\stream_monitor_cli.exe
echo   GUI: build\stream_monitor_gui.exe
echo.
echo To run GUI:
echo   cd build
echo   stream_monitor_gui.exe
echo.
echo To run CLI:
echo   cd build
echo   stream_monitor_cli.exe lydiaviolet
echo.

pause