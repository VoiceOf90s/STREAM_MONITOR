#!/bin/bash

echo "========================================"
echo "Twitch Stream Monitor GUI Build (MSYS2)"
echo "========================================"
echo ""

# Проверка что запущено в MSYS2 MinGW64
if [[ ! "$MSYSTEM" =~ "MINGW64" ]]; then
    echo "ERROR: This script must be run in MSYS2 MinGW 64-bit terminal"
    echo ""
    echo "Please:"
    echo "1. Open 'MSYS2 MinGW 64-bit' from Start menu"
    echo "2. Navigate to: cd /c/Projects/STREAM_MONITOR"
    echo "3. Run: ./scripts/build_gui_msys2.sh"
    exit 1
fi

echo "[1/6] Checking Qt6 installation..."

if [ ! -f "/mingw64/bin/qmake.exe" ]; then
    echo ""
    echo "Qt6 not found! Installing..."
    echo ""
    pacman -S --needed --noconfirm mingw-w64-x86_64-qt6-base
    
    if [ $? -ne 0 ]; then
        echo "ERROR: Failed to install Qt6"
        exit 1
    fi
fi

echo "  [OK] Qt6 found"

echo ""
echo "[2/6] Cleaning old build..."
rm -rf build

echo ""
echo "[3/6] Configuring with CMake..."
mkdir build
cd build

cmake .. \
  -G "MinGW Makefiles" \
  -DCURL_INCLUDE_DIR=C:/curl/include \
  -DCURL_LIBRARY=C:/curl/lib/libcurl.dll.a \
  -DCMAKE_PREFIX_PATH=/mingw64 \
  -DBUILD_GUI=ON

if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed"
    exit 1
fi

echo ""
echo "[4/6] Building project..."
mingw32-make -j4

if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    exit 1
fi

echo ""
echo "[5/6] Copying DLLs..."

# Qt6 DLLs
cp /mingw64/bin/Qt6Core.dll .
cp /mingw64/bin/Qt6Gui.dll .
cp /mingw64/bin/Qt6Widgets.dll .
cp /mingw64/bin/Qt6Network.dll .
echo "  [OK] Qt6 DLLs"

# MinGW runtime
cp /mingw64/bin/libgcc_s_seh-1.dll .
cp /mingw64/bin/libstdc++-6.dll .
cp /mingw64/bin/libwinpthread-1.dll .
echo "  [OK] MinGW runtime DLLs"

# CURL
if [ -f "C:/curl/bin/libcurl.dll" ]; then
    cp C:/curl/bin/libcurl.dll .
    echo "  [OK] libcurl.dll"
else
    echo "  [WARNING] C:/curl/bin/libcurl.dll not found"
fi

# Platform plugin
mkdir -p platforms
cp /mingw64/share/qt6/plugins/platforms/qwindows.dll platforms/
echo "  [OK] Platform plugin"

# Дополнительные зависимости Qt
if [ -f "/mingw64/bin/libicuin73.dll" ]; then
    cp /mingw64/bin/libicuin*.dll . 2>/dev/null
    cp /mingw64/bin/libicuuc*.dll . 2>/dev/null
    cp /mingw64/bin/libicudt*.dll . 2>/dev/null
    cp /mingw64/bin/libpcre2-16-0.dll . 2>/dev/null
    cp /mingw64/bin/zlib1.dll . 2>/dev/null
    cp /mingw64/bin/libharfbuzz-0.dll . 2>/dev/null
    cp /mingw64/bin/libpng16-16.dll . 2>/dev/null
    cp /mingw64/bin/libfreetype-6.dll . 2>/dev/null
    cp /mingw64/bin/libbz2-1.dll . 2>/dev/null
    cp /mingw64/bin/libbrotlidec.dll . 2>/dev/null
    cp /mingw64/bin/libbrotlicommon.dll . 2>/dev/null
    cp /mingw64/bin/libglib-2.0-0.dll . 2>/dev/null
    cp /mingw64/bin/libintl-8.dll . 2>/dev/null
    cp /mingw64/bin/libiconv-2.dll . 2>/dev/null
    cp /mingw64/bin/libgraphite2.dll . 2>/dev/null
    cp /mingw64/bin/libpcre-1.dll . 2>/dev/null
    echo "  [OK] Additional dependencies"
fi

# Копирование конфигов
if [ -f "../config/config.ini" ]; then
    cp ../config/config.ini .
fi
if [ -f "../config/streamers.txt" ]; then
    cp ../config/streamers.txt .
fi

# Создание папок
mkdir -p logs stats

echo ""
echo "[6/6] Build complete!"
echo ""
echo "========================================"
echo "SUCCESS!"
echo "========================================"
echo ""
echo "Executables:"
echo "  CLI: build/stream_monitor_cli.exe"
echo "  GUI: build/stream_monitor_gui.exe"
echo ""
echo "To run GUI:"
echo "  cd build"
echo "  ./stream_monitor_gui.exe"
echo ""
echo "Or from Windows CMD:"
echo "  cd C:\\Projects\\STREAM_MONITOR\\build"
echo "  stream_monitor_gui.exe"
echo ""