#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>


Logger::Logger(const std::string& filePath, bool verbose)
    : logFilePath(filePath), verboseLogging(verbose), minLogLevel(LogLevel::INFO) {
    
    // Извлекаем путь к папке из полного пути
    size_t lastSlash = filePath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        std::string directory = filePath.substr(0, lastSlash);
        
        #ifdef _WIN32
            std::string command = "if not exist \"" + directory + "\" mkdir \"" + directory + "\" >nul 2>&1";
            system(command.c_str());
        #else
            std::string command = "mkdir -p \"" + directory + "\" 2>/dev/null";
            system(command.c_str());
        #endif
    }
    
    // Открываем файл
    logFile.open(logFilePath, std::ios::app);
    
    if (!logFile.is_open()) {
        std::cerr << "ERROR: Cannot open log file: " << logFilePath << std::endl;
        std::cerr << "  Current directory: ";
        #ifdef _WIN32
            system("cd");
        #else
            system("pwd");
        #endif
    } else {
        // Тестовая запись
        logFile << "[" << GetCurrentTimestamp() << "] [SYSTEM] [Logger] Log file opened successfully" << std::endl;
        logFile.flush();
    }
}


Logger::~Logger() {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        logFile << "[" << GetCurrentTimestamp() << "] [SYSTEM] [Logger] Log file closing" << std::endl;
        logFile.close();
    }
}


std::string Logger::GetCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}


std::string Logger::LogLevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:       return "DEBUG";
        case LogLevel::INFO:        return "INFO";
        case LogLevel::WARNING:     return "WARNING";
        case LogLevel::ERROR_LEVEL: return "ERROR";
        case LogLevel::CRITICAL:    return "CRITICAL";
        case LogLevel::SUCCESS:     return "SUCCESS";
        case LogLevel::EVENT:       return "EVENT";
        case LogLevel::SYSTEM:      return "SYSTEM";
        default:                    return "UNKNOWN";
    }
}


bool Logger::ShouldLog(LogLevel level) const {
    if (!verboseLogging && level == LogLevel::DEBUG) {
        return false;
    }
    return static_cast<int>(level) >= static_cast<int>(minLogLevel);
}


void Logger::Log(const std::string& message, LogLevel level, const std::string& module) {
    if (!ShouldLog(level)) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(logMutex);
    
    if (logFile.is_open()) {
        logFile << "[" << GetCurrentTimestamp() << "] "
                << "[" << LogLevelToString(level) << "] "
                << "[" << module << "] "
                << message << std::endl;
        logFile.flush();  // ВАЖНО: принудительная запись
    } else {
        // Если файл не открыт, пишем в консоль
        std::cerr << "[LOG ERROR] File not open, writing to console: " << message << std::endl;
    }
}


void Logger::SetVerbose(bool verbose) {
    verboseLogging = verbose;
}


void Logger::SetMinLogLevel(LogLevel level) {
    minLogLevel = level;
}


void Logger::Debug(const std::string& message, const std::string& module) {
    Log(message, LogLevel::DEBUG, module);
}


void Logger::Info(const std::string& message, const std::string& module) {
    Log(message, LogLevel::INFO, module);
}


void Logger::Warning(const std::string& message, const std::string& module) {
    Log(message, LogLevel::WARNING, module);
}


void Logger::Error(const std::string& message, const std::string& module) {
    Log(message, LogLevel::ERROR_LEVEL, module);
}


void Logger::Critical(const std::string& message, const std::string& module) {
    Log(message, LogLevel::CRITICAL, module);
}


void Logger::Success(const std::string& message, const std::string& module) {
    Log(message, LogLevel::SUCCESS, module);
}


void Logger::Event(const std::string& message, const std::string& module) {
    Log(message, LogLevel::EVENT, module);
}


void Logger::System(const std::string& message, const std::string& module) {
    Log(message, LogLevel::SYSTEM, module);
}