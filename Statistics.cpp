#include "Statistics.h"
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
    
    LoadFromFile();
}


Statistics::~Statistics() {
    SaveToFile();
}


void Statistics::RecordCheck(long long checkTimeMs) {
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
    onlineDetections++;
    
    if (currentSessionStart == 0) {
        currentSessionStart = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }
}


void Statistics::RecordStreamOffline() {
    offlineDetections++;
    
    if (currentSessionStart > 0) {
        long long endTime = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        
        StreamSession session;
        session.startTime = currentSessionStart;
        session.endTime = endTime;
        session.duration = static_cast<int>(endTime - currentSessionStart);
        
        sessions.push_back(session);
        currentSessionStart = 0;
        
        SaveToFile();  // Автосохранение после каждой сессии
    }
}


long long Statistics::GetAverageCheckTime() const {
    if (totalChecks == 0) return 0;
    return totalCheckTime / totalChecks;
}


long long Statistics::GetTotalStreamTime() const {
    long long total = 0;
    for (const auto& session : sessions) {
        total += session.duration;
    }
    return total;
}


long long Statistics::GetAverageStreamDuration() const {
    if (sessions.empty()) return 0;
    return GetTotalStreamTime() / sessions.size();
}


long long Statistics::GetLongestStream() const {
    if (sessions.empty()) return 0;
    
    auto maxSession = std::max_element(sessions.begin(), sessions.end(),
        [](const StreamSession& a, const StreamSession& b) {
            return a.duration < b.duration;
        });
    
    return maxSession->duration;
}


long long Statistics::GetShortestStream() const {
    if (sessions.empty()) return 0;
    
    auto minSession = std::min_element(sessions.begin(), sessions.end(),
        [](const StreamSession& a, const StreamSession& b) {
            return a.duration < b.duration;
        });
    
    return minSession->duration;
}


std::string Statistics::GetSummaryString() const {
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
    ss << "║   Fastest check:               " << std::right << std::setw(23) << fastestCheck << " ms ║\n";
    ss << "║   Slowest check:               " << std::right << std::setw(23) << slowestCheck << " ms ║\n";
    ss << "║                                                               ║\n";
    
    ss << "║ Stream Sessions:                                              ║\n";
    ss << "║   Total streams recorded:      " << std::right << std::setw(28) << sessions.size() << " ║\n";
    
    if (!sessions.empty()) {
        long long totalTime = GetTotalStreamTime();
        int hours = totalTime / 3600;
        int minutes = (totalTime % 3600) / 60;
        
        ss << "║   Total stream time:           " << std::right << std::setw(18) << hours << "h " << minutes << "m ║\n";
        
        long long avgDuration = GetAverageStreamDuration();
        int avgHours = avgDuration / 3600;
        int avgMinutes = (avgDuration % 3600) / 60;
        
        ss << "║   Average stream duration:     " << std::right << std::setw(18) << avgHours << "h " << avgMinutes << "m ║\n";
        
        long long longest = GetLongestStream();
        int longHours = longest / 3600;
        int longMinutes = (longest % 3600) / 60;
        
        ss << "║   Longest stream:              " << std::right << std::setw(18) << longHours << "h " << longMinutes << "m ║\n";
        
        long long shortest = GetShortestStream();
        int shortHours = shortest / 3600;
        int shortMinutes = (shortest % 3600) / 60;
        
        ss << "║   Shortest stream:             " << std::right << std::setw(18) << shortHours << "h " << shortMinutes << "m ║\n";
    }
    
    ss << "╚═══════════════════════════════════════════════════════════════╝\n";
    
    return ss.str();
}


void Statistics::PrintSummary() const {
    std::cout << GetSummaryString();
}


void Statistics::Reset() {
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
        return false;
    }
    
    // Простой JSON формат
    file << "{\n";
    file << "  \"streamer\": \"" << streamerName << "\",\n";
    file << "  \"total_checks\": " << totalChecks << ",\n";
    file << "  \"online_detections\": " << onlineDetections << ",\n";
    file << "  \"offline_detections\": " << offlineDetections << ",\n";
    file << "  \"total_check_time\": " << totalCheckTime << ",\n";
    file << "  \"fastest_check\": " << fastestCheck << ",\n";
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
    return true;
}


bool Statistics::LoadFromFile() {
    std::ifstream file(statsFilePath);
    
    if (!file.is_open()) {
        // Файл не существует - это нормально при первом запуске
        return false;
    }
    
    // Упрощенный парсинг JSON (для production лучше использовать библиотеку)
    std::string line;
    while (std::getline(file, line)) {
        // Парсим основные поля
        if (line.find("\"total_checks\":") != std::string::npos) {
            size_t pos = line.find(":");
            totalChecks = std::stoi(line.substr(pos + 1));
        }
        else if (line.find("\"online_detections\":") != std::string::npos) {
            size_t pos = line.find(":");
            onlineDetections = std::stoi(line.substr(pos + 1));
        }
        else if (line.find("\"offline_detections\":") != std::string::npos) {
            size_t pos = line.find(":");
            offlineDetections = std::stoi(line.substr(pos + 1));
        }
        else if (line.find("\"total_check_time\":") != std::string::npos) {
            size_t pos = line.find(":");
            totalCheckTime = std::stoll(line.substr(pos + 1));
        }
        else if (line.find("\"fastest_check\":") != std::string::npos) {
            size_t pos = line.find(":");
            fastestCheck = std::stoll(line.substr(pos + 1));
        }
        else if (line.find("\"slowest_check\":") != std::string::npos) {
            size_t pos = line.find(":");
            slowestCheck = std::stoll(line.substr(pos + 1));
        }
    }
    
    file.close();
    return true;
}