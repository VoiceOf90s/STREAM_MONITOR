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


StreamMonitor::StreamMonitor(const std::string& streamer, const std::string& configPath)
    : streamerName(streamer), wasOnlineBefore(false), curlHandle(nullptr),
      lastOnlineTimestamp(0), browserTabOpened(false), config(configPath) {
    
    // Загрузка конфигурации
    config.Load();
    
    // Применение настроек из конфига
    maxHtmlSizeToAnalyze = config.GetInt("max_html_size", 100000);
    checkInterval = config.GetInt("check_interval", 30);
    checkIntervalFast = config.GetInt("check_interval_fast", 10);
    fastModeDuration = config.GetInt("fast_mode_duration", 300);
    logFilePath = config.GetString("log_file", "stream_monitor.log");
    verboseLogging = config.GetBool("verbose_logging", false);
    useHttp2 = config.GetBool("use_http2", true);
    dnsCacheTimeout = config.GetInt("dns_cache_timeout", 300);
    useHeadRequest = config.GetBool("use_head_request", true);
    openBrowser = config.GetBool("open_browser", true);
    browserDelayMin = config.GetInt("browser_delay_min", 800);
    browserDelayMax = config.GetInt("browser_delay_max", 2000);
    
    // Новые функции v2.2
    enableNotifications = config.GetBool("enable_notifications", true);
    enableStatistics = config.GetBool("enable_statistics", true);
    
    // Инициализация уведомлений
    notification = std::make_unique<Notification>(enableNotifications);
    
    // Инициализация статистики
    statistics = std::make_unique<Statistics>(streamerName, 
                                              "stats_" + streamerName + ".json");
    
    LogMessage("=== Stream Monitor v2.2 initialized ===", "SYSTEM");
    LogMessage("Monitoring streamer: " + streamerName, "SYSTEM");
    LogMessage("Configuration loaded from: " + configPath, "INFO");
    LogMessage("Features: HTTP/2=" + std::string(useHttp2 ? "ON" : "OFF") + 
               ", HEAD=" + std::string(useHeadRequest ? "ON" : "OFF") + 
               ", Notifications=" + std::string(enableNotifications ? "ON" : "OFF") +
               ", Statistics=" + std::string(enableStatistics ? "ON" : "OFF"), "INFO");
    
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
    
    // Базовые настройки
    curl_easy_setopt(curlHandle, CURLOPT_USERAGENT, 
                     "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, 1L);
    
    // SSL настройки из конфига
    curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYPEER, 
                     config.GetBool("ssl_verify_peer", false) ? 1L : 0L);
    curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYHOST, 
                     config.GetBool("ssl_verify_host", false) ? 2L : 0L);
    
    // Таймауты из конфига
    curl_easy_setopt(curlHandle, CURLOPT_TIMEOUT, (long)config.GetInt("timeout", 20));
    curl_easy_setopt(curlHandle, CURLOPT_CONNECTTIMEOUT, (long)config.GetInt("connect_timeout", 10));
    
    // Оптимизации
    curl_easy_setopt(curlHandle, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curlHandle, CURLOPT_ACCEPT_ENCODING, "");
    
    // HTTP/2 поддержка
    if (useHttp2) {
        curl_easy_setopt(curlHandle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
        LogMessage("HTTP/2 enabled", "SUCCESS");
    }
    
    // Кеширование DNS
    if (dnsCacheTimeout > 0) {
        curl_easy_setopt(curlHandle, CURLOPT_DNS_CACHE_TIMEOUT, (long)dnsCacheTimeout);
        LogMessage("DNS cache timeout set to " + std::to_string(dnsCacheTimeout) + " seconds", "SUCCESS");
    }
    
    LogMessage("cURL handle initialized with v2.2 optimizations", "SUCCESS");
}


void StreamMonitor::CleanupCurlHandle() {
    if (curlHandle) {
        curl_easy_cleanup(curlHandle);
        curlHandle = nullptr;
        if (verboseLogging) {
            LogMessage("cURL handle cleaned up", "DEBUG");
        }
    }
}


