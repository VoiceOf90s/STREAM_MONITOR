#ifndef MULTI_STREAM_MONITOR_H
#define MULTI_STREAM_MONITOR_H

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include "StreamMonitor.h"
#include "Config.h"


// Структура для хранения информации о мониторе
struct MonitorInfo {
    std::string streamerName;
    std::unique_ptr<StreamMonitor> monitor;
    std::unique_ptr<std::thread> thread;
    bool isRunning;
};


// Класс для параллельного мониторинга нескольких стримеров
class MultiStreamMonitor {
private:
    std::vector<MonitorInfo> monitors;
    std::mutex mutex;
    Config config;
    bool isRunning;
    
    // Поток для каждого монитора
    void MonitorThreadFunction(StreamMonitor* monitor);

public:
    MultiStreamMonitor(const std::string& configPath = "config.ini");
    ~MultiStreamMonitor();
    
    // Добавить стримера для мониторинга
    bool AddStreamer(const std::string& streamerName);
    
    // Удалить стримера из мониторинга
    bool RemoveStreamer(const std::string& streamerName);
    
    // Получить список отслеживаемых стримеров
    std::vector<std::string> GetStreamers() const;
    
    // Запустить мониторинг всех стримеров
    void StartAll();
    
    // Остановить мониторинг всех стримеров
    void StopAll();
    
    // Показать статус всех стримеров
    void PrintStatus() const;
    
    // Загрузить список стримеров из файла
    bool LoadStreamersFromFile(const std::string& filePath);
};

#endif // MULTI_STREAM_MONITOR_H