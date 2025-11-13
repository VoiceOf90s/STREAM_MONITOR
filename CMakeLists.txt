cmake_minimum_required(VERSION 3.15)

# Установить UTF-8 кодировку
if(WIN32)
    set(CMAKE_SYSTEM_ENCODING UTF-8)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

project(TwitchStreamMonitor VERSION 2.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Опция для сборки GUI (по умолчанию ON, но можно отключить)
option(BUILD_GUI "Build GUI version with Qt6" ON)

# Поиск обязательных зависимостей
find_package(Threads REQUIRED)

# Поиск CURL (пробуем разные способы)
find_package(CURL QUIET)
if(NOT CURL_FOUND)
    # Попытка найти через pkg-config
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(CURL QUIET libcurl)
    endif()
endif()

# Если CURL всё ещё не найден, используем ручные пути (для MinGW)
if(NOT CURL_FOUND)
    message(STATUS "CURL not found via find_package, trying manual search...")
    
    # Типичные пути для Windows/MinGW
    set(CURL_SEARCH_PATHS
        "C:/curl"
        "C:/Program Files/CURL"
        "C:/mingw64"
        "${CMAKE_SOURCE_DIR}/../curl"
        "${CMAKE_SOURCE_DIR}/dependencies/curl"
    )
    
    find_path(CURL_INCLUDE_DIR
        NAMES curl/curl.h
        PATHS ${CURL_SEARCH_PATHS}
        PATH_SUFFIXES include
    )
    
    find_library(CURL_LIBRARY
        NAMES curl libcurl libcurl_imp curl-d libcurl-d
        PATHS ${CURL_SEARCH_PATHS}
        PATH_SUFFIXES lib lib64 bin
    )
    
    if(CURL_INCLUDE_DIR AND CURL_LIBRARY)
        set(CURL_FOUND TRUE)
        set(CURL_LIBRARIES ${CURL_LIBRARY})
        set(CURL_INCLUDE_DIRS ${CURL_INCLUDE_DIR})
        message(STATUS "Found CURL manually:")
        message(STATUS "  Include: ${CURL_INCLUDE_DIR}")
        message(STATUS "  Library: ${CURL_LIBRARY}")
    endif()
endif()

# Если CURL так и не найден - выводим подсказку
if(NOT CURL_FOUND)
    message(FATAL_ERROR 
        "CURL not found!\n"
        "Please install CURL:\n"
        "  Windows: Download from https://curl.se/windows/\n"
        "           Or install via vcpkg: vcpkg install curl\n"
        "  Linux:   sudo apt-get install libcurl4-openssl-dev\n"
        "  macOS:   brew install curl\n"
        "\n"
        "Or specify CURL path manually:\n"
        "  cmake .. -DCURL_INCLUDE_DIR=C:/curl/include -DCURL_LIBRARY=C:/curl/lib/libcurl.dll.a"
    )
endif()

# Qt настройки (только если BUILD_GUI включен)
if(BUILD_GUI)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    
    find_package(Qt6 COMPONENTS Core Widgets Network QUIET)
    
    if(NOT Qt6_FOUND)
        message(WARNING 
            "Qt6 not found! GUI version will NOT be built.\n"
            "To build GUI, install Qt6 and specify path:\n"
            "  cmake .. -DCMAKE_PREFIX_PATH=C:/Qt/6.5.0/msvc2019_64\n"
            "\n"
            "Building CLI version only..."
        )
        set(BUILD_GUI OFF)
    endif()
endif()

# Исходные файлы CORE (в подпапке src/core/)
set(CORE_SOURCES
    src/core/Config.cpp
    src/core/Logger.cpp
    src/core/StringUtils.cpp
    src/core/HumanBehavior.cpp
    src/core/Notification.cpp
    src/core/Statistics.cpp
    src/core/WebScraper.cpp
    src/core/BrowserController.cpp
    src/core/StreamMonitor.cpp
    src/core/MultiStreamMonitor.cpp
)

# ============================================================
# Консольная версия (CLI) - всегда собирается
# ============================================================

# Определяем путь к main.cpp
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    set(MAIN_CPP "src/main.cpp")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/core/main.cpp")
    set(MAIN_CPP "src/core/main.cpp")
else()
    message(FATAL_ERROR "Cannot find main.cpp!")
endif()

add_executable(stream_monitor_cli
    ${MAIN_CPP}
    ${CORE_SOURCES}
)

# Определяем пути к заголовкам
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/Config.h")
    set(INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/core/Config.h")
    set(INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/core")
else()
    set(INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include")
endif()

target_include_directories(stream_monitor_cli PRIVATE
    ${INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CURL_INCLUDE_DIRS}
)

target_link_libraries(stream_monitor_cli PRIVATE
    ${CURL_LIBRARIES}
    Threads::Threads
)

# Windows-specific линковка
if(WIN32)
    target_link_libraries(stream_monitor_cli PRIVATE ws2_32 wldap32 crypt32)
endif()

message(STATUS "✓ CLI version configured: stream_monitor_cli")

# ============================================================
# GUI версия (только если Qt6 найден)
# ============================================================
if(BUILD_GUI AND Qt6_FOUND)
    # Исходные файлы GUI
    set(GUI_SOURCES
        src/gui/MainWindow.cpp
        src/gui/StreamerListWidget.cpp
        src/gui/StreamerItem.cpp
        src/gui/AddStreamerDialog.cpp
        src/gui/TrayIcon.cpp
    )
    
    # Исходные файлы Models
    set(MODEL_SOURCES
        src/models/StreamerPriority.cpp
        src/models/MonitorController.cpp
    )
    
    # Ресурсы Qt
    set(RESOURCES
        resources/resources.qrc
    )
    
    add_executable(stream_monitor_gui WIN32
        src/main_gui.cpp
        ${CORE_SOURCES}
        ${GUI_SOURCES}
        ${MODEL_SOURCES}
        ${RESOURCES}
    )
    
    target_include_directories(stream_monitor_gui PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/gui
        ${CMAKE_CURRENT_SOURCE_DIR}/include/models
        ${CURL_INCLUDE_DIRS}
    )
    
    target_link_libraries(stream_monitor_gui PRIVATE
        ${CURL_LIBRARIES}
        Threads::Threads
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
    )
    
    if(WIN32)
        target_link_libraries(stream_monitor_gui PRIVATE ws2_32 wldap32 crypt32)
    endif()
    
    message(STATUS "✓ GUI version configured: stream_monitor_gui")
else()
    message(STATUS "✗ GUI version skipped (Qt6 not found or BUILD_GUI=OFF)")
endif()

# Копирование конфигов
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config/config.ini")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config/config.ini 
         DESTINATION ${CMAKE_BINARY_DIR})
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config/streamers.txt")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config/streamers.txt 
         DESTINATION ${CMAKE_BINARY_DIR})
endif()

# Создание папок
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/logs)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/stats)

# Итоговое сообщение
message(STATUS "")
message(STATUS "═══════════════════════════════════════")
message(STATUS "Twitch Stream Monitor v2.4")
message(STATUS "═══════════════════════════════════════")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  CURL: ${CURL_FOUND}")
if(CURL_FOUND)
    message(STATUS "    Include: ${CURL_INCLUDE_DIRS}")
    message(STATUS "    Library: ${CURL_LIBRARIES}")
endif()
message(STATUS "  Qt6: ${Qt6_FOUND}")
if(Qt6_FOUND)
    message(STATUS "    Version: ${Qt6_VERSION}")
endif()
message(STATUS "")
message(STATUS "Targets:")
message(STATUS "  ✓ stream_monitor_cli (CLI version)")
if(BUILD_GUI AND Qt6_FOUND)
    message(STATUS "  ✓ stream_monitor_gui (GUI version)")
else()
    message(STATUS "  ✗ stream_monitor_gui (disabled)")
endif()
message(STATUS "═══════════════════════════════════════")
message(STATUS "")