@echo off
setlocal enabledelayedexpansion
echo ========================================
echo Qt Installation Analysis
echo ========================================
echo.

set "QT_ROOT=C:\Qt"

if not exist "%QT_ROOT%" (
    echo ERROR: %QT_ROOT% does not exist
    pause
    exit /b 1
)

echo Root: %QT_ROOT%
echo.

echo === Qt Versions Found ===
for /d %%v in ("%QT_ROOT%\*") do (
    set "ver=%%~nxv"
    if "!ver:~0,1!"=="6" (
        echo [Qt6] !ver!
    ) else if "!ver:~0,1!"=="5" (
        echo [Qt5] !ver!
    ) else (
        echo [Other] !ver!
    )
)

echo.
echo === Compiler Kits ===
for /d %%v in ("%QT_ROOT%\6.*" "%QT_ROOT%\5.*") do (
    if exist "%%v" (
        echo.
        echo Version: %%~nxv
        for /d %%k in ("%%v\*") do (
            set "kit=%%~nxk"
            if /i "!kit:~0,5!"=="mingw" (
                echo   [MinGW] !kit!
                if exist "%%k\bin\qmake.exe" echo      - Has qmake
                if exist "%%k\bin\Qt6Core.dll" echo      - Has Qt6 DLLs
                if exist "%%k\lib\cmake\Qt6" echo      - Has CMake config
            ) else if /i "!kit:~0,4!"=="msvc" (
                echo   [MSVC] !kit!
            ) else (
                REM Count files in other directories
                for /f %%c in ('dir "%%k" /a-d /s 2^>nul ^| find "File(s)"') do set count=%%c
            )
        )
    )
)

echo.
echo === Critical Files Location ===
echo.
echo Looking for qmake.exe...
set QMAKE_FOUND=0
for /r "%QT_ROOT%" %%f in (qmake.exe) do (
    set QMAKE_FOUND=1
    echo   [FOUND] %%f
)
if !QMAKE_FOUND!==0 echo   [NOT FOUND] qmake.exe

echo.
echo Looking for Qt6Core.dll...
set QT6CORE_FOUND=0
for /r "%QT_ROOT%" %%f in (Qt6Core.dll) do (
    set QT6CORE_FOUND=1
    echo   [FOUND] %%f
)
if !QT6CORE_FOUND!==0 echo   [NOT FOUND] Qt6Core.dll

echo.
echo Looking for CMake Qt6 config...
set CMAKE_FOUND=0
for /d /r "%QT_ROOT%" %%d in (Qt6) do (
    if exist "%%d\Qt6Config.cmake" (
        set CMAKE_FOUND=1
        echo   [FOUND] %%d\Qt6Config.cmake
    )
)
if !CMAKE_FOUND!==0 echo   [NOT FOUND] Qt6Config.cmake

echo.
echo === Recommended CMAKE_PREFIX_PATH ===
echo.
for /d %%v in ("%QT_ROOT%\6.*") do (
    if exist "%%v" (
        for /d %%k in ("%%v\mingw*") do (
            if exist "%%k\bin\qmake.exe" (
                echo   -DCMAKE_PREFIX_PATH=%%k
                goto :path_found
            )
        )
    )
)
echo   [WARNING] No suitable Qt6 MinGW installation found
:path_found

echo.
echo === Summary ===
echo.

REM Count total files in Qt folder
for /f "tokens=3" %%a in ('dir "%QT_ROOT%" /s /a-d 2^>nul ^| find "File(s)"') do set TOTAL_FILES=%%a
echo Total files in Qt folder: %TOTAL_FILES%

REM Get folder size
for /f "tokens=3" %%a in ('dir "%QT_ROOT%" /s /-c 2^>nul ^| find "bytes"') do set TOTAL_SIZE=%%a
echo Total size: %TOTAL_SIZE% bytes

echo.
echo ========================================
echo.
pause