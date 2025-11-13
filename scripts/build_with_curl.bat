@echo off
echo ========================================
echo Twitch Stream Monitor Build
echo CURL Location: C:/curl
echo ========================================
echo.

cd ..

REM Проверка наличия DLL в корне проекта
if exist libcurl-x64.dll (
    echo [FOUND] libcurl-x64.dll in project root
    echo [ACTION] Copying to C:\curl\bin\
    if not exist C:\curl\bin mkdir C:\curl\bin
    copy /Y libcurl-x64.dll C:\curl\bin\libcurl.dll
    echo.
)

REM Очистка старой сборки
if exist build (
    echo [CLEAN] Removing old build...
    rmdir /s /q build
)

mkdir build
cd build

echo [1/3] Configuring with CMake...
echo.
echo CURL paths:
echo   Headers: C:/curl/include
echo   Library: C:/curl/lib/libcurl.dll.a
echo.

cmake .. ^
  -G "MinGW Makefiles" ^
  -DCURL_INCLUDE_DIR=C:/curl/include ^
  -DCURL_LIBRARY=C:/curl/lib/libcurl.dll.a

if errorlevel 1 (
    echo.
    echo ========================================
    echo ERROR: CMake configuration failed
    echo ========================================
    pause
    exit /b 1
)

echo.
echo [2/3] Building project...
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
echo [3/3] Copying CURL DLL...
echo.

REM Копируем DLL из разных возможных мест
set DLL_COPIED=0

if exist C:\curl\bin\libcurl.dll (
    copy /Y C:\curl\bin\libcurl.dll .
    echo   [OK] Copied libcurl.dll from C:\curl\bin\
    set DLL_COPIED=1
)

if exist ..\libcurl-x64.dll (
    copy /Y ..\libcurl-x64.dll libcurl.dll
    echo   [OK] Copied libcurl-x64.dll from project root
    set DLL_COPIED=1
)

if %DLL_COPIED%==0 (
    echo   [WARNING] libcurl.dll not found
    echo   Please copy libcurl-x64.dll to build\ manually
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
echo ========================================
echo Build complete!
echo ========================================
echo.
echo Executable: build\stream_monitor_cli.exe
echo.
echo To run:
echo   cd build
echo   stream_monitor_cli.exe lydiaviolet
echo.

pause