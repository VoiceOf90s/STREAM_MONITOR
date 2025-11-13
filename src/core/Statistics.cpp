#include "Statistics.h"
#include "StringUtils.h"
#include "Constants.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>


Statistics::Statistics(const std::string& streamer, const std::string& statsFile)
    : streamerName(streamer), statsFilePath(statsFile),
      totalChecks(0), onlineDetections(0), offlineDetections(0),
      totalCheckTime(0), fastestCheck(999999), slowestCheck(0),
      currentSessionStart(0) {
    
    // Извлекаем путь к папке
    size_t lastSlash = statsFile.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        std::string directory = statsFile.substr(0, lastSlash);
        
        #ifdef _WIN32
            std::string command = "if not exist \"" + directory + "\" mkdir \"" + directory + "\" >nul 2>&1";
            system(command.c_str());
        #else
            std::string command = "mkdir -p \"" + directory + "\" 2>/dev/null";
            system(command.c_str());
        #endif
        
        std::cout << "[Statistics] Created directory: " << directory << std::endl;
    }
    
    LoadFromFile();
}


Statistics::~Statistics() {
    SaveToFile();
}


void Statistics::TrimOldSessions() {
    // Вызывается внутри locked секции
    if (sessions.size() > MAX_SESSIONS) {
        // Удаляем самые старые сессии
        size_t toRemove = sessions.size() - MAX_SESSIONS;
        sessions.erase(sessions.begin(), sessions.begin() + toRemove);
    }
}


void Statistics::RecordCheck(long long checkTimeMs) {
    std::lock_guard<std::mutex> lock(statsMutex);
    
    totalChecks++;
    totalCheckTime += checkTimeMs;
    
    if (checkTimeMs < fastestCheck) {
        fastestCheck = checkTimeMs;
    }
    if (checkTimeMs > slowestCheck) {
        slowestCheck = checkTimeMs;
    }
}


void Statistics::RecordStreamOnline() {
    std::lock_guard<std::mutex> lock(statsMutex);
    
    onlineDetections++;
    
    if (currentSessionStart == 0) {
        currentSessionStart = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }
}


void Statistics::RecordStreamOffline() {
    std::lock_guard<std::mutex> lock(statsMutex);
    
    offlineDetections++;
    
    if (currentSessionStart > 0) {
        long long endTime = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        
        StreamSession session(
            currentSessionStart,
            endTime,
            static_cast<int>(endTime - currentSessionStart)
        );
        
        sessions.push_back(session);
        currentSessionStart = 0;
        
        TrimOldSessions();
        
        SaveToFile();  // Автосохранение после каждой сессии
    }
}


int Statistics::GetTotalChecks() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    return totalChecks;
}


int Statistics::GetOnlineDetections() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    return onlineDetections;
}


int Statistics::GetOfflineDetections() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    return offlineDetections;
}


long long Statistics::GetAverageCheckTime() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    if (totalChecks == 0) return 0;
    return totalCheckTime / totalChecks;
}


long long Statistics::GetFastestCheck() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    return fastestCheck == 999999 ? 0 : fastestCheck;
}


long long Statistics::GetSlowestCheck() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    return slowestCheck;
}


int Statistics::GetTotalStreams() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    return static_cast<int>(sessions.size());
}


long long Statistics::GetTotalStreamTime() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    
    long long total = 0;
    for (const auto& session : sessions) {
        total += session.duration;
    }
    return total;
}


long long Statistics::GetAverageStreamDuration() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    
    if (sessions.empty()) return 0;
    return GetTotalStreamTime() / static_cast<long long>(sessions.size());
}


long long Statistics::GetLongestStream() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    
    if (sessions.empty()) return 0;
    
    auto maxSession = std::max_element(sessions.begin(), sessions.end(),
        [](const StreamSession& a, const StreamSession& b) {
            return a.duration < b.duration;
        });
    
    return maxSession->duration;
}


long long Statistics::GetShortestStream() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    
    if (sessions.empty()) return 0;
    
    auto minSession = std::min_element(sessions.begin(), sessions.end(),
        [](const StreamSession& a, const StreamSession& b) {
            return a.duration < b.duration;
        });
    
    return minSession->duration;
}


