#include "StreamMonitor.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <ctime>
#include <random>
#include <iomanip>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <cstdlib>
#endif


StreamMonitor::StreamMonitor(const std::string& streamer)
    : streamerName(streamer), wasOnlineBefore(false), curlHandle(nullptr),
      lastOnlineTimestamp(0), maxHtmlSizeToAnalyze(100000) {  // Анализируем только первые 100KB
    
    LogMessage("=== Stream Monitor initialized (OPTIMIZED) ===", "SYSTEM");
    LogMessage("Monitoring streamer: " + streamerName, "SYSTEM");
    LogMessage("Optimization features: Reusable CURL handle, Early termination, Adaptive intervals", "INFO");
    
    InitializeCurlHandle();
}


StreamMonitor::~StreamMonitor() {
    LogMessage("=== Stream Monitor shutdown ===", "SYSTEM");
    CleanupCurlHandle();
}


void StreamMonitor::InitializeCurlHandle() {
    curlHandle = curl_easy_init();
    
    if (!curlHandle) {
        LogMessage("Failed to initialize cURL handle", "ERROR");
        return;
    }
    
    // Настройка переиспользуемого handle с оптимальными параметрами
    curl_easy_setopt(curlHandle, CURLOPT_USERAGENT, 
                     "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYHOST, 0L);  // Отключаем проверку хоста
    curl_easy_setopt(curlHandle, CURLOPT_TIMEOUT, 20L);  // Сокращенный таймаут
    curl_easy_setopt(curlHandle, CURLOPT_CONNECTTIMEOUT, 5L);  // Быстрое подключение
    curl_easy_setopt(curlHandle, CURLOPT_TCP_KEEPALIVE, 1L);  // Keep-alive для переиспользования
    curl_easy_setopt(curlHandle, CURLOPT_ACCEPT_ENCODING, "");  // Автоматическая декомпрессия
    
    LogMessage("cURL handle initialized with optimizations", "SUCCESS");
}


void StreamMonitor::CleanupCurlHandle() {
    if (curlHandle) {
        curl_easy_cleanup(curlHandle);
        curlHandle = nullptr;
        LogMessage("cURL handle cleaned up", "DEBUG");
    }
}


size_t StreamMonitor::WriteCallbackOptimized(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    CallbackData* data = static_cast<CallbackData*>(userp);
    
    // Оптимизация: ранний выход если маркер уже найден
    if (*(data->foundMarker)) {
        return totalSize;  // Продолжаем получать данные, но не обрабатываем
    }
    
    // Оптимизация: ограничение размера буфера
    if (data->buffer->length() >= data->maxSize) {
        return totalSize;
    }
    
    // Добавляем только необходимую часть
    size_t remainingSpace = data->maxSize - data->buffer->length();
    size_t sizeToAdd = std::min(totalSize, remainingSpace);
    
    data->buffer->append((char*)contents, sizeToAdd);
    
    // Проверяем маркеры в режиме реального времени
    if (data->buffer->find("\"isLiveBroadcast\"") != std::string::npos ||
        data->buffer->find("\"type\":\"live\"") != std::string::npos ||
        data->buffer->find("\"broadcastType\":\"STREAM\"") != std::string::npos) {
        *(data->foundMarker) = true;
    }
    
    return totalSize;
}


std::string StreamMonitor::GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}


long long StreamMonitor::GetUnixTimestamp() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}


void StreamMonitor::LogMessage(const std::string& message, const std::string& level) {
    std::ofstream logFile(LOG_FILE_PATH, std::ios::app);
    
    if (logFile.is_open()) {
        logFile << "[" << GetCurrentTimestamp() << "] "
                << "[" << level << "] "
                << "[StreamMonitor] " << message << std::endl;
        logFile.close();
    } else {
        std::cerr << "ERROR: Unable to open log file: " << LOG_FILE_PATH << std::endl;
    }
}


