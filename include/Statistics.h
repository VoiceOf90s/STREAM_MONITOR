#ifndef STATISTICS_H
#define STATISTICS_H

#include <string>
#include <vector>
#include <chrono>
#include <mutex>


// Структура для хранения информации о сессии стрима
struct StreamSession {
    long long startTime;
    long long endTime;
    int duration;  // В секундах
    
    StreamSession() : startTime(0), endTime(0), duration(0) {}
    StreamSession(long long start, long long end, int dur)
        : startTime(start), endTime(end), duration(dur) {}
};


// Класс для сбора и хранения статистики (THREAD-SAFE)
class Statistics {
private:
    std::string streamerName;
    std::string statsFilePath;
    mutable std::mutex statsMutex;  // Для thread-safety
    
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
    
    // Ограничение размера истории
    static const size_t MAX_SESSIONS = 1000;
    
    // Сохранение/загрузка статистики
    bool SaveToFile();
    bool LoadFromFile();
    
    // Очистка старых сессий при превышении лимита
    void TrimOldSessions();


public:
    Statistics(const std::string& streamer, 
               const std::string& statsFile = "statistics.json");
    ~Statistics();
    
    // Запрет копирования
    Statistics(const Statistics&) = delete;
    Statistics& operator=(const Statistics&) = delete;
    
    // Обновление счетчиков (thread-safe)
    void RecordCheck(long long checkTimeMs);
    void RecordStreamOnline();
    void RecordStreamOffline();
    
    // Получение статистики (thread-safe)
    int GetTotalChecks() const;
    int GetOnlineDetections() const;
    int GetOfflineDetections() const;
    long long GetAverageCheckTime() const;
    long long GetFastestCheck() const;
    long long GetSlowestCheck() const;
    
    // Статистика по стримам (thread-safe)
    int GetTotalStreams() const;
    long long GetTotalStreamTime() const;
    long long GetAverageStreamDuration() const;
    long long GetLongestStream() const;
    long long GetShortestStream() const;
    
    // Вывод статистики
    void PrintSummary() const;
    std::string GetSummaryString() const;
    
    // Сброс статистики
    void Reset();
    
    // Принудительное сохранение
    void Save();
};

#endif // STATISTICS_H