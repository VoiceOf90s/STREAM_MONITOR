#ifndef STREAM_MONITOR_H
#define STREAM_MONITOR_H

#include <string>
#include <curl/curl.h>
#include <memory>


// Глобальные константы
const int CHECK_INTERVAL_SECONDS = 30;
const int CHECK_INTERVAL_FAST_SECONDS = 10;  // Быстрый режим при недавнем онлайн
const int FAST_MODE_DURATION = 300;  // 5 минут быстрого режима после оффлайн
const std::string LOG_FILE_PATH = "stream_monitor.log";


class StreamMonitor {
private:
    std::string streamerName;
    bool wasOnlineBefore;
    CURL* curlHandle;  // Переиспользуемый handle
    
    // Оптимизация: время последнего онлайн статуса
    long long lastOnlineTimestamp;
    
    // Оптимизация: кеширование части HTML для уменьшения парсинга
    size_t maxHtmlSizeToAnalyze;
    
    // Callback для обработки ответа от cURL с оптимизацией
    static size_t WriteCallbackOptimized(void* contents, size_t size, size_t nmemb, void* userp);
    
    // Структура для передачи данных в callback
    struct CallbackData {
        std::string* buffer;
        bool* foundMarker;
        size_t maxSize;
    };
    
    // Логирование в файл с временной меткой
    void LogMessage(const std::string& message, const std::string& level = "INFO");
    
    // Получение текущего времени в читаемом формате
    std::string GetCurrentTimestamp();
    
    // Получение Unix timestamp
    long long GetUnixTimestamp();
    
    // Загрузка HTML страницы стримера (оптимизированная версия)
    std::string DownloadPageHtmlOptimized();
    
    // Быстрая проверка онлайн статуса (останавливается при первом найденном маркере)
    bool IsStreamOnlineFast(const std::string& html);
    
    // Проверка статуса стрима (главный метод)
    bool CheckStreamStatus();
    
    // Определение текущего интервала проверки (адаптивный)
    int GetCurrentCheckInterval();
    
    // Открытие браузера с задержкой для имитации человеческого поведения
    void OpenStreamInBrowser();
    
    // Инициализация cURL handle с оптимальными настройками
    void InitializeCurlHandle();
    
    // Очистка ресурсов cURL
    void CleanupCurlHandle();

public:
    StreamMonitor(const std::string& streamer);
    ~StreamMonitor();
    
    // Основной цикл мониторинга
    void StartMonitoring();
};

#endif // STREAM_MONITOR_H