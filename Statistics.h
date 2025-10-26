#ifndef STATISTICS_H
#define STATISTICS_H

#include <string>
#include <vector>
#include <map>
#include <chrono>


// Структура для хранения информации о сессии стрима
struct StreamSession {
    long long startTime;
    long long endTime;
    int duration;  // В секундах
};


// Класс для сбора и хранения статистики
class Statistics {
private:
    std::string streamerName;
    std::string statsFilePath;
    
    // Счетчики
    int totalChecks;
    int onlineDetections;
    int offlineDetections;
    long long totalCheckTime;  // В миллисекундах
    long long fastestCheck;
    long long slowestCheck;
    
    // Сессии стримов
    std::vector<StreamSession> sessions;
    long long currentSessionStart;
    
    // Сохранение/загрузка статистики
    bool SaveToFile();
    bool LoadFromFile();

public:
    Statistics(const std::string& streamer, const std::string& statsFile = "statistics.json");
    ~Statistics();
    
    // Обновление счетчиков
    void RecordCheck(long long checkTimeMs);
    void RecordStreamOnline();
    void RecordStreamOffline();
    
    // Получение статистики
    int GetTotalChecks() const { return totalChecks; }
    int GetOnlineDetections() const { return onlineDetections; }
    int GetOfflineDetections() const { return offlineDetections; }
    long long GetAverageCheckTime() const;
    long long GetFastestCheck() const { return fastestCheck; }
    long long GetSlowestCheck() const { return slowestCheck; }
    
    // Статистика по стримам
    int GetTotalStreams() const { return sessions.size(); }
    long long GetTotalStreamTime() const;
    long long GetAverageStreamDuration() const;
    long long GetLongestStream() const;
    long long GetShortestStream() const;
    
    // Вывод статистики
    void PrintSummary() const;
    std::string GetSummaryString() const;
    
    // Сброс статистики
    void Reset();
    
    // Сохранение
    void Save();
};

#endif // STATISTICS_H