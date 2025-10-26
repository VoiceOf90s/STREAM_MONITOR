#ifndef STREAM_MONITOR_H
#define STREAM_MONITOR_H

#include <string>
#include <curl/curl.h>
#include <memory>
#include "Config.h"
#include "Notification.h"
#include "Statistics.h"


class StreamMonitor {
private:
    std::string streamerName;
    bool wasOnlineBefore;
    CURL* curlHandle;
    long long lastOnlineTimestamp;
    bool browserTabOpened;  // NEW: отслеживание открытой вкладки
    
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
    bool autoCloseTab;  // NEW: автозакрытие вкладки
    
    // Новые функции v2.2
    std::unique_ptr<Notification> notification;
    std::unique_ptr<Statistics> statistics;
    bool enableNotifications;
    bool enableStatistics;
    
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
    void CloseBrowserTab();  // NEW: закрытие вкладки
    void InitializeCurlHandle();
    void CleanupCurlHandle();

public:
    StreamMonitor(const std::string& streamer, const std::string& configPath = "config.ini");
    ~StreamMonitor();
    
    void StartMonitoring();
    
    // Получение текущей конфигурации
    const Config& GetConfig() const { return config; }
    
    // Получение статистики (новая функция v2.2)
    const Statistics* GetStatistics() const { return statistics.get(); }
    
    // Управление уведомлениями (новая функция v2.2)
    void EnableNotifications(bool enable);
    
    // Управление статистикой (новая функция v2.2)
    void EnableStatistics(bool enable);
    
    // Показать статистику (новая функция v2.2)
    void ShowStatistics() const;
};

#endif // STREAM_MONITOR_H