size_t StreamMonitor::WriteCallbackOptimized(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    CallbackData* data = static_cast<CallbackData*>(userp);
    
    if (*(data->foundMarker)) {
        return totalSize;
    }
    
    if (data->buffer->length() >= data->maxSize) {
        return totalSize;
    }
    
    size_t remainingSpace = data->maxSize - data->buffer->length();
    size_t sizeToAdd = std::min(totalSize, remainingSpace);
    
    data->buffer->append((char*)contents, sizeToAdd);
    
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
    std::ofstream logFile(logFilePath, std::ios::app);
    
    if (logFile.is_open()) {
        logFile << "[" << GetCurrentTimestamp() << "] "
                << "[" << level << "] "
                << "[StreamMonitor] " << message << std::endl;
        logFile.close();
    } else {
        std::cerr << "ERROR: Unable to open log file: " << logFilePath << std::endl;
    }
}


bool StreamMonitor::QuickHeadCheck() {
    if (!curlHandle || !useHeadRequest) {
        return true;
    }
    
    std::string url = "https://www.twitch.tv/" + streamerName;
    
    curl_easy_setopt(curlHandle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curlHandle, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, nullptr);
    
    if (verboseLogging) {
        LogMessage("Performing HEAD request", "DEBUG");
    }
    
    CURLcode res = curl_easy_perform(curlHandle);
    
    curl_easy_setopt(curlHandle, CURLOPT_NOBODY, 0L);
    
    if (res != CURLE_OK) {
        LogMessage("HEAD request failed: " + std::string(curl_easy_strerror(res)), "WARNING");
        return true;
    }
    
    long httpCode = 0;
    curl_easy_getinfo(curlHandle, CURLINFO_RESPONSE_CODE, &httpCode);
    
    if (verboseLogging) {
        LogMessage("HEAD request returned: " + std::to_string(httpCode), "DEBUG");
    }
    
    return (httpCode == 200);
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
    
    curl_easy_setopt(curlHandle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, WriteCallbackOptimized);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &callbackData);
    
    if (verboseLogging) {
        LogMessage("Downloading page (max " + std::to_string(maxHtmlSizeToAnalyze / 1024) + " KB)", "DEBUG");
    }
    
    CURLcode res = curl_easy_perform(curlHandle);
    
    if (res != CURLE_OK) {
        std::string errorMsg = "cURL request failed: " + std::string(curl_easy_strerror(res));
        LogMessage(errorMsg, "ERROR");
        return "";
    }
    
    long httpCode = 0;
    curl_easy_getinfo(curlHandle, CURLINFO_RESPONSE_CODE, &httpCode);
    
    if (verboseLogging) {
        LogMessage("HTTP " + std::to_string(httpCode) + ", Downloaded " + 
                   std::to_string(readBuffer.length() / 1024) + " KB" +
                   (foundMarker ? " (marker found!)" : ""), "DEBUG");
    }
    
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
    
    if (html.find("\"isLiveBroadcast\"") != std::string::npos) {
        LogMessage("Stream ONLINE (found isLiveBroadcast)", "INFO");
        return true;
    }
    
    size_t streamPos = html.find("\"stream\":{");
    if (streamPos != std::string::npos) {
        size_t searchEnd = std::min(streamPos + 500, html.length());
        std::string streamSection = html.substr(streamPos, searchEnd - streamPos);
        
        if (streamSection.find("\"type\":\"live\"") != std::string::npos) {
            LogMessage("Stream ONLINE (found stream type:live)", "INFO");
            return true;
        }
    }
    
    if (html.find("\"broadcastType\":\"STREAM\"") != std::string::npos) {
        LogMessage("Stream ONLINE (found broadcastType)", "INFO");
        return true;
    }
    
    LogMessage("Stream OFFLINE (no markers found)", "INFO");
    return false;
}