std::string Statistics::GetSummaryString() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    
    std::stringstream ss;
    
    ss << "\n╔═══════════════════════════════════════════════════════════════╗\n";
    ss << "║           STATISTICS FOR " << std::left << std::setw(37) << streamerName << "║\n";
    ss << "╠═══════════════════════════════════════════════════════════════╣\n";
    
    ss << "║ Monitoring Statistics:                                        ║\n";
    ss << "║   Total checks performed:      " << std::right << std::setw(28) << totalChecks << " ║\n";
    ss << "║   Online detections:           " << std::right << std::setw(28) << onlineDetections << " ║\n";
    ss << "║   Offline detections:          " << std::right << std::setw(28) << offlineDetections << " ║\n";
    ss << "║                                                               ║\n";
    
    ss << "║ Check Performance:                                            ║\n";
    ss << "║   Average check time:          " << std::right << std::setw(23) << GetAverageCheckTime() << " ms ║\n";
    ss << "║   Fastest check:               " << std::right << std::setw(23) << (fastestCheck == 999999 ? 0 : fastestCheck) << " ms ║\n";
    ss << "║   Slowest check:               " << std::right << std::setw(23) << slowestCheck << " ms ║\n";
    ss << "║                                                               ║\n";
    
    ss << "║ Stream Sessions:                                              ║\n";
    ss << "║   Total streams recorded:      " << std::right << std::setw(28) << sessions.size() << " ║\n";
    
    if (!sessions.empty()) {
        long long totalTime = 0;
        for (const auto& session : sessions) {
            totalTime += session.duration;
        }
        
        int hours = totalTime / 3600;
        int minutes = (totalTime % 3600) / 60;
        
        ss << "║   Total stream time:           " << std::right << std::setw(18) << hours << "h " << minutes << "m ║\n";
        
        long long avgDuration = sessions.empty() ? 0 : totalTime / static_cast<long long>(sessions.size());
        int avgHours = avgDuration / 3600;
        int avgMinutes = (avgDuration % 3600) / 60;
        
        ss << "║   Average stream duration:     " << std::right << std::setw(18) << avgHours << "h " << avgMinutes << "m ║\n";
        
        auto maxSession = std::max_element(sessions.begin(), sessions.end(),
            [](const StreamSession& a, const StreamSession& b) {
                return a.duration < b.duration;
            });
        
        if (maxSession != sessions.end()) {
            int longHours = maxSession->duration / 3600;
            int longMinutes = (maxSession->duration % 3600) / 60;
            ss << "║   Longest stream:              " << std::right << std::setw(18) << longHours << "h " << longMinutes << "m ║\n";
        }
        
        auto minSession = std::min_element(sessions.begin(), sessions.end(),
            [](const StreamSession& a, const StreamSession& b) {
                return a.duration < b.duration;
            });
        
        if (minSession != sessions.end()) {
            int shortHours = minSession->duration / 3600;
            int shortMinutes = (minSession->duration % 3600) / 60;
            ss << "║   Shortest stream:             " << std::right << std::setw(18) << shortHours << "h " << shortMinutes << "m ║\n";
        }
    }
    
    ss << "╚═══════════════════════════════════════════════════════════════╝\n";
    
    return ss.str();
}


void Statistics::PrintSummary() const {
    std::cout << GetSummaryString();
}


void Statistics::Reset() {
    std::lock_guard<std::mutex> lock(statsMutex);
    
    totalChecks = 0;
    onlineDetections = 0;
    offlineDetections = 0;
    totalCheckTime = 0;
    fastestCheck = 999999;
    slowestCheck = 0;
    sessions.clear();
    currentSessionStart = 0;
    
    SaveToFile();
}


void Statistics::Save() {
    SaveToFile();
}


bool Statistics::SaveToFile() {
    std::ofstream file(statsFilePath);
    
    if (!file.is_open()) {
        std::cerr << "Warning: Cannot save statistics to " << statsFilePath << std::endl;
        std::cerr << "  Attempted path: " << statsFilePath << std::endl;
        return false;
    }
    
    std::cout << "[Statistics] Saving to: " << statsFilePath << std::endl;
    
    // Простой JSON формат
    file << "{\n";
    file << "  \"streamer\": \"" << streamerName << "\",\n";
    file << "  \"total_checks\": " << totalChecks << ",\n";
    file << "  \"online_detections\": " << onlineDetections << ",\n";
    file << "  \"offline_detections\": " << offlineDetections << ",\n";
    file << "  \"total_check_time\": " << totalCheckTime << ",\n";
    file << "  \"fastest_check\": " << (fastestCheck == 999999 ? 0 : fastestCheck) << ",\n";
    file << "  \"slowest_check\": " << slowestCheck << ",\n";
    file << "  \"sessions\": [\n";
    
    for (size_t i = 0; i < sessions.size(); i++) {
        file << "    {\n";
        file << "      \"start_time\": " << sessions[i].startTime << ",\n";
        file << "      \"end_time\": " << sessions[i].endTime << ",\n";
        file << "      \"duration\": " << sessions[i].duration << "\n";
        file << "    }";
        if (i < sessions.size() - 1) {
            file << ",";
        }
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    file.close();
    
    std::cout << "[Statistics] File saved successfully" << std::endl;
    return true;
}


bool Statistics::LoadFromFile() {
    std::ifstream file(statsFilePath);
    
    if (!file.is_open()) {
        // Файл не существует - это нормально при первом запуске
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Парсим основные поля (упрощенный парсинг)
        try {
            if (line.find("\"total_checks\":") != std::string::npos) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    totalChecks = StringUtils::SafeStoi(line.substr(pos + 1), 0);
                }
            }
            else if (line.find("\"online_detections\":") != std::string::npos) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    onlineDetections = StringUtils::SafeStoi(line.substr(pos + 1), 0);
                }
            }
            else if (line.find("\"offline_detections\":") != std::string::npos) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    offlineDetections = StringUtils::SafeStoi(line.substr(pos + 1), 0);
                }
            }
            else if (line.find("\"total_check_time\":") != std::string::npos) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    totalCheckTime = StringUtils::SafeStoll(line.substr(pos + 1), 0);
                }
            }
            else if (line.find("\"fastest_check\":") != std::string::npos) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    fastestCheck = StringUtils::SafeStoll(line.substr(pos + 1), 999999);
                }
            }
            else if (line.find("\"slowest_check\":") != std::string::npos) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    slowestCheck = StringUtils::SafeStoll(line.substr(pos + 1), 0);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Warning: Error parsing statistics file: " << e.what() << std::endl;
        }
    }
    
    file.close();
    return true;
}