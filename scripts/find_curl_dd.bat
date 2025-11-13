@echo off
echo Searching for libcurl DLL...
echo.

REM Поиск в C:\curl
echo Checking C:\curl...
dir C:\curl\*.dll /s /b 2>nul

REM Поиск в MinGW
echo.
echo Checking C:\mingw64...
dir C:\mingw64\bin\libcurl*.dll /s /b 2>nul

REM Поиск в MSYS2
echo.
echo Checking C:\msys64...
dir C:\msys64\mingw64\bin\libcurl*.dll /s /b 2>nul

REM Поиск в PATH
echo.
echo Checking system PATH...
where libcurl.dll 2>nul
where libcurl-4.dll 2>nul

echo.
echo ========================================
echo.
echo If DLL found above, note the path.
echo If NOT found, you need to download it.
echo.

pause