std::string StreamMonitor::DownloadPageHtmlOptimized() {
    if (!curlHandle) {
        LogMessage("cURL handle not initialized", "ERROR");
        return "";
    }
    
    std::string readBuffer;
    bool foundMarker = false;
    
    CallbackData callbackData = {&readBuffer, &foundMarker, maxHtmlSizeToAnalyze};
    
    std::string url = "https://www.twitch.tv/" + streamerName;
    
    // Устанавливаем URL и callback для текущего запроса
    curl_easy_setopt(curlHandle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, WriteCallbackOptimized);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &callbackData);
    
    LogMessage("Downloading page (optimized, max " + 
               std::to_string(maxHtmlSizeToAnalyze / 1024) + " KB)", "DEBUG");
    
    CURLcode res = curl_easy_perform(curlHandle);
    
    if (res != CURLE_OK) {
        std::string errorMsg = "cURL request failed: " + std::string(curl_easy_strerror(res));
        LogMessage(errorMsg, "ERROR");
        return "";
    }
    
    long httpCode = 0;
    curl_easy_getinfo(curlHandle, CURLINFO_RESPONSE_CODE, &httpCode);
    
    LogMessage("HTTP " + std::to_string(httpCode) + ", Downloaded " + 
               std::to_string(readBuffer.length() / 1024) + " KB" +
               (foundMarker ? " (marker found during download!)" : ""), "DEBUG");
    
    if (httpCode == 200) {
        return readBuffer;
    }
    
    return "";
}


bool StreamMonitor::IsStreamOnlineFast(const std::string& html) {
    if (html.empty()) {
        LogMessage("HTML is empty, assuming offline", "WARNING");
        return false;
    }
    
    // Оптимизация: проверка маркеров с ранним выходом
    // Используем find вместо регулярных выражений для скорости
    
    // Первый и самый надежный маркер
    if (html.find("\"isLiveBroadcast\"") != std::string::npos) {
        LogMessage("Stream ONLINE (found isLiveBroadcast)", "INFO");
        return true;
    }
    
    // Второй маркер
    size_t streamPos = html.find("\"stream\":{");
    if (streamPos != std::string::npos) {
        // Проверяем наличие "type":"live" рядом (в пределах 500 символов)
        size_t searchEnd = std::min(streamPos + 500, html.length());
        std::string streamSection = html.substr(streamPos, searchEnd - streamPos);
        
        if (streamSection.find("\"type\":\"live\"") != std::string::npos) {
            LogMessage("Stream ONLINE (found stream type:live)", "INFO");
            return true;
        }
    }
    
    // Третий маркер
    if (html.find("\"broadcastType\":\"STREAM\"") != std::string::npos) {
        LogMessage("Stream ONLINE (found broadcastType)", "INFO");
        return true;
    }
    
    LogMessage("Stream OFFLINE (no markers found)", "INFO");
    return false;
}


bool StreamMonitor::CheckStreamStatus() {
    std::string html = DownloadPageHtmlOptimized();
    
    if (html.empty()) {
        LogMessage("Could not download page, assuming stream is offline", "WARNING");
        return false;
    }
    
    return IsStreamOnlineFast(html);
}


int StreamMonitor::GetCurrentCheckInterval() {
    // Адаптивный интервал: быстрее проверяем если стрим недавно был онлайн
    long long currentTime = GetUnixTimestamp();
    long long timeSinceLastOnline = currentTime - lastOnlineTimestamp;
    
    if (lastOnlineTimestamp > 0 && timeSinceLastOnline < FAST_MODE_DURATION) {
        LogMessage("Using FAST check interval (" + 
                   std::to_string(CHECK_INTERVAL_FAST_SECONDS) + "s) - stream was recently online", "DEBUG");
        return CHECK_INTERVAL_FAST_SECONDS;
    }
    
    return CHECK_INTERVAL_SECONDS;
}


