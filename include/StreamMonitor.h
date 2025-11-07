#ifndef STREAM_MONITOR_H
#define STREAM_MONITOR_H

#include <string>
#include <memory>
#include <atomic>
#include "Config.h"
#include "Logger.h"
#include "Notification.h"
#include "Statistics.h"
#include "WebScraper.h"
#include "BrowserController.h"


class StreamMonitor {
private:
    std::string streamerName;
    std::atomic<bool> wasOnlineBefore;
    std::atomic<bool> shouldStop;  // Для graceful shutdown
    long long lastOnlineTimestamp;
    
    // Зависимости (Dependency Injection)
    std::shared_ptr<Logger> logger;
    std::unique_ptr<Config> config;
    std::unique_ptr<Notification> notification;
    std::unique_ptr<Statistics> statistics;
    std::unique_ptr<WebScraper> webScraper;
    std::unique_ptr<BrowserController> browserController;
    
    // Настройки из конфига
    int checkInterval;
    int checkIntervalFast;
    int fastModeDuration;
    bool enableNotifications;
    bool enableStatistics;
    
    // Вычисление текущего интервала проверки
    int GetCurrentCheckInterval();
    
    // Получение Unix timestamp
    long long GetUnixTimestamp() const;
    
    // Обработка изменения статуса
    void HandleStreamOnline();
    void HandleStreamOffline();


public:
    StreamMonitor(const std::string& streamer, 
                  const std::string& configPath = "config.ini");
    ~StreamMonitor();
    
    // Запрет копирования
    StreamMonitor(const StreamMonitor&) = delete;
    StreamMonitor& operator=(const StreamMonitor&) = delete;
    
    // Основной метод мониторинга
    void StartMonitoring();
    
    // Graceful shutdown
    void Stop();
    bool IsStopped() const { return shouldStop.load(); }
    
    // Получение текущей конфигурации
    const Config& GetConfig() const { return *config; }
    
    // Получение статистики
    const Statistics* GetStatistics() const { return statistics.get(); }
    
    // Управление уведомлениями
    void EnableNotifications(bool enable);
    
    // Управление статистикой
    void EnableStatistics(bool enable);
    
    // Показать статистику
    void ShowStatistics() const;
    
    // Получение имени стримера
    std::string GetStreamerName() const { return streamerName; }
};

#endif // STREAM_MONITOR_H