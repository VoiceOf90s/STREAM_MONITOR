@echo off
echo ========================================
echo CURL Installation Check
echo ========================================
echo.

echo Checking C:/curl structure...
echo.

REM Проверка заголовков
if exist C:\curl\include\curl\curl.h (
    echo [OK] Headers found: C:\curl\include\curl\curl.h
) else (
    echo [MISSING] C:\curl\include\curl\curl.h
    set HAS_ERROR=1
)

echo.
echo Checking libraries...
echo.

REM Проверка библиотек (разные варианты)
if exist C:\curl\lib\libcurl.dll.a (
    echo [OK] Library found: C:\curl\lib\libcurl.dll.a
    set CURL_LIB=C:\curl\lib\libcurl.dll.a
) else if exist C:\curl\lib\libcurl.a (
    echo [OK] Library found: C:\curl\lib\libcurl.a
    set CURL_LIB=C:\curl\lib\libcurl.a
) else if exist C:\curl\lib\libcurl_imp.lib (
    echo [OK] Library found: C:\curl\lib\libcurl_imp.lib
    set CURL_LIB=C:\curl\lib\libcurl_imp.lib
) else if exist C:\curl\lib\libcurl.lib (
    echo [OK] Library found: C:\curl\lib\libcurl.lib
    set CURL_LIB=C:\curl\lib\libcurl.lib
) else (
    echo [MISSING] No library found in C:\curl\lib\
    echo.
    echo Expected one of:
    echo   - libcurl.dll.a  (MinGW)
    echo   - libcurl.a      (MinGW static)
    echo   - libcurl_imp.lib (MSVC)
    echo   - libcurl.lib    (MSVC)
    set HAS_ERROR=1
)

echo.
echo Checking DLL...
echo.

if exist C:\curl\bin\libcurl.dll (
    echo [OK] DLL found: C:\curl\bin\libcurl.dll
) else if exist C:\curl\bin\libcurl-4.dll (
    echo [OK] DLL found: C:\curl\bin\libcurl-4.dll
) else if exist C:\curl\lib\libcurl.dll (
    echo [OK] DLL found: C:\curl\lib\libcurl.dll
) else (
    echo [WARNING] DLL not found
    echo   Expected in: C:\curl\bin\libcurl.dll
    echo   This is needed at runtime
)

echo.
echo ========================================
echo Full C:/curl structure:
echo ========================================
echo.
dir C:\curl /s /b | findstr /i "curl\.h libcurl\."

echo.
echo ========================================

if defined HAS_ERROR (
    echo.
    echo [ERROR] CURL installation incomplete!
    echo.
    echo Please ensure you have:
    echo   C:\curl\include\curl\curl.h
    echo   C:\curl\lib\libcurl.dll.a  (or similar)
    echo   C:\curl\bin\libcurl.dll
    echo.
    echo If you downloaded from https://curl.se/windows/
    echo make sure to extract ALL folders to C:\curl
    echo.
) else (
    echo.
    echo [SUCCESS] CURL installation looks good!
    echo.
    echo To build the project, use:
    echo   scripts\build_with_curl.bat
    echo.
    echo Or manually:
    if defined CURL_LIB (
        echo   cd build
        echo   cmake .. -G "MinGW Makefiles" ^
          -DCURL_INCLUDE_DIR=C:/curl/include ^
          -DCURL_LIBRARY=%CURL_LIB%
        echo   mingw32-make
    )
    echo.
)

pause