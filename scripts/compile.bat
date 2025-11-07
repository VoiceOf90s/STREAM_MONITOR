@echo off
cd ..
echo 🔨 Building Twitch Stream Monitor v2.3...
echo.

g++ -std=c++17 -o stream_monitor.exe ^
    -Iinclude ^
    -I"C:\curl\include" ^
    src\main.cpp ^
    src\Config.cpp ^
    src\Logger.cpp ^
    src\StringUtils.cpp ^
    src\HumanBehavior.cpp ^
    src\Notification.cpp ^
    src\Statistics.cpp ^
    src\WebScraper.cpp ^
    src\BrowserController.cpp ^
    src\StreamMonitor.cpp ^
    src\MultiStreamMonitor.cpp ^
    -L"C:\curl\lib" ^
    -lcurl -lbrotlidec -lbrotlicommon -lnghttp2 -lssl -lcrypto -lssh2 -lz -lzstd -lws2_32 -lwldap32 -lcrypt32 -lnormaliz

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ✅ Build successful!
    echo 📦 Executable: stream_monitor.exe
    echo.
    
    REM Копируем конфиг
    xcopy /Y config\config.ini . >nul 2>&1
    xcopy /Y config\streamers.txt . >nul 2>&1
    
    REM Создаем папки
    if not exist logs mkdir logs
    if not exist stats mkdir stats
    
    echo 📁 Config copied to root
    echo 📁 Folders: logs/, stats/
    echo.
    echo Run with: stream_monitor.exe lydiaviolet
) else (
    echo.
    echo ❌ Build failed! Check errors above.
)

pause