bool StreamMonitor::CheckStreamStatus() {
    if (useHeadRequest) {
        if (!QuickHeadCheck()) {
            if (verboseLogging) {
                LogMessage("HEAD check indicates offline, skipping full GET", "DEBUG");
            }
            return false;
        }
    }
    
    std::string html = DownloadPageHtmlOptimized();
    
    if (html.empty()) {
        LogMessage("Could not download page, assuming stream is offline", "WARNING");
        return false;
    }
    
    return IsStreamOnlineFast(html);
}


int StreamMonitor::GetCurrentCheckInterval() {
    long long currentTime = GetUnixTimestamp();
    long long timeSinceLastOnline = currentTime - lastOnlineTimestamp;
    
    if (lastOnlineTimestamp > 0 && timeSinceLastOnline < fastModeDuration) {
        if (verboseLogging) {
            LogMessage("Using FAST check interval (" + 
                       std::to_string(checkIntervalFast) + "s)", "DEBUG");
        }
        return checkIntervalFast;
    }
    
    return checkInterval;
}


void StreamMonitor::OpenStreamInBrowser() {
    if (!openBrowser) {
        LogMessage("Browser opening disabled in config", "INFO");
        return;
    }
    
    // Проверка: не открываем повторно если уже открыто
    if (browserTabOpened) {
        LogMessage("Browser tab already opened, skipping", "INFO");
        std::cout << "  (Browser tab already open)" << std::endl;
        return;
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(browserDelayMin, browserDelayMax);
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

    browserTabOpened = true;  // Помечаем что вкладка открыта
}


void StreamMonitor::CloseBrowserTab() {
    if (!browserTabOpened) {
        return;  // Нечего закрывать
    }
    
    LogMessage("Attempting to close browser tab", "INFO");
    std::cout << "  (Attempting to close browser tab...)" << std::endl;
    
#ifdef _WIN32
    // Windows: Закрываем вкладку Twitch через PowerShell
    std::string psCommand = "powershell -Command \""
        "$wshell = New-Object -ComObject wscript.shell; "
        "$wshell.AppActivate('twitch.tv/" + streamerName + "'); "
        "Start-Sleep -Milliseconds 500; "
        "$wshell.SendKeys('^w')\"";  // Ctrl+W для закрытия вкладки
    
    int result = system(psCommand.c_str());
    if (result == 0) {
        LogMessage("Browser tab close command sent", "SUCCESS");
    } else {
        LogMessage("Could not close browser tab automatically", "WARNING");
    }
#elif __APPLE__
    // macOS: AppleScript для закрытия вкладки
    std::string command = "osascript -e 'tell application \"Safari\" to close (first tab of first window whose URL contains \"twitch.tv/" + streamerName + "\")'";
    int result = system(command.c_str());
    if (result != 0) {
        // Пробуем Chrome
        command = "osascript -e 'tell application \"Google Chrome\" to close (first tab of first window whose URL contains \"twitch.tv/" + streamerName + "\")'";
        system(command.c_str());
    }
#else
    // Linux: сложно автоматически, просто уведомляем
    LogMessage("Auto-close not supported on Linux, please close manually", "INFO");
    std::cout << "  (Please close the browser tab manually)" << std::endl;
#endif

    browserTabOpened = false;  // Сбрасываем флаг
}


void StreamMonitor::StartMonitoring() {
    LogMessage("Starting monitoring loop (v2.2)", "SYSTEM");
    
    std::cout << "\n+=======================================+" << std::endl;
    std::cout << "|   Twitch Stream Monitor v2.2          |" << std::endl;
    std::cout << "|   Notifications | Stats | Multi       |" << std::endl;
    std::cout << "+=======================================+" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Monitoring: " << streamerName << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "  - HTTP/2: " << (useHttp2 ? "ON" : "OFF") << std::endl;
    std::cout << "  - HEAD requests: " << (useHeadRequest ? "ON" : "OFF") << std::endl;
    std::cout << "  - DNS cache: " << dnsCacheTimeout << "s" << std::endl;
    std::cout << "  - Notifications: " << (enableNotifications ? "ON" : "OFF") << std::endl;
    std::cout << "  - Statistics: " << (enableStatistics ? "ON" : "OFF") << std::endl;
    std::cout << "  - Check intervals: " << checkInterval << "s / " << checkIntervalFast << "s" << std::endl;
    std::cout << std::endl;
    
    while (true) {
        try {
            auto checkStartTime = std::chrono::steady_clock::now();
            
            bool isCurrentlyOnline = CheckStreamStatus();
            
            auto checkEndTime = std::chrono::steady_clock::now();
            auto checkDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                checkEndTime - checkStartTime
            ).count();
            
            // Запись статистики
            if (enableStatistics) {
                statistics->RecordCheck(checkDuration);
            }
            
            if (isCurrentlyOnline && !wasOnlineBefore) {
                std::cout << "[ONLINE] " << streamerName << " started streaming! (" 
                         << checkDuration << "ms)" << std::endl;
                LogMessage("Stream status changed: OFFLINE -> ONLINE (check: " + 
                          std::to_string(checkDuration) + "ms)", "EVENT");
                
                // Уведомление
                if (enableNotifications) {
                    notification->NotifyStreamOnline(streamerName);
                }
                
                // Статистика
                if (enableStatistics) {
                    statistics->RecordStreamOnline();
                }
                
                OpenStreamInBrowser();
                wasOnlineBefore = true;
                lastOnlineTimestamp = GetUnixTimestamp();
            } 
            else if (!isCurrentlyOnline && wasOnlineBefore) {
                std::cout << "[OFFLINE] " << streamerName << " ended stream (" 
                         << checkDuration << "ms)" << std::endl;
                LogMessage("Stream status changed: ONLINE -> OFFLINE (check: " + 
                          std::to_string(checkDuration) + "ms)", "EVENT");
                
                // Уведомление
                if (enableNotifications) {
                    notification->NotifyStreamOffline(streamerName);
                }
                
                // Статистика
                if (enableStatistics) {
                    statistics->RecordStreamOffline();
                }
                
                // NEW: Закрываем вкладку браузера
                CloseBrowserTab();
                
                wasOnlineBefore = false;
                lastOnlineTimestamp = GetUnixTimestamp();
            }
            else if (isCurrentlyOnline) {
                std::cout << "[OK] " << streamerName << " online (" 
                         << checkDuration << "ms)" << std::endl;
                lastOnlineTimestamp = GetUnixTimestamp();
            }
            else {
                int interval = GetCurrentCheckInterval();
                std::cout << "[WAIT] " << streamerName << " checking... (" 
                         << checkDuration << "ms, next in " << interval << "s)" << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::string errorMsg = "Exception in monitoring loop: " + std::string(e.what());
            LogMessage(errorMsg, "CRITICAL");
            std::cerr << "[ERROR] " << streamerName << ": " << errorMsg << std::endl;
        }
        
        int sleepInterval = GetCurrentCheckInterval();
        if (verboseLogging) {
            LogMessage("Sleeping for " + std::to_string(sleepInterval) + " seconds", "DEBUG");
        }
        std::this_thread::sleep_for(std::chrono::seconds(sleepInterval));
    }
}


void StreamMonitor::EnableNotifications(bool enable) {
    enableNotifications = enable;
    if (notification) {
        notification->SetEnabled(enable);
    }
    LogMessage("Notifications " + std::string(enable ? "enabled" : "disabled"), "INFO");
}


void StreamMonitor::EnableStatistics(bool enable) {
    enableStatistics = enable;
    LogMessage("Statistics " + std::string(enable ? "enabled" : "disabled"), "INFO");
}


void StreamMonitor::ShowStatistics() const {
    if (statistics) {
        statistics->PrintSummary();
    } else {
        std::cout << "Statistics not available" << std::endl;
    }
}