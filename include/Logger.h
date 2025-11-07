#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <memory>


enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR_LEVEL,  // ERROR конфликтует с макросом Windows
    CRITICAL,
    SUCCESS,
    EVENT,
    SYSTEM
};


class Logger {
private:
    std::string logFilePath;
    std::ofstream logFile;
    mutable std::mutex logMutex;
    bool verboseLogging;
    LogLevel minLogLevel;
    
    std::string GetCurrentTimestamp() const;
    std::string LogLevelToString(LogLevel level) const;
    bool ShouldLog(LogLevel level) const;


public:
    Logger(const std::string& filePath = "stream_monitor.log", 
           bool verbose = false);
    ~Logger();
    
    void Log(const std::string& message, 
             LogLevel level = LogLevel::INFO, 
             const std::string& module = "General");
    
    void SetVerbose(bool verbose);
    void SetMinLogLevel(LogLevel level);
    
    // Convenience methods
    void Debug(const std::string& message, const std::string& module = "General");
    void Info(const std::string& message, const std::string& module = "General");
    void Warning(const std::string& message, const std::string& module = "General");
    void Error(const std::string& message, const std::string& module = "General");
    void Critical(const std::string& message, const std::string& module = "General");
    void Success(const std::string& message, const std::string& module = "General");
    void Event(const std::string& message, const std::string& module = "General");
    void System(const std::string& message, const std::string& module = "General");
};

#endif // LOGGER_H