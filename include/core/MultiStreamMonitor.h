#ifndef MULTI_STREAM_MONITOR_H
#define MULTI_STREAM_MONITOR_H

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <atomic>
#include "StreamMonitor.h"
#include "Config.h"
#include "Logger.h"


// Структура для хранения информации о мониторе
struct MonitorInfo {
    std::string streamerName;
    std::unique_ptr<StreamMonitor> monitor;
    std::unique_ptr<std::thread> thread;
    bool isRunning;  // ← ИЗМЕНЕНО: обычный bool вместо std::atomic<bool>
    
    // Конструктор по умолчанию
    MonitorInfo() : isRunning(false) {}
    
    // Запрет копирования
    MonitorInfo(const MonitorInfo&) = delete;
    MonitorInfo& operator=(const MonitorInfo&) = delete;
    
    // Разрешаем перемещение
    MonitorInfo(MonitorInfo&& other) noexcept
        : streamerName(std::move(other.streamerName)),
          monitor(std::move(other.monitor)),
          thread(std::move(other.thread)),
          isRunning(other.isRunning) {
        other.isRunning = false;
    }
    
    MonitorInfo& operator=(MonitorInfo&& other) noexcept {
        if (this != &other) {
            streamerName = std::move(other.streamerName);
            monitor = std::move(other.monitor);
            thread = std::move(other.thread);
            isRunning = other.isRunning;
            other.isRunning = false;
        }
        return *this;
    }
};


// Класс для параллельного мониторинга нескольких стримеров
class MultiStreamMonitor {
private:
    std::vector<MonitorInfo> monitors;
    mutable std::mutex monitorsMutex;
    std::shared_ptr<Config> config;
    std::shared_ptr<Logger> logger;
    std::atomic<bool> isRunning;
    
    void MonitorThreadFunction(StreamMonitor* monitor);

public:
    MultiStreamMonitor(const std::string& configPath = "config.ini");
    ~MultiStreamMonitor();
    
    MultiStreamMonitor(const MultiStreamMonitor&) = delete;
    MultiStreamMonitor& operator=(const MultiStreamMonitor&) = delete;
    
    bool AddStreamer(const std::string& streamerName);
    bool RemoveStreamer(const std::string& streamerName);
    std::vector<std::string> GetStreamers() const;
    
    void StartAll();
    void StopAll();
    void PrintStatus() const;
    
    bool LoadStreamersFromFile(const std::string& filePath);
    bool IsRunning() const { return isRunning.load(); }
};

#endif // MULTI_STREAM_MONITOR_H