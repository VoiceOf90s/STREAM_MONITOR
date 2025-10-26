#ifndef STREAM_MONITOR_H
#define STREAM_MONITOR_H

#include <string>
#include <curl/curl.h>
#include <memory>
#include "Config.h"


class StreamMonitor {
private:
    std::string streamerName;
    bool wasOnlineBefore;
    CURL* curlHandle;
    long long lastOnlineTimestamp;
    
    // Конфигурация
    Config config;
    size_t maxHtmlSizeToAnalyze;
    int checkInterval;
    int checkIntervalFast;
    int fastModeDuration;
    std::string logFilePath;
    bool verboseLogging;
    bool useHttp2;
    int dnsCacheTimeout;
    bool useHeadRequest;
    bool openBrowser;
    int browserDelayMin;
    int browserDelayMax;
    
    // Callback для обработки ответа от cURL
    static size_t WriteCallbackOptimized(void* contents, size_t size, size_t nmemb, void* userp);
    
    // Структура для передачи данных в callback
    struct CallbackData {
        std::string* buffer;
        bool* foundMarker;
        size_t maxSize;
    };
    
    void LogMessage(const std::string& message, const std::string& level = "INFO");
    std::string GetCurrentTimestamp();
    long long GetUnixTimestamp();
    
    // HEAD запрос для быстрой предпроверки
    bool QuickHeadCheck();
    
    // Загрузка HTML страницы стримера
    std::string DownloadPageHtmlOptimized();
    
    bool IsStreamOnlineFast(const std::string& html);
    bool CheckStreamStatus();
    int GetCurrentCheckInterval();
    void OpenStreamInBrowser();
    void InitializeCurlHandle();
    void CleanupCurlHandle();

public:
    StreamMonitor(const std::string& streamer, const std::string& configPath = "config.ini");
    ~StreamMonitor();
    
    void StartMonitoring();
    
    // Получение текущей конфигурации
    const Config& GetConfig() const { return config; }
};

#endif // STREAM_MONITOR_H