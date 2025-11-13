@echo off
echo ========================================
echo CURL DLL Setup
echo ========================================
echo.

cd ..

REM Проверка наличия libcurl-x64.dll в корне
if not exist libcurl-x64.dll (
    echo [ERROR] libcurl-x64.dll not found in project root
    echo.
    echo Expected location: %CD%\libcurl-x64.dll
    echo.
    echo Please make sure the file is in the project root directory.
    pause
    exit /b 1
)

echo [FOUND] libcurl-x64.dll in project root
echo.

REM Создание C:\curl\bin если нет
if not exist C:\curl\bin (
    echo [CREATE] Creating C:\curl\bin directory
    mkdir C:\curl\bin
)

REM Копирование и переименование в C:\curl\bin
echo [COPY] Copying libcurl-x64.dll to C:\curl\bin\libcurl.dll
copy /Y libcurl-x64.dll C:\curl\bin\libcurl.dll

if errorlevel 1 (
    echo [ERROR] Failed to copy DLL
    pause
    exit /b 1
)

echo.
echo ========================================
echo Setup complete!
echo ========================================
echo.
echo CURL DLL installed to: C:\curl\bin\libcurl.dll
echo.
echo You can now build the project:
echo   scripts\build_with_curl.bat
echo.

REM Опционально: копирование в build если уже существует
if exist build (
    echo [INFO] build\ folder exists, copying DLL there too...
    copy /Y C:\curl\bin\libcurl.dll build\
    if exist build\Release (
        copy /Y C:\curl\bin\libcurl.dll build\Release\
    )
    echo.
)

echo ========================================
echo Current CURL structure:
echo ========================================
echo.
echo C:\curl\
if exist C:\curl\bin\libcurl.dll (
    echo   bin\libcurl.dll               [OK]
) else (
    echo   bin\libcurl.dll               [MISSING]
)

if exist C:\curl\include\curl\curl.h (
    echo   include\curl\curl.h           [OK]
) else (
    echo   include\curl\curl.h           [MISSING]
)

if exist C:\curl\lib\libcurl.dll.a (
    echo   lib\libcurl.dll.a             [OK]
) else (
    echo   lib\libcurl.dll.a             [MISSING]
)

if exist C:\curl\lib\libcurl.a (
    echo   lib\libcurl.a                 [OK]
) else (
    echo   lib\libcurl.a                 [MISSING]
)

echo.
echo All CURL components are ready!
echo.

pause