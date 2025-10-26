#include "MultiStreamMonitor.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>


MultiStreamMonitor::MultiStreamMonitor(const std::string& configPath)
    : config(configPath), isRunning(false) {
    
    config.Load();
    std::cout << "Multi-Stream Monitor initialized" << std::endl;
}


MultiStreamMonitor::~MultiStreamMonitor() {
    StopAll();
}


void MultiStreamMonitor::MonitorThreadFunction(StreamMonitor* monitor) {
    try {
        monitor->StartMonitoring();
    } catch (const std::exception& e) {
        std::cerr << "Error in monitor thread: " << e.what() << std::endl;
    }
}


bool MultiStreamMonitor::AddStreamer(const std::string& streamerName) {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Проверка на дубликаты
    for (const auto& info : monitors) {
        if (info.streamerName == streamerName) {
            std::cerr << "Streamer " << streamerName << " is already being monitored" << std::endl;
            return false;
        }
    }
    
    MonitorInfo info;
    info.streamerName = streamerName;
    info.monitor = std::make_unique<StreamMonitor>(streamerName, "config.ini");
    info.isRunning = false;
    
    monitors.push_back(std::move(info));
    
    std::cout << "Added streamer: " << streamerName << std::endl;
    return true;
}


bool MultiStreamMonitor::RemoveStreamer(const std::string& streamerName) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = std::find_if(monitors.begin(), monitors.end(),
        [&streamerName](const MonitorInfo& info) {
            return info.streamerName == streamerName;
        });
    
    if (it == monitors.end()) {
        std::cerr << "Streamer " << streamerName << " not found" << std::endl;
        return false;
    }
    
    // Остановка потока если запущен
    if (it->isRunning && it->thread && it->thread->joinable()) {
        // Note: в production нужно добавить механизм graceful shutdown
        it->thread->detach();
    }
    
    monitors.erase(it);
    std::cout << "Removed streamer: " << streamerName << std::endl;
    return true;
}


std::vector<std::string> MultiStreamMonitor::GetStreamers() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex));
    
    std::vector<std::string> streamers;
    for (const auto& info : monitors) {
        streamers.push_back(info.streamerName);
    }
    
    return streamers;
}


void MultiStreamMonitor::StartAll() {
    std::lock_guard<std::mutex> lock(mutex);
    
    std::cout << "\nStarting monitoring for " << monitors.size() << " streamer(s)...\n" << std::endl;
    
    for (auto& info : monitors) {
        if (!info.isRunning) {
            info.thread = std::make_unique<std::thread>(
                &MultiStreamMonitor::MonitorThreadFunction, 
                this, 
                info.monitor.get()
            );
            info.isRunning = true;
            
            std::cout << "  ✓ Started monitoring: " << info.streamerName << std::endl;
            
            // Небольшая задержка между запусками потоков
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    
    isRunning = true;
    std::cout << "\nAll monitors started! Press Ctrl+C to stop.\n" << std::endl;
}


void MultiStreamMonitor::StopAll() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!isRunning) return;
    
    std::cout << "\nStopping all monitors..." << std::endl;
    
    for (auto& info : monitors) {
        if (info.isRunning && info.thread && info.thread->joinable()) {
            info.thread->detach();  // В production нужен graceful shutdown
            info.isRunning = false;
            std::cout << "  ✓ Stopped: " << info.streamerName << std::endl;
        }
    }
    
    isRunning = false;
    std::cout << "All monitors stopped." << std::endl;
}


void MultiStreamMonitor::PrintStatus() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex));
    
    std::cout << "\n╔════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║        MULTI-STREAM MONITOR STATUS            ║" << std::endl;
    std::cout << "╠════════════════════════════════════════════════╣" << std::endl;
    std::cout << "║ Total streamers: " << std::left << std::setw(29) << monitors.size() << "║" << std::endl;
    std::cout << "║ Running: " << std::left << std::setw(37) << (isRunning ? "YES" : "NO") << "║" << std::endl;
    std::cout << "╠════════════════════════════════════════════════╣" << std::endl;
    
    for (const auto& info : monitors) {
        std::cout << "║ • " << std::left << std::setw(30) << info.streamerName;
        std::cout << std::right << std::setw(15) << (info.isRunning ? "[RUNNING]" : "[STOPPED]") << " ║" << std::endl;
    }
    
    std::cout << "╚════════════════════════════════════════════════╝" << std::endl;
}


bool MultiStreamMonitor::LoadStreamersFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        std::cerr << "Cannot open streamers file: " << filePath << std::endl;
        return false;
    }
    
    std::string streamerName;
    int count = 0;
    
    while (std::getline(file, streamerName)) {
        // Пропускаем комментарии и пустые строки
        if (streamerName.empty() || streamerName[0] == '#') {
            continue;
        }
        
        // Убираем пробелы
        streamerName.erase(0, streamerName.find_first_not_of(" \t\r\n"));
        streamerName.erase(streamerName.find_last_not_of(" \t\r\n") + 1);
        
        if (!streamerName.empty()) {
            if (AddStreamer(streamerName)) {
                count++;
            }
        }
    }
    
    file.close();
    
    std::cout << "Loaded " << count << " streamer(s) from " << filePath << std::endl;
    return count > 0;
}