void StreamMonitor::OpenStreamInBrowser() {
    // Имитация человеческой задержки перед открытием (от 800 до 2000 мс)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(800, 2000);
    int delayMs = distrib(gen);
    
    LogMessage("Waiting " + std::to_string(delayMs) + "ms before opening browser", "INFO");
    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
    
    std::string streamUrl = "https://www.twitch.tv/" + streamerName;
    
    LogMessage("Opening stream in browser: " + streamUrl, "SUCCESS");
    
#ifdef _WIN32
    ShellExecuteA(0, 0, streamUrl.c_str(), 0, 0, SW_SHOW);
#elif __APPLE__
    std::string command = "open \"" + streamUrl + "\"";
    int result = system(command.c_str());
    if (result != 0) {
        LogMessage("Failed to open browser on macOS (code: " + std::to_string(result) + ")", "ERROR");
    }
#else
    std::string command = "xdg-open \"" + streamUrl + "\" 2>/dev/null &";
    int result = system(command.c_str());
    if (result != 0) {
        LogMessage("Failed to open browser on Linux (code: " + std::to_string(result) + ")", "ERROR");
    }
#endif
}


void StreamMonitor::StartMonitoring() {
    LogMessage("Starting optimized monitoring loop", "SYSTEM");
    std::cout << "\n🔍 Monitoring stream: " << streamerName << std::endl;
    std::cout << "⚡ Optimizations enabled:" << std::endl;
    std::cout << "   • Reusable cURL connection" << std::endl;
    std::cout << "   • Early termination on marker detection" << std::endl;
    std::cout << "   • Adaptive check intervals" << std::endl;
    std::cout << "   • Limited HTML parsing (100KB max)" << std::endl;
    std::cout << "⏱️  Normal interval: " << CHECK_INTERVAL_SECONDS << "s" << std::endl;
    std::cout << "⏱️  Fast interval: " << CHECK_INTERVAL_FAST_SECONDS << "s (after recent activity)\n" << std::endl;
    
    while (true) {
        try {
            auto checkStartTime = std::chrono::steady_clock::now();
            
            bool isCurrentlyOnline = CheckStreamStatus();
            
            auto checkEndTime = std::chrono::steady_clock::now();
            auto checkDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                checkEndTime - checkStartTime
            ).count();
            
            // Открываем браузер только при переходе из offline в online
            if (isCurrentlyOnline && !wasOnlineBefore) {
                std::cout << "🎥 Stream just went ONLINE! Opening browser... (check took " 
                         << checkDuration << "ms)" << std::endl;
                LogMessage("Stream status changed: OFFLINE -> ONLINE (check duration: " + 
                          std::to_string(checkDuration) + "ms)", "EVENT");
                OpenStreamInBrowser();
                wasOnlineBefore = true;
                lastOnlineTimestamp = GetUnixTimestamp();
            } 
            else if (!isCurrentlyOnline && wasOnlineBefore) {
                std::cout << "📴 Stream went offline. (check took " << checkDuration << "ms)" << std::endl;
                LogMessage("Stream status changed: ONLINE -> OFFLINE (check duration: " + 
                          std::to_string(checkDuration) + "ms)", "EVENT");
                wasOnlineBefore = false;
                lastOnlineTimestamp = GetUnixTimestamp();
            }
            else if (isCurrentlyOnline) {
                std::cout << "✅ Stream is online (check took " << checkDuration << "ms)" << std::endl;
                lastOnlineTimestamp = GetUnixTimestamp();
            }
            else {
                int interval = GetCurrentCheckInterval();
                std::cout << "⏳ Waiting... [" << GetCurrentTimestamp() 
                         << "] (check took " << checkDuration << "ms, next in " 
                         << interval << "s)" << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::string errorMsg = "Exception in monitoring loop: " + std::string(e.what());
            LogMessage(errorMsg, "CRITICAL");
            std::cerr << "❌ " << errorMsg << std::endl;
        }
        
        // Адаптивное ожидание перед следующей проверкой
        int sleepInterval = GetCurrentCheckInterval();
        LogMessage("Sleeping for " + std::to_string(sleepInterval) + " seconds", "DEBUG");
        std::this_thread::sleep_for(std::chrono::seconds(sleepInterval));
    }
}