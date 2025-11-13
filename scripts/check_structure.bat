@echo off
echo ========================================
echo Checking Project Structure
echo ========================================
echo.

cd ..

echo Current directory: %CD%
echo.

echo === Checking include/ ===
if exist include (
    echo [OK] include\ exists
    dir include\*.h /b 2>nul
) else (
    echo [MISSING] include\ folder not found
)

echo.
echo === Checking src/ ===
if exist src (
    echo [OK] src\ exists
    dir src\*.cpp /b 2>nul
) else (
    echo [MISSING] src\ folder not found
)

echo.
echo === Checking for Config files ===
if exist src\Config.cpp (
    echo [OK] src\Config.cpp
) else if exist src\core\Config.cpp (
    echo [OK] src\core\Config.cpp ^(in core subfolder^)
) else (
    echo [MISSING] Config.cpp not found
)

if exist include\Config.h (
    echo [OK] include\Config.h
) else if exist include\core\Config.h (
    echo [OK] include\core\Config.h ^(in core subfolder^)
) else (
    echo [MISSING] Config.h not found
)

echo.
echo === Checking for main.cpp ===
if exist src\main.cpp (
    echo [OK] src\main.cpp
) else if exist src\core\main.cpp (
    echo [OK] src\core\main.cpp ^(in core subfolder^)
) else (
    echo [MISSING] main.cpp not found
)

echo.
echo === Full src/ tree ===
dir src /s /b 2>nul | findstr /i "\.cpp$"

echo.
echo === Full include/ tree ===
dir include /s /b 2>nul | findstr /i "\.h$"

echo.
echo ========================================
pause